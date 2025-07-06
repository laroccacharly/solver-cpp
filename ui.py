import streamlit as st
import pandas as pd
import sqlite3
from typing import Callable
# --- 
from pysolver import get_instance_selection_df
from pydantic import BaseModel

class Page(BaseModel): 
    name: str
    function: Callable

def get_pages() -> list[Page]: 
    return [
        Page(name="Instance Selection", function=instance_selection_page),
        Page(name="Tables", function=tables_page),
    ]

def instance_selection_page(): 
    st.title("Instance Selection")
    df = get_instance_selection_df()
    st.dataframe(df)
    
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
    pages = get_pages()

    st.set_page_config(page_title="Solver", page_icon=":robot:", layout="wide")
    st.sidebar.title("Navigation")
    selected_page_name = st.sidebar.selectbox("Select a page", [p.name for p in pages], index=0)
    
    # Find the page object that matches the selected name
    selected_page = next(p for p in pages if p.name == selected_page_name)
    selected_page.function()

if __name__ == "__main__":
    main_ui()