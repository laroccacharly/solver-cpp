import sqlite3
import duckdb
import pandas as pd  

DB_PATH = "data/db.sqlite"

_connection = None  
def close_connection():
    global _connection
    if _connection is not None:
        _connection.close()
        _connection = None

def get_connection() -> sqlite3.Connection:
    global _connection
    if _connection is None:
        _connection = sqlite3.connect(DB_PATH)
    return _connection

def query_to_df(query: str) -> pd.DataFrame: 
    con = get_connection()
    df = pd.read_sql_query(query, con)
    close_connection()
    return df

_duck_connection = None 
def get_duckdb_connection() -> duckdb.DuckDBPyConnection:
    global _duck_connection
    if _duck_connection is None:
        _duck_connection = duckdb.connect(database=DB_PATH, read_only=True)
        _duck_connection.execute(f"ATTACH '{DB_PATH}' as sqlite_db (TYPE sqlite);")
        _duck_connection.execute("USE sqlite_db;")
    return _duck_connection

def close_duckdb_connection():
    global _duck_connection
    if _duck_connection is not None:
        _duck_connection.close()
        _duck_connection = None

def query_with_duckdb(query: str) -> pd.DataFrame:
    con = get_duckdb_connection()
    df = con.execute(query).fetch_df()
    close_duckdb_connection()
    return df