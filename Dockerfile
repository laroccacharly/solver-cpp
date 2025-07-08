FROM python:3.12

ENV PYTHONUNBUFFERED=1 \
    PYTHONDONTWRITEBYTECODE=1

WORKDIR /app

COPY data/ data/
# Install uv
RUN pip install uv
# Copy the project files
COPY pyproject.toml .
RUN uv sync 

COPY src/ src/
COPY ui.py .
RUN uv pip install . 

EXPOSE 8501

CMD ["uv", "run", "streamlit", "run", "--server.address=0.0.0.0", "--server.port=8501", "ui.py"]