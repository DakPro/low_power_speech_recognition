#!/bin/bash

DIR="$HOME/moonshine-enhanced"
if [ ! -d "$DIR/.venv" ]; then
  uv venv "$DIR/.venv"
fi
source "$DIR/.venv/bin/activate"
if ! uv pip install -r "$DIR/requirements.txt" --dry-run --no-index; then
    uv pip install -r "$DIR/requirements.txt"
fi
uv run "$DIR/main.py"

