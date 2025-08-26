# Example usage using Dutch (nl) recognition model: `python test_microphone.py -m nl`
# For more help run: `python test_microphone.py -h`
import queue
import soundfile as sf
import json

from vosk import Model, KaldiRecognizer, SetLogLevel

SetLogLevel(-1)
rec = KaldiRecognizer(Model(lang="en-us"), 16000)
rec.SetWords(True)
rec.SetPartialWords(True)


def transcribe(audio: str) -> str:
    data, samplerate = sf.read(audio, dtype='int16')
    assert samplerate == 16000, print(samplerate)
    rec.AcceptWaveform(data.tobytes())
    result = json.loads(rec.Result())['text']
    return result


if __name__ == "__main__":
    transcribe('../evaluation/assets/sample_15.wav')
