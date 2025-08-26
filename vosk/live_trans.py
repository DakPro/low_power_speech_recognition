# prerequisites: as described in https://alphacephei.com/vosk/install and also python module `sounddevice` (simply run command `pip install sounddevice`)
# Example usage using Dutch (nl) recognition model: `python test_microphone.py -m nl`
# For more help run: `python test_microphone.py -h`

import argparse
import queue
import sys
import sounddevice as sd
import json

from vosk import Model, KaldiRecognizer

q = queue.Queue()


def callback(indata, frames, time, status):
    if status:
        print(status, file=sys.stderr)
    q.put(bytes(indata))


def main(model: str = "en-us"):
    input_device = sd.query_devices(kind="input")
    if input_device is None:
        raise Exception("No input devices found. Available devices:\n", sd.query_devices())
    SAMPLE_RATE = input_device['default_samplerate']
    result = []

    print("Set device:", input_device['name'], "\nListening...")

    try:
        rec = KaldiRecognizer(Model(lang=model), SAMPLE_RATE)

        with sd.RawInputStream(samplerate=SAMPLE_RATE, blocksize=8000, device=input_device['index'],
                               dtype="int16", channels=1, callback=callback):
            while True:
                data = q.get()
                if rec.AcceptWaveform(data):
                    result.append(json.loads(rec.Result())['text'])
                # else:
                #   print(rec.PartialResult())
    except KeyboardInterrupt:
        return ''.join(result)
    except Exception as e:
        exit(type(e).__name__ + ": " + str(e))


if __name__ == '__main__':
    captions = main(model="en-us")
    print("Recognized captions:", captions)
