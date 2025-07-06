from .connection import get_connection, close_connection

def set_primal_gap():
    """
    Computes the primal gap for each job and updates the grb_attributes table.
    The primal gap is defined as abs(obj_val - best_obj_val) / abs(best_obj_val).
    """
    
    query = """
    SELECT
        ga.id,
        ga.obj_val,
        i.best_known_obj_val
    FROM
        grb_attributes ga
    JOIN
        jobs j ON ga.job_id = j.id
    JOIN
        instances i ON j.instance_id = i.id
    WHERE
        ga.sol_count > 0;
    """

    con = get_connection()
    try:
        cursor = con.cursor()
        cursor.execute(query)
        results = cursor.fetchall()

        if not results:
            print("No solutions found to compute primal gap.")
            return

        updates = []
        for row in results:
            ga_id, obj_val, best_obj_val = row
            
            if best_obj_val is not None and abs(best_obj_val) > 1e-9:
                primal_gap = abs(obj_val - best_obj_val) / abs(best_obj_val)
            else:
                primal_gap = float('inf')

            updates.append((primal_gap, ga_id))

        cursor.executemany(
            "UPDATE grb_attributes SET primal_gap = ? WHERE id = ?",
            updates
        )
        con.commit()
        print(f"Successfully updated primal gap for {len(updates)} jobs.")

    finally:
        close_connection()

