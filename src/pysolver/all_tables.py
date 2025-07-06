import streamlit as st
from .connection import get_connection, close_connection
import pandas as pd

def all_tables_ui(): 
    st.title("All Tables in Database")
    con = get_connection()
    cursor = con.cursor()
    cursor.execute("SELECT name FROM sqlite_master WHERE type='table';")
    tables = cursor.fetchall()

    for table in tables:
        # Get row count for the table
        cursor.execute(f"SELECT COUNT(*) FROM {table[0]}")
        row_count = cursor.fetchone()[0]
        
        st.subheader(f"Table: {table[0]} ({row_count} rows)")
        df = pd.read_sql_query(f"SELECT * FROM {table[0]} LIMIT 1000", con)
        st.dataframe(df)    

    close_connection()