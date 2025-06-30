# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "pandas",
#     "plotly",
#     "duckdb",
# ]
# ///

import pandas as pd
import duckdb
import plotly.graph_objects as go

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
    ORDER BY g.mip_gap ASC
    """

    df = con.execute(query).fetch_df()
    con.close()
    return df

def plotly_table(df: pd.DataFrame) -> go.Figure:
    return go.Figure(
        data=[
            go.Table(
                header=dict(
                    values=list(df.columns), fill_color="paleturquoise", align="left"
                ),
                cells=dict(
                    values=[df[col] for col in df.columns],
                    fill_color="lavender",
                    align="left",
                ),
            )
        ]
    )

def run():
    df = get_latest_job_per_instance_df()
    print(df)
    fig = plotly_table(df)
    fig.show()

if __name__ == "__main__":
    run()