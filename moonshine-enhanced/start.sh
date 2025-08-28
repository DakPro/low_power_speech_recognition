#!/bin/bash

DIR="$HOME/moonshine-enhanced"
if [ ! -d "$DIR/.env" ]; then
  uv venv
fi
source "$DIR/.venv/bin/activate"
uv pip install -r requirements.txt --dry-run --no-index
if [ $? -eq 1 ]; then
  uv pip install -r requirements.txt
fi
uv run "$DIR/main.py"

