from .connection import get_connection, close_connection

def set_best_obj_val(): 
    """
        This function sets the best_known_obj_val for each instance based on the best solution found by the jobs. 
        Queries all jobs for a given instance, finds the best obj val (lowest) and sets it as the best_known_obj_val for the instance. 
    """
    query = """
    SELECT
        i.id,
        MIN(ga.obj_val) as min_obj_val
    FROM
        instances i
    JOIN
        jobs j ON i.id = j.instance_id
    JOIN
        grb_attributes ga ON j.id = ga.job_id
    WHERE
        ga.sol_count > 0
    GROUP BY
        i.id;
    """

    con = get_connection()

    cursor = con.cursor()
    cursor.execute(query)
    results = cursor.fetchall()

    if not results:
        print("No solutions with sol_count > 0 found to update best objective values.")
        return

    for row in results:
        instance_id, min_obj_val = row
        print(f"Updating instance {instance_id} with best_known_obj_val = {min_obj_val}")
        cursor.execute(
            "UPDATE instances SET best_known_obj_val = ? WHERE id = ?",
            (min_obj_val, instance_id),
        )
    con.commit()
    print("Finished updating best known objective values.")
    close_connection() 