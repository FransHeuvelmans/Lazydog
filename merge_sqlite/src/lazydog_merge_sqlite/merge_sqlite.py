# SPDX-License-Identifier: EUPL-1.2
import sqlite3
import logging
from typing import List, Dict
from dataclasses import dataclass
from pathlib import Path
from enum import Enum

COMMON_SQLITE_SUFFIX = {".sqlite", ".db"}
logger = logging.getLogger("sqlite_merger")


@dataclass(frozen=True)
class Column:
    """Column in a Sqlite table"""

    cid: int
    name: str
    type_: str
    notnull: bool
    pk: int


class InsertMode(Enum):
    """What should happen on conflicts during insertion"""

    ABORT = (1,)
    IGNORE = (2,)
    REPLACE = 3


TABLE_NAME_QUERY = """
SELECT 
    name
FROM 
    sqlite_schema
WHERE 
    type ='table' AND 
    name NOT LIKE 'sqlite_%';
"""


def run_table_info_query(curs: sqlite3.Cursor, table_name: str) -> List[Column]:
    """Get Table information for a particular table"""

    def map_tup(t) -> Column:
        return Column(t[0], t[1], t[2], bool(t[3]), t[5])

    res = curs.execute("pragma table_info(" + table_name + ")")
    return [map_tup(t) for t in res.fetchall()]


def get_table_info(db_loc: Path) -> Dict[str, List[Column]]:
    """Retrieve the list of tables can be moved"""
    if db_loc.suffix not in COMMON_SQLITE_SUFFIX:
        logger.warn("Database filetype might be wrong: " + db_loc.suffix)
    conn = sqlite3.connect(db_loc)
    curs = conn.cursor()
    res = curs.execute(TABLE_NAME_QUERY)
    tables = res.fetchall()
    out_tables = {}
    for (table_name,) in tables:
        table_info = run_table_info_query(curs, table_name)
        out_tables[table_name] = table_info
    conn.close()
    return out_tables


@dataclass(frozen=True)
class CompatResult:
    no_overlap: bool
    mergable: bool
    overlapping_tables: List[str]
    non_overlapping_tables: List[str]
    failed_table: str = ""


def check_compatibility(
    src_db_info: Dict[str, List[Column]], dst_db_info: Dict[str, List[Column]]
) -> CompatResult:
    """Check if two databases are compatible and can be merged
    Returns results from the tables of a's perspective"""
    src_keys = set(src_db_info.keys())
    dst_keys = set(dst_db_info.keys())
    matching_tables = src_keys.intersection(dst_keys)
    if len(matching_tables) == 0:
        return CompatResult(True, True, [], list(src_keys))
    for tab in matching_tables:
        a_cols = set(src_db_info[tab])
        b_cols = set(dst_db_info[tab])
        if a_cols != b_cols:
            return CompatResult(False, False, [], [], tab)
    non_matching_tables = src_keys.difference(matching_tables)
    assert matching_tables.union(non_matching_tables) == src_keys
    return CompatResult(False, True, list(matching_tables), list(non_matching_tables))


def first_copy(src: Path, dst: Path):
    """Quickly copy the complete src db into a fresh dst db"""
    src_conn = sqlite3.connect(src)
    dst_conn = sqlite3.connect(dst)
    logger.debug(f"Running backup: {src} → {dst}")
    src_conn.backup(dst_conn, pages=4)
    dst_conn.close()
    src_conn.close()


def merge_dbs(
    src: Path,
    dst: Path,
    overlapping_src_tables: List[str],
    non_overlap_src_tables: List[str],
    insert_mode: InsertMode,
):
    """Copy all tables from the src db into a dst db"""
    dst_conn = sqlite3.connect(dst)

    dst_cur = dst_conn.cursor()
    dst_cur.execute(f"ATTACH DATABASE '{str(src.absolute())}' AS dba")

    for non_ovr_st in non_overlap_src_tables:
        q = f"CREATE TABLE {non_ovr_st} AS SELECT * FROM dba.{non_ovr_st}"
        logger.debug("Running non overlap: " + q)
        dst_cur.execute(q)
        dst_conn.commit()

    for ovr_st in overlapping_src_tables:
        q = f"""
        INSERT OR {insert_mode.name} INTO {ovr_st}
        SELECT *
        FROM dba.{ovr_st}
        """
        logger.debug("Running overlap: " + q)
        try:
            dst_cur.execute(q)
            dst_conn.commit()
        except sqlite3.IntegrityError as e:
            logger.error(e)

    dst_conn.close()


def sql_merge_dbs(rest: List[Path], dst: Path, insert_mode: InsertMode):
    """Merge DBs with data. copies tables from rest to dst"""
    for other_db in rest:
        dst_info = get_table_info(dst)
        other_info = get_table_info(other_db)
        compat = check_compatibility(other_info, dst_info)
        if not compat.mergable:
            logger.error(
                f"Can't merge {other_db} into {dst}\n"
                + f"The table {compat.failed_table} can't be matched"
            )
            continue
        merge_dbs(
            other_db,
            dst,
            compat.overlapping_tables,
            compat.non_overlapping_tables,
            insert_mode,
        )
