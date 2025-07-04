# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "pandas",
# ]
# ///
import streamlit as st
import pandas as pd
import sqlite3

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
