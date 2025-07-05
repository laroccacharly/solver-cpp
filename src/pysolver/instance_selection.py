import sqlite3
import pandas as pd
from .connection import get_connection, close_connection

def get_instance_selection_query() -> str: 
    return """
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
            lj.instance_id,
            g.runtime,
            g.mip_gap,
            g.solution
        FROM latest_jobs lj
        JOIN grb_attributes g ON lj.job_id = g.job_id
        WHERE g.mip_gap > 0.05 AND g.mip_gap < 10 AND g.solution IS NOT NULL AND g.solution != ''
    """

def query_to_df(query: str) -> pd.DataFrame: 
    con = get_connection()
    df = pd.read_sql_query(query, con)
    close_connection()
    return df

def get_instance_selection_df() -> pd.DataFrame: 
    return query_to_df(get_instance_selection_query())

def _make_instance_selection():
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

