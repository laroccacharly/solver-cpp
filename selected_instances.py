# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "pandas",
# ]
# ///

import sqlite3
import pandas as pd

def get_sqlite_path() -> str: 
    return "data/db.sqlite"

def create_selected_instances_table():
    """
    Create a new table called selected_instances with instances that have
    a mip_gap above 0.05 and below 10.
    """
    db_path = get_sqlite_path()
    
    try:
        con = sqlite3.connect(db_path)
        cursor = con.cursor()
        
        # First, drop the table if it exists
        cursor.execute("DROP TABLE IF EXISTS selected_instances")
        
        # Create the selected_instances table
        create_table_query = """
        CREATE TABLE selected_instances (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            created_at INTEGER
        )
        """
        cursor.execute(create_table_query)
        
        # Insert data from instances with mip_gap between 0.05 and 10
        insert_query = """
        INSERT INTO selected_instances
        WITH latest_jobs AS (
            SELECT * FROM (
                SELECT
                    *,
                    ROW_NUMBER() OVER (PARTITION BY instance_id ORDER BY created_at DESC) as rn
                FROM jobs
            )
            WHERE rn = 1
        )
        SELECT
            i.id,
            i.name,
            i.created_at
        FROM instances AS i
        INNER JOIN latest_jobs AS j ON i.id = j.instance_id
        INNER JOIN grb_attributes AS g ON j.id = g.job_id
        WHERE g.mip_gap > 0.05 AND g.mip_gap < 10
        """
        cursor.execute(insert_query)
        
        # Get count of inserted rows
        cursor.execute("SELECT COUNT(*) FROM selected_instances")
        count = cursor.fetchone()[0]
        
        con.commit()
        print(f"Successfully created selected_instances table with {count} instances")
        print("Criteria: mip_gap > 0.05 AND mip_gap < 10")
        
        # Show the selected instances
        cursor.execute("SELECT id, name FROM selected_instances ORDER BY name")
        results = cursor.fetchall()
        
        print("\nSelected instances:")
        for id, name in results:
            print(f"  {id}: {name}")
            
    except sqlite3.Error as e:
        print(f"Database error: {e}")
    finally:
        if 'con' in locals() and con:
            con.close()

def show_selected_instances():
    """
    Display the contents of the selected_instances table.
    """
    db_path = get_sqlite_path()
    
    try:
        con = sqlite3.connect(db_path)
        df = pd.read_sql_query("SELECT * FROM selected_instances ORDER BY name", con)
        print("Selected Instances Table:")
        print(df)
        con.close()
    except sqlite3.Error as e:
        print(f"Database error: {e}")

if __name__ == "__main__":
    create_selected_instances_table()
    print("\n" + "="*50)
    show_selected_instances()
