from .instance_selection import get_instance_selection_df
from .connection import get_connection, close_connection

def make_instance_selection():
    df = get_instance_selection_df()
    selected_instance_ids = df["instance_id"].tolist()

    if not selected_instance_ids:
        print("No instances to select.")
        return


    placeholders = ",".join("?" for _ in selected_instance_ids)
    update_query = f"UPDATE instances SET selected = 1 WHERE id IN ({placeholders})"

    con = get_connection()
    cursor = con.cursor()
    cursor.execute(update_query, selected_instance_ids)
    con.commit()
    print(f"Selected {len(selected_instance_ids)} instances.")
    close_connection()

if __name__ == "__main__":
    make_instance_selection()