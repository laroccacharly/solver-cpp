import streamlit as st
from .connection import query_with_duckdb
import pandas as pd

def explanations(): 
    st.markdown("""
    Notes: 
    - **Primal gap** is the gap relative to the best known solution for the instance (always positive). 
    - **Warm start**: it uses the best solution found by the *grb_only* run and passes it to the solver as a starting point. 
    - **LNS**: it randomly selects a subset of variables (configured by the fixing ratio) and fixes them to their value in the best solution found by the *grb_only* run. 
    - All jobs are run with the same time limit of 10 seconds and 3 different seeds. 
    """)

def primal_gap_results_ui():     
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
    
    st.header("Performance Results")
    explanations()
    st.subheader("Distribution of the primal gap for each strategy")
    st.dataframe(df)

    # key takeaways 
    st.subheader("Conclusions")
    st.markdown("""
    - Based on the data, a smaller fixing ratio for the LNS is significantly more effective. As the fixing ratio increases from 0.05 to 0.80, the performance consistently degrades, with both the average and median primal gaps increasing and the number of wins decreasing. A high fixing ratio may be more effective for very difficult problems.
    - The average is 2 orders of magnitude larger than the median which suggests that there are a few instances where the primal gap is very large. 
    - If your goal is to have the best performance on the majority of problems and you can tolerate an occasional very poor result, warm_start looks good. It's more often the winner on a typical instance.
    - If your goal is to have the most reliable performance and minimize the risk of a very bad outcome, lns_0.05 is the better choice. It might not win as often on the "easy" half of problems, but it is more stable and avoids the extreme performance degradations that hurt warm_start's average.
    """)