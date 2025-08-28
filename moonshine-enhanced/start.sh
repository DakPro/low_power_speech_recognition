#!/bin/bash

DIR="$HOME/moonshine-enhanced"
if [[ ! -d "$DIR/.venv" ]]; then
  uv venv "$DIR/.venv"
fi
source "$DIR/.venv/bin/activate"
flag=uv pip install --dry-run --offline -r "$DIR/requirements.txt" | grep "Would make no changes"
if uv pip install --dry-run --offline -r "$DIR/requirements.txt" | grep -q "Would make no changes"; then
    uv pip install -r "$DIR/requirements.txt"
fi
MODEL_REQ="cat $DIR/model_requirements.txt"
if ! uv pip list --format=freeze | grep "$MODEL_REQ"; then
  uv pip install -r "$DIR/model_requirements.txt"
fi

uv run "$DIR/main.py"

