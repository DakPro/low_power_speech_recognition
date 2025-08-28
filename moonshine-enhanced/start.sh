#!/bin/bash

DIR="$HOME/moonshine-enhanced"
if [ ! -d "$DIR/.venv" ]; then
  uv venv
fi
echo "venv created"
source "$DIR/.venv/bin/activate"
echo "venv activated"
if ! uv pip install -r requirements.txt --dry-run --no-index; then
    uv pip install -r requirements.txt
fi
echo "requirements installed; running demo"
uv run "$DIR/main.py"

