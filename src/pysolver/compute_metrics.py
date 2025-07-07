from .set_best_obj_val import set_best_obj_val
from .set_primal_gap import set_primal_gap

def compute_metrics():
    set_best_obj_val()
    set_primal_gap()

if __name__ == "__main__":
    compute_metrics()