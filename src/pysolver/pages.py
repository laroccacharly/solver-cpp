from typing import Callable
from pydantic import BaseModel
from .instance_selection import instance_selection_ui
from .all_tables import all_tables_ui

class Page(BaseModel): 
    name: str
    function: Callable


def get_pages() -> list[Page]: 
    return [
        Page(name="Instance Selection", function=instance_selection_ui),
        Page(name="Tables", function=all_tables_ui),
    ]