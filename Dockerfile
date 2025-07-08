FROM python:3.12

ENV PYTHONUNBUFFERED=1 \
    PYTHONDONTWRITEBYTECODE=1

WORKDIR /app

# Install uv
RUN pip install uv
# Copy the project files
COPY data/ data/
COPY src/ src/
COPY ui.py .
COPY pyproject.toml .
RUN uv sync 
RUN uv pip install .  

EXPOSE 8501

CMD ["uv", "run", "streamlit", "run", "--server.address=0.0.0.0", "--server.port=8501", "ui.py"]