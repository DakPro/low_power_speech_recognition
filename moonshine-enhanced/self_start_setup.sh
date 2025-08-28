#!/bin/bash

# Running this file makes the program run automatically after OS load

if ! command -v uv &> /dev/null; then
  curl -LsSf https://astral.sh/uv/install.sh | sh
fi
echo "./moonshine-enhanced/start.sh" >> .bashrc
./start.sh