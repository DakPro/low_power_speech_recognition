from time import time
import librosa


def evaluate_on_audio(transcribe, audio):
    start_time = time()
    transcribe(audio)
    end_time = time()
    y, sr = librosa.load(audio)
    audio_duration = librosa.get_duration(y=y, sr=sr)
    return (end_time-start_time)/audio_duration


if __name__ == '__main__':
    pass
