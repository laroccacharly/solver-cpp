import streamlit as st
from .connection import get_connection, close_connection

def run_sql_ui():
    st.subheader("Run SQL")
    sql = st.text_area("SQL")
    if st.button("Run"):
        con = get_connection()
        cursor = con.cursor()
        cursor.execute(sql)
        results = cursor.fetchall()
        st.dataframe(results)
        con.commit()
        close_connection()
