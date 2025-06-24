# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "pandas",
#     "plotly",
# ]
# ///

import pandas as pd
import plotly.express as px


def plot(): 
    filename = "metrics.csv"
    df = pd.read_csv(filename)
    fig = px.line(df, x="elapsed_ms", y="non_zero_count", color="phase")
    fig.show()

plot()