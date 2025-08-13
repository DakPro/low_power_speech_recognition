import argparse
import os
import subprocess
from evaluation.eval import main as evaluate

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--model', type=str, help='model name')
    model_name = parser.parse_args().model

    if model_name == "moonshine":
        from moonshine.src.file_trans import transcribe
    elif model_name == "whisper_cpp":
        from whisper_cpp.file_trans import transcribe

    evaluate(transcribe)