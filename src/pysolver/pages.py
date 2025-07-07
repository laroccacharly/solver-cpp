from typing import Callable
from pydantic import BaseModel
from pysolver.primal_gap_results import primal_gap_results_ui
from .all_tables import all_tables_ui
from .primal_gap_results import primal_gap_results_ui
from .run_sql import run_sql_ui

class Page(BaseModel): 
    name: str
    function: Callable


def get_pages() -> list[Page]: 
    from .instance_selection import instance_selection_ui
    return [
        Page(name="Instance Selection", function=instance_selection_ui),
        Page(name="Primal Gap Results", function=primal_gap_results_ui),
        Page(name="Tables", function=all_tables_ui),
        # Page(name="Run SQL", function=run_sql_ui),
    ]