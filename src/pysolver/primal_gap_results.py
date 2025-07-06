import streamlit as st
from .connection import query_to_df

def primal_gap_results_ui(): 
    """
        Display the results for the primal gap for each job group. 
        Only include instances that are selected (instance.selected = true)
        Each row of the result table corresponds to a job group (groupby job.group_name). 
        The value of primal gap is available in the grb_attributes table. 
        Columns: 
            - group_name: the name of the job group
            - sample_size: number of values in the group
            - avg_primal_gap: the average primal gap for the group
            - max_primal_gap: the maximum primal gap for the group
            - wins: the number of times that group holds the best objective value for that instance
    """
    query = """
        WITH best_instance_obj_val AS (
            SELECT
                j.instance_id,
                MIN(ga.obj_val) as min_obj_val
            FROM grb_attributes ga
            JOIN jobs j ON ga.job_id = j.id
            JOIN instances i ON j.instance_id = i.id
            WHERE i.selected = 1 AND ga.sol_count > 0
            GROUP BY j.instance_id
        ),
        instance_winners AS (
            SELECT
                j.group_name,
                j.instance_id
            FROM grb_attributes ga
            JOIN jobs j ON ga.job_id = j.id
            JOIN best_instance_obj_val biov ON j.instance_id = biov.instance_id
            WHERE ga.obj_val = biov.min_obj_val AND ga.sol_count > 0
        ),
        group_wins AS (
            SELECT
                group_name,
                COUNT(instance_id) as wins
            FROM instance_winners
            GROUP BY group_name
        )
        SELECT 
            j.group_name, 
            count(ga.primal_gap) as sample_size,
            avg(ga.primal_gap) as avg_primal_gap,
            max(ga.primal_gap) as max_primal_gap,
            COALESCE(gw.wins, 0) as wins
        FROM grb_attributes ga
        JOIN jobs j on ga.job_id = j.id
        JOIN instances i on j.instance_id = i.id
        LEFT JOIN group_wins gw ON j.group_name = gw.group_name
        WHERE i.selected = 1
        GROUP BY j.group_name
        ORDER BY wins DESC
    """
    df = query_to_df(query)
    st.subheader("Primal Gap Results")
    st.dataframe(df)