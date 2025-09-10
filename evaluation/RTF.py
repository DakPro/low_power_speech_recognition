import pathlib
from time import time
import librosa
import os
from typing import *
import matplotlib.pyplot as plt
import numpy as np

ASSETS_DIR = os.path.dirname(os.path.abspath(__file__)) + "/assets/"


def evaluate_on_audio(transcribe: Callable[[str], str], audio: str):
    start_time = time()
    transcribe(audio)
    end_time = time()
    y, sr = librosa.load(audio)
    audio_duration = librosa.get_duration(y=y, sr=sr)
    return (end_time - start_time) / audio_duration


def evaluate(transcribe: Callable[[str], str]):
    names = list(filter(lambda x: x.endswith(".wav"), os.listdir(ASSETS_DIR)))
    results = []
    for name in names:
        i = int(name.strip("sample_.wav"))
        results.append((i, evaluate_on_audio(transcribe, ASSETS_DIR + name)))
    x, y = zip(*results)
    plt.plot(np.ndarray(x), np.ndarray(y))
    plt.show()
    plt.savefig("m")

if __name__ == '__main__':
    from moonshine.src.file_trans import transcribe

    evaluate(transcribe)
