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
            i.num_bin_variables,
            g.solution
        FROM latest_jobs lj
        JOIN grb_attributes g ON lj.job_id = g.job_id
        JOIN instances i ON lj.instance_id = i.id
        WHERE g.mip_gap > 0.05 AND g.mip_gap < 10 AND g.solution IS NOT NULL AND g.solution != ''
    """

def query_to_df(query: str) -> pd.DataFrame: 
    con = get_connection()
    df = pd.read_sql_query(query, con)
    close_connection()
    return df

def get_instance_selection_df() -> pd.DataFrame: 
    return query_to_df(get_instance_selection_query())

def make_instance_selection():
    df = get_instance_selection_df()
    selected_instance_ids = df["instance_id"].tolist()

    if not selected_instance_ids:
        print("No instances to select.")
        return


    placeholders = ",".join("?" for _ in selected_instance_ids)
    update_query = f"UPDATE instances SET selected = 1 WHERE id IN ({placeholders})"

    con = get_connection()
    cursor = con.cursor()
    cursor.execute(update_query, selected_instance_ids)
    con.commit()
    print(f"Selected {len(selected_instance_ids)} instances.")
    close_connection()


def get_selected_instances_df() -> pd.DataFrame: 
    return query_to_df("SELECT * FROM instances WHERE selected = 1")
