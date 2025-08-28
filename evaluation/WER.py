from datasets import load_dataset, Audio, Dataset, IterableDataset
from evaluate import load
import re
from itertools import islice
from typing import Tuple, List, Callable, Iterable, cast

wer = load("wer")

file = open("../.venv/huggingface_token")
huggingface_access_token = file.readline().strip()
file.close()

# List of used Datasets, can be expended by yourself.
# !!! When adding new datasets, please go through the dictionaries below.
availableDatasets = [
    "kensho/spgispeech",
    "distil-whisper/earnings22",
    "edinburghcstr/ami"
]

# Parameters for loading datasets from datasets library
datasetParams = {
    "kensho/spgispeech": {"name": "test", "token": huggingface_access_token, "split": "test",
                          "trust_remote_code": True},
    "distil-whisper/earnings22": {"name": "chunked", "split": "test"},
    "edinburghcstr/ami": {"name": "ihm", "split": "test", "trust_remote_code": True}
}

# Different datasets may have different column names
# By default: 'audio' for audio and 'transcript' for transcription
# !!! Please specify in the dictionary below if columns names differ
datasetColumnNames = {
    "distil-whisper/earnings22": {'transcript': 'transcription'},
    "edinburghcstr/ami": {'transcript': 'text'}
}

# Some datasets have unconventional format of transcripts
# !!! Please specify formating functions here
datasetFormatingFunction = {
    "edinburghcstr/ami": lambda text: re.sub(r"\b([A-Z])\b", r"\1.",
                                             re.sub(r"\s+", " ", re.sub(r"([.,!?])", r" \1 ", text.upper()))).strip()
}


class Counter:
    def __init__(self):
        self.count = 0

    def inc(self) -> None:
        self.count += 1
        if self.count % 10 == 0:
            print(self.count)


def prepare_dataset(datasetName: str, streaming=False) -> Iterable[Tuple[str, str]]:
    specialNames = datasetColumnNames[datasetName] if datasetName in datasetColumnNames else dict()

    print(f"Loading {datasetName} dataset...", end='')
    dataset = load_dataset(datasetName, **datasetParams[datasetName], streaming=streaming)
    print("Loaded.")

    dataset = dataset.rename_columns({specialNames[i]: i for i in specialNames})
    dataset.remove_columns(list(set(dataset.column_names) - {'audio', 'transcript'}))
    if dataset.features['audio'].sampling_rate != 16000:
        dataset.cast_column('audio', Audio(sampling_rate=16000))

    print(dataset)
    if isinstance(dataset, Dataset):
        return map(lambda x: (x['audio'], x['transcript']), dataset)

    elif isinstance(dataset, IterableDataset):
        return map(lambda x: (x['audio'], x['transcript']), dataset)

    else:
        raise Exception("Unexpected dataset type")


def compare(resultList: List[str]) -> float:
    predictedText, trueText = zip(*resultList)
    predictedText, trueText = list(predictedText), list(trueText)
    score = wer.compute(predictions=predictedText, references=trueText)
    return score


def evaluate_on_dataset(transcribe: Callable[[str], :str], datasetName, streaming: bool) -> float:
    preparedDataset = prepare_dataset(datasetName, streaming)
    processText = (lambda x: x) if datasetName not in datasetFormatingFunction else datasetFormatingFunction[
        datasetName]

    counter = Counter()

    def f(x):
        counter.inc()
        audio_path = x[0]['path']
        # if not re.search(r"\.cache", audio_path): audio_path = ""
        predictedText = processText(transcribe(audio_path))
        trueText = x[1]
        return predictedText, trueText

    processedDataset = list(islice(map(f, preparedDataset), 100))
    accuracy = compare(processedDataset)
    return accuracy


def evaluate(transcribe: Callable[[str], str], streaming=False) -> None:
    for datasetName in availableDatasets:
        accuracy = evaluate_on_dataset(transcribe, datasetName, streaming)
        print("WER on", datasetName, ":", accuracy)


if __name__ == "__main__":
    from moonshine.src.file_trans import transcribe as trans

    evaluate(trans)
