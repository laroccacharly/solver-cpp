import sqlite3  

_connection = None  
def close_connection():
    global _connection
    if _connection is not None:
        _connection.close()
        _connection = None

def get_connection() -> sqlite3.Connection:
    global _connection
    if _connection is None:
        _connection = sqlite3.connect("data/db.sqlite")
    return _connection