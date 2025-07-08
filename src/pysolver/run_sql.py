import streamlit as st
from .connection import query_with_duckdb

def get_example_sql() -> str:
    return """
    SELECT * FROM instances LIMIT 10
    """

def run_sql_ui():
    st.subheader("Run SQL")

    example_sql = get_example_sql()

    sql = st.text_area("SQL", example_sql, placeholder="Enter your SQL query here...")
    if st.button("Run"):
        df = query_with_duckdb(sql)
        st.dataframe(df)
