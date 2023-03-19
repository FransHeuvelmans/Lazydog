# SPDX-License-Identifier: EUPL-1.2
from pathlib import Path
from lazydog_merge_sqlite import merge_sqlite
import sqlite3


def check_tool_info_merger(db_path):
    "Get all tools found"
    conn = sqlite3.connect(db_path)
    curs = conn.cursor()
    res = curs.execute("SELECT tool_name FROM tool_info")
    return set([x[0] for x in res.fetchall()])


def test_table_info():
    a_db_path = (Path(__file__).parent) / "data" / "openScale.db"
    all_info = merge_sqlite.get_table_info(a_db_path)
    assert len(all_info) == 2
    assert len(all_info["openscale_measurements"]) == 4


def test_table_comparison():
    columns_a = [
        merge_sqlite.Column(0, "a", "REAL", True, 1),
        merge_sqlite.Column(1, "b", "TEXT", False, 0),
        merge_sqlite.Column(2, "c", "REAL", False, 0),
    ]
    a = set(columns_a)
    columns_b = [
        merge_sqlite.Column(1, "b", "TEXT", False, 0),
        merge_sqlite.Column(0, "a", "REAL", True, 1),
        merge_sqlite.Column(2, "c", "REAL", False, 0),
    ]
    b = set(columns_b)
    assert a == b
    db_1 = {"tab_a": columns_a}
    db_2 = {"tab_a": columns_b}
    check1 = merge_sqlite.check_compatibility(db_1, db_2)
    assert check1.mergable == True
    assert check1.no_overlap == False
    assert check1.overlapping_tables == ["tab_a"]
    assert check1.non_overlapping_tables == []

    columns_c = [
        merge_sqlite.Column(1, "b", "TEXT", False, 0),
        merge_sqlite.Column(0, "a", "REAL", True, 1),
        merge_sqlite.Column(2, "c", "REAL", True, 0),
    ]
    db_1 = {"tab_a": columns_a}
    db_2 = {"tab_a": columns_c}
    check2 = merge_sqlite.check_compatibility(db_1, db_2)
    assert check2.mergable == False
    assert check2.no_overlap == False
    assert check2.overlapping_tables == []  # Nothing, bad match
    assert check2.non_overlapping_tables == []

    db_1 = {"tab_a": columns_a}
    db_2 = {"tab_b": columns_c}
    check3 = merge_sqlite.check_compatibility(db_1, db_2)
    assert check3.mergable == True
    assert check3.no_overlap == True
    assert check3.overlapping_tables == []
    assert check3.non_overlapping_tables == ["tab_a"]


def test_first_copy(db_export_file):
    a_db_path = (Path(__file__).parent) / "data" / "openScale.db"

    merge_sqlite.first_copy(a_db_path, db_export_file)

    table_info = merge_sqlite.get_table_info(db_export_file)
    assert "openscale_measurements" in table_info


def test_merger(db_export_file):
    a_db_path = (Path(__file__).parent) / "data" / "healthkit.db"
    merge_sqlite.first_copy(a_db_path, db_export_file)

    b_db_path = (Path(__file__).parent) / "data" / "paseo.db"

    a_info = merge_sqlite.get_table_info(db_export_file)
    b_info = merge_sqlite.get_table_info(b_db_path)
    compat_res = merge_sqlite.check_compatibility(b_info, a_info)
    assert compat_res.mergable == True
    assert compat_res.no_overlap == False
    merge_sqlite.merge_dbs(
        b_db_path,
        db_export_file,
        compat_res.overlapping_tables,
        compat_res.non_overlapping_tables,
    )

    out_info = merge_sqlite.get_table_info(db_export_file)
    assert "healthkit_Height" in out_info
    assert "paseo_hours" in out_info

    tools_found = check_tool_info_merger(db_export_file)
    assert "paseo-to-lazydog" in tools_found
    assert "healthkit-to-sqlite" in tools_found