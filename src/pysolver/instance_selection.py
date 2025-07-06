import pandas as pd
from .connection import get_connection, close_connection, query_to_df
import streamlit as st

def instance_selection_ui(): 
    st.subheader("Instances before filtering")
    before_filter_df = get_instance_selection_base_df()
    st.dataframe(before_filter_df)
    st.write(f"Total instances before filtering: {len(before_filter_df)}")
    
    st.subheader("Instances after filtering (mips gap > 0.05 and < 10)")
    after_filter_df = get_instance_selection_df()
    st.dataframe(after_filter_df)
    st.write(f"Total instances after filtering: {len(after_filter_df)}")

    if st.button("Make Selection"):
        make_instance_selection()

    st.subheader("Selected Instances")
    selected_df = get_selected_instances_df()
    st.dataframe(selected_df)

def get_instance_selection_base_query() -> str:
    """Get the base query without the final WHERE clause filtering"""
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
    """

def get_instance_selection_query() -> str: 
    base_query = get_instance_selection_base_query()
    return base_query + "\n WHERE g.mip_gap > 0.05 AND g.mip_gap < 10 AND g.solution IS NOT NULL AND g.solution != ''"


def get_instance_selection_base_df() -> pd.DataFrame:
    """Get dataframe before applying the final WHERE clause"""
    return query_to_df(get_instance_selection_base_query())

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
