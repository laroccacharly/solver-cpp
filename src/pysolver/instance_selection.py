import pandas as pd
from .connection import query_to_df
import streamlit as st

def instance_selection_ui(): 
    st.subheader("Instances with binary variables")
    st.write("This is the list of instances in MIPLIB that contain binary variables.")

    df = get_instance_selection_base_df()
    st.dataframe(df)
    st.write(f"Instance count: {len(df)}")

    st.write(f"Number of instances with no solution: {len(df[df['sol_count'] == 0])}")
    st.write(f"Number of instances where optimal solution is found: {len(df[df['mip_gap'] == 0])}")

    
    st.subheader("Instances after filtering (0.05 < mip_gap < 10)")
    st.write("We select instances that have an medium level of difficulty. We use the MIP gap to assess difficulty.")

    after_filter_df = get_instance_selection_df()
    st.dataframe(after_filter_df)
    st.write(f"Instance count: {len(after_filter_df)}")

    st.subheader("Selected Instances")
    selected_df = get_selected_instances_df()
    st.dataframe(selected_df)
    st.write(f"Instance count: {len(selected_df)}")
def get_instance_selection_base_query() -> str:
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
            g.sol_count,
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
    return query_to_df(get_instance_selection_base_query())

def get_instance_selection_df() -> pd.DataFrame: 
    return query_to_df(get_instance_selection_query())

def get_selected_instances_df() -> pd.DataFrame: 
    return query_to_df("SELECT * FROM instances WHERE selected = 1")
