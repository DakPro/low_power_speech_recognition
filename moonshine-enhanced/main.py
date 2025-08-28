from src.live_trans import TranscriptionProcess
import re

class SpeechProcessor:
    def __init__(self):
        self.recording_file = None
        self.recording = False
        self.recording_file_name = 'recording.txt'
    def process_speech(self, speech: str | None) -> None:
        if speech is None:
            pass

        if self.recording:
            self.recording_file.write(speech)
        if re.match(r'start recording(.)*$', speech, re.IGNORECASE):
            self.recording = True
            self.recording_file = open(self.recording_file_name, 'a+t')
        if re.match(r'stop recording(.)*$', speech, re.IGNORECASE):
            self.recording = False
            self.recording_file.close()
        print(speech)


if __name__ == "__main__":
    speechProcessor = SpeechProcessor()
    process = TranscriptionProcess(speechProcessor.process_speech)
    process.start(print_transcriptions=True)
