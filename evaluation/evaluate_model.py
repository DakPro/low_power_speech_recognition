import argparse
import os
import re
import sys
from functools import partial

import WER

parent_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(parent_dir)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--model', type=str, help='model name')
    parser.add_argument('--wer', action='store_true', help='flag whether to evaluate WER')
    parser.add_argument('--all', action='store_true', help='flag whether to evaluate WER and ___ (TODO);'
                                                           'if used, --wer and other such flags have no effect')

    parser.add_argument('-s', '--streaming', action='store_true', help='flag whether to use streaming'
                                                                       'on WER evaluation')

    args = parser.parse_args()
    model_name = args.model
    f_wer = args.wer
    if args.all:
        f_wer = True

    # REMINDER: after implementing new metrix, add them to the if statement in the next line
    if not f_wer:
        f_wer = True

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
    if f_wer:
        WER.evaluate(transcribe, streaming=args.streaming)
