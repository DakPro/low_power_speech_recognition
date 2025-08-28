#!/bin/bash

DIR="$HOME/moonshine-enhanced"
if [ ! -d "$DIR/.venv" ]; then
  uv venv "$DIR/.venv"
fi
source "$DIR/.venv/bin/activate"
pkgs=$(grep -v "^#\|^$" "$DIR/requirements.txt" | sed 's/@.*//' | sed 's/[<>=].*//' | tr '\n' '|' | sed 's/|$//')
if ! uv pip list --format=freeze | grep -q "$pkgs"; then
  echo "Something not found. Installing requirements"
  uv pip install -r "$DIR/requirements.txt"
fi
uv run "$DIR/main.py"

