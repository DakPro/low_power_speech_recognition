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


def transcribe(audio: str, model_name: str = "moonshine/base") -> str:
    try:
        model = models[model_name]
    except KeyError as e:
        if isinstance(model_name, str):
            model = MoonshineOnnxModel(model_name=model_name)
        else:
            raise Exception(f"Expected a model name, not a {type(model_name)}")
        models[model_name] = model

    audio = load_audio(audio)
    assert_audio_size(audio)

    tokens = model.generate(audio)
    ans = tokenizer.decode_batch(tokens)
    ans = '' if ans is None else ans[0]  # keeping the desired format
    return ans


def benchmark(audio: str, model: str = "moonshine/base") -> None:
    import time
    model = MoonshineOnnxModel(model_name=model)

    audio = load_audio(audio)
    num_seconds = assert_audio_size(audio)

    print("Warming up...")
    for _ in range(4):
        _ = model.generate(audio)

    print("Benchmarking...")
    N = 8
    start_time = time.time_ns()
    for _ in range(N):
        _ = model.generate(audio)
    end_time = time.time_ns()

    elapsed_time = (end_time - start_time) / N
    elapsed_time /= 1e6

    print(f"Time to transcribe {num_seconds:.2f}s of speech is {elapsed_time:.2f}ms")
