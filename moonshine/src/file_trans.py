from pathlib import Path

import numpy
from tokenizers import Tokenizer
from .model import MoonshineOnnxModel

from . import ASSETS_DIR


def load_audio(audio: str | Path) -> numpy.ndarray:
    import librosa
    audio, _ = librosa.load(audio, sr=16_000)
    return audio[None, ...]


def assert_audio_size(audio: numpy.ndarray) -> float:
    assert len(audio.shape) == 2, "audio should be of shape [batch, samples]"
    num_seconds = audio.size / 16000
    assert 0.1 < num_seconds < 64, (
        "Moonshine models support audio segments that are between 0.1s and 64s in a single transcribe call. "
        "For transcribing longer segments, pre-segment your audio and provide shorter segments."
    )
    return num_seconds


def load_tokenizer() -> Tokenizer:
    tokenizer_file = ASSETS_DIR / "tokenizer.json"
    return Tokenizer.from_file(str(tokenizer_file))


models = {}
tokenizer = load_tokenizer()


def transcribe(audio: str | numpy.ndarray, model_name: str = "moonshine/base") -> str:
    try:
        model = models[model_name]
    except KeyError as e:
        if isinstance(model_name, str):
            model = MoonshineOnnxModel(model_name=model_name)
        else:
            raise Exception(f"Expected a model name, not a {type(model_name)}")
        models[model_name] = model

    audio = audio if isinstance(audio, numpy.ndarray) else load_audio(audio)
    assert_audio_size(audio)

    tokens = model.generate(audio)
    ans = tokenizer.decode_batch(tokens)
    ans = '' if ans is None else ans[0]  # keeping the desired format
    return ans

