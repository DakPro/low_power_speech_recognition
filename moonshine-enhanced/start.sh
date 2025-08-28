#!/bin/bash

DIR="$HOME/moonshine-enhanced"
if [ ! -d "$DIR/.env" ]; then
  uv venv
fi
source "$DIR/.venv/bin/activate"
if [ -n "$(uv sync --dry-run)" ]; then
  uv pip install -r DIR/requirements.txt
fi
uv run "$DIR/main.py"

