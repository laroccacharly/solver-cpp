# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "pandas",
# ]
# ///

import sqlite3
import pandas as pd

def get_sqlite_path() -> str: 
    return "data/db.sqlite"

def make_instance_selection():
    """
    An instance is selected if it has a mip_gap above 0.05 and below 10.
    We set the bool selected to true for these instances.
    """
    db_path = get_sqlite_path()
    con = sqlite3.connect(db_path)
    cursor = con.cursor()

    # First, set all instances to not selected
    cursor.execute("UPDATE instances SET selected = 0")

    # Now, find the instances to select
    query = """
    WITH latest_jobs AS (
        SELECT
            instance_id,
            id as job_id
        FROM (
            SELECT
                j.*,
                ROW_NUMBER() OVER (PARTITION BY j.instance_id ORDER BY j.created_at DESC) as rn
            FROM jobs j
            WHERE j.group_name = 'grb_only'
        )
        WHERE rn = 1
    )
    SELECT
        lj.instance_id
    FROM latest_jobs lj
    JOIN grb_attributes g ON lj.job_id = g.job_id
    WHERE g.mip_gap > 0.05 AND g.mip_gap < 10 AND g.solution IS NOT NULL AND g.solution != ''
    """

    cursor.execute(query)
    selected_instance_ids = [row[0] for row in cursor.fetchall()]

    if not selected_instance_ids:
        print("No instances to select.")
        con.close()
        return

    # Update selected instances
    # Create a string of question marks for the IN clause
    placeholders = ",".join("?" for _ in selected_instance_ids)
    update_query = f"UPDATE instances SET selected = 1 WHERE id IN ({placeholders})"

    cursor.execute(update_query, selected_instance_ids)

    print(f"Selected {len(selected_instance_ids)} instances.")

    con.commit()
    con.close()


if __name__ == "__main__":
    make_instance_selection() 