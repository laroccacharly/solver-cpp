import streamlit as st
from .connection import query_to_df
import pandas as pd

def get_warm_start_jobs_df() -> pd.DataFrame: 
    return query_to_df("SELECT * FROM jobs WHERE group_name = 'warm_start'")

def warm_start_ui(): 
    st.title("Warm Start")
    df = get_warm_start_jobs_df()
    st.dataframe(df)