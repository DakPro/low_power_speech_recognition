import argparse
import moonshine.src.live_transcription as live_transcription

SUPPORTED_MODELS = ["moonshine"]
MODEL = "moonshine"
DEFAULT_MODEL_SIZES = {"moonshine": "moonshine/base"}

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        prog="transcription_from_mic",
        description="Demo of transcription from default mic device"
    )
    parser.add_argument(
        "-m", "--model",
        help=f"Model to run. Default: moonshine.",
        default="moonshine",
        choices=SUPPORTED_MODELS
    )
    args = parser.parse_args()
    model = args.model

    if model == 'moonshine':
        live_transcription.main(DEFAULT_MODEL_SIZES[model], include_captions=True)
