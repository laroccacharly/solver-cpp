import streamlit as st

from pysolver import get_pages


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