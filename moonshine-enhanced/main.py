from src.live_trans import TranscriptionProcess

process = TranscriptionProcess()
process.start(print_transcriptions=True)
print("Process ended")