# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "pandas",
#     "plotly",
#     "duckdb",
# ]
# ///

import pandas as pd
import plotly.express as px
import duckdb

def get_sqlite_path() -> str: 
    return "data/db.sqlite"

def get_latest_job_per_instance_df() -> pd.DataFrame:
    """
    Get the latest job for each instance and its GRB attributes.
    """
    con = duckdb.connect()
    con.execute(f"ATTACH '{get_sqlite_path()}' AS sqlitedb (TYPE sqlite);")

    query = """
    WITH latest_jobs AS (
        SELECT * EXCLUDE (rn) FROM (
            SELECT
                *,
                ROW_NUMBER() OVER (PARTITION BY instance_id ORDER BY created_at DESC) as rn
            FROM sqlitedb.jobs
        )
        WHERE rn = 1
    )
    SELECT
        i.name as instance_name,
        j.time_limit_s,
        j.created_at as job_created_at,
        g.* EXCLUDE (id, job_id)
    FROM sqlitedb.instances AS i
    LEFT JOIN latest_jobs AS j ON i.id = j.instance_id
    LEFT JOIN sqlitedb.grb_attributes AS g ON j.id = g.job_id
    ORDER BY i.name
    """

    df = con.execute(query).fetch_df()
    con.close()
    return df

def run():
    df = get_latest_job_per_instance_df()
    print(df)

run()