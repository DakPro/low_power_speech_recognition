import os
from pathlib import Path
import subprocess

LOCAL_PATH = Path(__file__).parent.joinpath("src")


def preheat() -> None:
    OLD_PATH = os.getcwd()
    os.chdir(LOCAL_PATH)
    subprocess.run(args=[], executable=f"{LOCAL_PATH}/prepare.sh", stdout=subprocess.DEVNULL)
    os.chdir(OLD_PATH)


def transcribe(audio: str, model="base") -> str:
    if isinstance(audio, (str, Path)):
        OLD_PATH = os.getcwd()
        os.chdir(LOCAL_PATH)
        completedProcess = subprocess.run(["build/bin/whisper-cli", "-m", f"models/ggml-{model}.bin",
                                           "--no-timestamps", "-f", audio],
                                          stderr=subprocess.DEVNULL, stdout=subprocess.PIPE, text=True)
        os.chdir(OLD_PATH)
        if completedProcess.returncode != 0:
            raise Exception("Error in running whisper_cpp:\n Code:", completedProcess.returncode,
                            "\n Message:", completedProcess.stderr)
        return completedProcess.stdout
    else:
        raise Exception("Audio has unexpected type:", type(audio))


preheat()
