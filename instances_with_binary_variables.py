# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "miplib-benchmark",
# ]
# ///

import miplib_benchmark
import polars as pl

instances: pl.DataFrame = miplib_benchmark.get_instances()
instances_head = instances.head(10)
columns = instances_head.columns
print(columns)
print(instances_head)

# filter out instances where n_binary_variables is 0
instances = instances.filter(pl.col('n_binary_variables') > 0)
instance_count_remaining = instances.height
print(f"Instance count remaining: {instance_count_remaining}")

instance_names = instances.select(pl.col('instance_name'))
# save to txt file
instance_names.write_csv('data/instance_names.txt', include_header=False)