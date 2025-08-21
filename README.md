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

*RTF* - Real-Time Factor = transcription time / audio length.

*sRTF* - Streaming Real-Time Factor =  transcription time / speech length.

*UPL* - User Perceived Latency, or latency = average time it takes for a spoken word to get displayed. For models with
segmental transcription, the result for the start of a segment is taken.   

| Model               | sRTF on rPi 4 | RTF on rPi 4 | UPL*, seconds | WER on SPGISpeech | WER on Earnings22 | WER on AMI | 
|---------------------|---------------|--------------|---------------|-------------------|-------------------|------------| 
| Moonshine  (base)   | 0.28-0.35 (1) | 0.64         | 0.2 + 0.2×RTF |                   |                   |            | 
| Whisper.cpp (tiny)  | 30xRTF/X **   | 0.39 (2)     | X + 30×RTF    |                   |                   |            |
| Whisper.cpp (base)  | same as above | 0.74 (2)     | same as above |                   |                   |            |
| Whisper.cpp (small) | same as above | 2.44 (2)     | same as above |                   |                   |            |
| Kuytai              | \>1           | 10           | not feasible  |                   |                   |            |

UPL* - listed UPLs are theoretical and valid only if RTF on device is < 1. 
Formulas for values are given using standard configuration of models (read below for more details).

** - streaming on whisper model works by recording audio for X (default: 0.5) seconds and then 
transcribing it. The architecture transcribes audio in 30 s chunks only, thus sRTF depends on X.
In combination with UPL explanation, X just defines balance between UPL and sRTF.

(1) - Lower RTFs were observed on more consistent/faster speeches.
(2) - RTF for whisper depends on length of an audio being transcribed. The RTF evaluation is done on 90s audio sample,
as due to "30s-input-only" architecture constraint whisper gives highest RTFs on audios with length divisible by 30.

#### Explanation to UPLs

* Moonshine - streaming on moonshine works by accumulating audio chunks, until it reaches
X seconds (default - 0.2), and then transcribes the batch and displays last line of 
transcriptions buffer. X is [customizable](https://github.com/DakPro/low_power_speech_recognition/tree/main/moonshine/src/live_trans.py#L20),
but shouldn't be made very small due to increased overhead of initiating transcription.

* Whisper.cpp - streaming on whisper model works by recording audio for 1/2 (customizable) a sec and then 
transcribing it. The architecture transcribes audio in 30 s chunks only, thus the higher
the computational resources, the lower the latency.