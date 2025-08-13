from datasets import load_dataset, Audio
from evaluate import load
import itertools
import tempfile
import soundfile as sf

wer = load("wer")

file = open(".venv/huggingface_token")
huggingface_access_token = file.readline().strip()
file.close()

availableDatasets = [
    "kensho/spgispeech",
    "distil-whisper/earnings22",
    "edinburghcstr/ami"
]
datasetParams = {
    "kensho/spgispeech": {"name": "test", "token": huggingface_access_token, "split": "test", "streaming": True},
    "distil-whisper/earnings22": {"name": "chunked", "split": "test", "streaming": True},
    "edinburghcstr/ami": {"name": "ihm", "split": "test", "streaming": True, "trust_remote_code": True}
}
datasetColumnNames = {
    "kensho/spgispeech": ('audio', 'transcript'),
    "distil-whisper/earnings22": ('audio', 'transcription'),
    "edinburghcstr/ami": ('audio', 'text')
}


def prepare_dataset(datasetName):
    dataset = load_dataset(datasetName, **datasetParams[datasetName])
    dataset.cast_column('audio', Audio(sampling_rate=16000))
    print("Dataset structure:", dataset)
    return map(lambda x: [x[i] for i in datasetColumnNames[datasetName]], dataset)


def compare(lazyListResult):
    audioLimit = 10
    lazyListResult = list(itertools.islice(lazyListResult, audioLimit))
    print(lazyListResult)
    predictedText = list(map(lambda x: x[0], lazyListResult))
    trueText = list(map(lambda x: x[1], lazyListResult))
    score = wer.compute(predictions=predictedText, references=trueText)
    return score


def array_into_file(audio_array, sr):
    # Create temporary wav file
    with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as tmp:
        sf.write(tmp.name, audio_array, sr)
        tmp_path = tmp.name
    return tmp_path


def evaluate_on_dataset(transcribe, datasetName):
    preparedDataset = prepare_dataset(datasetName)

    def f(x):
        aud = array_into_file(x[0]['array'], 16000)
        transcript = x[1]
        return transcribe(aud), transcript

    processedDataset = map(f, preparedDataset)
    accuracy = compare(processedDataset)
    return accuracy


def main(transcribe):
    for datasetName in availableDatasets:
        accuracy = evaluate_on_dataset(transcribe, datasetName)
        print("WER on", datasetName, ":", accuracy)


if __name__ == "__main__":
    from moonshine.src.file_trans import transcribe as trans

    main(trans)
