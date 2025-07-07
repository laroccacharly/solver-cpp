import streamlit as st
from .connection import query_with_duckdb
import pandas as pd

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
    
    summary_query = """
        SELECT
            j.group_name,
            COUNT(ga.primal_gap) AS sample_size,
            AVG(ga.primal_gap) AS avg_primal_gap,
            MAX(ga.primal_gap) AS max_primal_gap,
            STDDEV_SAMP(ga.primal_gap) AS std_dev_primal_gap,
            QUANTILE_CONT(ga.primal_gap, 0.25) AS "25th_percentile",
            QUANTILE_CONT(ga.primal_gap, 0.50) AS "median",
            QUANTILE_CONT(ga.primal_gap, 0.75) AS "75th_percentile"
        FROM grb_attributes ga
        JOIN jobs j ON ga.job_id = j.id
        JOIN instances i ON j.instance_id = i.id
        WHERE i.selected = 1 AND ga.primal_gap IS NOT NULL
        GROUP BY j.group_name
    """
    summary_results_df = query_with_duckdb(summary_query)

    wins_query = """
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
        SELECT * FROM group_wins
    """
    wins_df = query_with_duckdb(wins_query)
    summary_results_df = pd.merge(summary_results_df, wins_df, on='group_name', how='left')

    display_columns = [
        'group_name', 'wins', 'sample_size', 'avg_primal_gap', 'std_dev_primal_gap',
        '25th_percentile', 'median', '75th_percentile'
    ]
    df = summary_results_df[display_columns].sort_values(by='wins', ascending=False)
    
    st.subheader("Primal Gap Results")
    st.dataframe(df)

    # keytake aways 
    st.subheader("Key Takeaways")
    st.markdown("""
    - **Most Reliable Strategy (`warm_start`):**
        - Choose `warm_start` if you need the approach most likely to produce the best result on any given instance. It is the most consistent and robust method, achieving the highest number of "wins."

    - **Best Average Performance (`lns`):**
        - Choose `lns` with a small fixing ratio if your primary goal is to achieve the best possible performance *on average* across all your problems. While it might not win on every single instance, it has the potential to find significantly better solutions for difficult problems where `warm_start` struggles.
    """)