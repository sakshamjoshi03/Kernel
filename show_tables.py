import sqlite3

conn = sqlite3.connect('data/watchos_sessions.db')
cur = conn.cursor()

tables_query = """
SELECT name 
FROM sqlite_master 
WHERE type = 'table' AND name NOT LIKE 'sqlite_%' 
ORDER BY name;
"""

print(f"{'#':<3} | {'TABLE NAME':<26} | {'COLUMNS':<8} | {'ROW COUNT'}")
print("=" * 55)

tables = [r[0] for r in cur.execute(tables_query).fetchall()]
for i, table in enumerate(tables, start=1):
    cols = len(cur.execute(f"PRAGMA table_info({table});").fetchall())
    rows = cur.execute(f"SELECT COUNT(*) FROM {table};").fetchone()[0]
    print(f"{i:<3} | {table:<26} | {cols:<8} | {rows}")

print("=" * 55)
print(f"Total Tables: {len(tables)}")

