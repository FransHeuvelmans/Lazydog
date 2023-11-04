import sys
import sqlite3
import csv

TABLE_NAME = "bangle_basic_heart"


def create_table(dest_conn):
    q = (
        f"CREATE TABLE IF NOT EXISTS {TABLE_NAME} ("
        + """
            rec_id INTEGER PRIMARY KEY,
            time INTEGER NOT NULL,
            heart_rate1 INTEGER,
            heart_rate2 INTEGER
            );
        """
    )
    dst_cur = dest_conn.cursor()
    dst_cur.execute(q)
    dest_conn.commit()


def csv_reader(source):
    with open(source, newline="") as csvfile:
        heart_reader = csv.reader(csvfile, delimiter=",")
        for row in heart_reader:
            yield row


def insert_batch(data, dest_conn):
    dst_cur = dest_conn.cursor()
    dst_cur.executemany(
        f"INSERT INTO {TABLE_NAME} (time, heart_rate1, heart_rate2)"
        + "VALUES(?, ?, ?)",
        data,
    )
    dest_conn.commit()


def transfer_csv_data(source, target):
    """Move csv data from heartrate csv data
    to an sqlite file"""
    dst_conn = sqlite3.connect(target)
    create_table(dst_conn)

    rows = []
    for x in csv_reader(source):
        rows.append(x)
        if len(rows) > 50:
            insert_batch(rows, dst_conn)
            rows = []
    if len(rows) > 0:
        insert_batch(rows, dst_conn)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Err: need source and target")
    transfer_csv_data(sys.argv[1], sys.argv[2])
