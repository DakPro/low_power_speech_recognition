import argparse
import os
import subprocess
from evaluation.WER import evaluate as evaluate
from functools import partial
import re

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--model', type=str, help='model name')
    model_name = parser.parse_args().model

    # moonshine
    if model_name == "moonshine":
        from moonshine.src.file_trans import transcribe
    elif re.match("moonshine/.*", model_name):
        from moonshine.src.file_trans import transcribe as _transcribe
        transcribe = partial(_transcribe, model=model_name)

    # whisper_cpp
    elif model_name == "whisper_cpp" or model_name == "whisper_cpp/base":
        from whisper_cpp.file_trans import transcribe
    elif re.match("whisper_cpp/.*$", model_name):
        from whisper_cpp.file_trans import transcribe as _transcribe
        transcribe = partial(_transcribe, model=re.match("whisper_cpp/(.*)$", model_name).group(0))

    # noinspection PyUnboundLocalVariable
    evaluate(transcribe)
