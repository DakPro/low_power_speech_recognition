import subprocess
from pathlib import Path

import numpy
import soundfile as sf
import tempfile
import subprocess


def preheat():
    subprocess.run("./src/prepare.sh", stdout=subprocess.DEVNULL)


def array_into_file(audio_array, sr):
    # Create temporary wav file
    with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as tmp:
        sf.write(tmp.name, audio_array, sr)
        tmp_path = tmp.name
    return tmp_path


def transcribe(audio):
    if isinstance(audio, numpy.ndarray):
        audio = array_into_file(audio, 16000)
    if isinstance(audio, (str, Path)):
        completedProcess = subprocess.run(args=["-f", "samples/jfk.wav"], executable="./src/build/bin/whisper-cli")
        if completedProcess.returncode != 0:
            raise Exception("Error in running whisper_cpp:\n", completedProcess.stderr)
        return completedProcess.stdout
    else:
        raise Exception("Audio has unexpected type:", type(audio))


preheat()

if __name__ == "__main__":
    pass
