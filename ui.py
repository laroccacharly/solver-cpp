# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "pandas",
# ]
# ///
import streamlit as st
import pandas as pd
import sqlite3

def tables_page(): 
    st.title("Database Tables")

    conn = sqlite3.connect("data/db.sqlite")

    # Get a list of tables
    cursor = conn.cursor()
    cursor.execute("SELECT name FROM sqlite_master WHERE type='table';")
    tables = cursor.fetchall()

    # Display each table
    for table_name in tables:
        table_name = table_name[0]
        st.subheader(f"Table: {table_name}")
        df = pd.read_sql_query(f"SELECT * FROM {table_name} LIMIT 1000", conn)
        st.dataframe(df)

    conn.close()


def home_page(): 
    st.title("Home")


def main_ui(): 
    st.set_page_config(page_title="Solver", page_icon=":robot:", layout="wide")
    st.sidebar.title("Navigation")
    page = st.sidebar.selectbox("Select a page", ["Home", "Tables"], index=0)
    if page == "Home":
        home_page()
    elif page == "Tables":
        tables_page()

if __name__ == "__main__":
    main_ui()