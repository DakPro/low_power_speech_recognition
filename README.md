# Speech recognition models on low-power devices

This repository is storage for code used in my summer research project

## Real-time transcription from mic

To test real-time transcription from mic: <code>python3 
transcription_from_mic.py</code>. This will run moonshine model by default.
For more details access READMEs in directories corresponding to models.

## Evaluation on datasets

To evaluate models performance, run
``` bash
uv run evaluate_model.py --model moonshine
```

Available models can be seen in the <code>evaluate_model.py</code>. To use with
yet unimplemented models, add an if-statement which imports <code>transcribe</code>
from the model.

For further details, see evaluation/README.md
