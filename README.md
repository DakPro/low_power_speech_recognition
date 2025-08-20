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


## Table of evaluated models

| Model       | RTF on rPi 4 | UPL*, seconds | WER on SPGISpeech | WER on Earnings22 | WER on AMI | 
|-------------|--------------|---------------|-------------------|-------------------|------------| 
| Moonshine   | 2.8-3.5 (1)  | 0.2 + 0.2×RTF |                   |                   |            | 
| Whisper.cpp |              | 0.5 + 30×RTF  |                   |                   |            |
| Kuytai      |              |               |                   |                   |            |

UPL* : listed UPLs are theoretical and valid only if RTF on device is < 1. 
Formulas for values are given using standard configuration of models (read below for more details).

(1) - Higher RTFs were observed on more consistent/faster speeches.

#### Explanation to UPLs

* Moonshine - streaming on moonshine works by accumulating audio chunks, until it reaches
X seconds (default - 0.2), and then transcribes the batch and displays last line of 
transcriptions buffer. X is [customizable](https://github.com/DakPro/low_power_speech_recognition/tree/main/moonshine/src/live_trans.py#L20).

* Whisper.cpp - streaming on whisper model works by recording audio for 1/2 a sec and then 
transcribing it. The architecture transcribes audio in 30 s chunks, thus the higher
the computational resources, the lower the latency.