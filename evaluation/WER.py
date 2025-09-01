import numpy
from datasets import load_dataset, Audio, Dataset, IterableDataset
from evaluate import load
import re
from typing import Tuple, List, Callable, Iterable, cast
from concurrent.futures import ProcessPoolExecutor, ThreadPoolExecutor
import os,sys

parent_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(parent_dir)

wer = load("wer")

file = open("../.venv/huggingface_token")
huggingface_access_token = file.readline().strip()
file.close()

# List of used Datasets, can be expended by yourself.
# !!! When adding new datasets, please go through the dictionaries below.
availableDatasets = [
    "kensho/spgispeech",
    # "distil-whisper/earnings22",
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

rows_cnt = {
    "kensho/spgispeech": 39341
}


class Counter:
    def __init__(self):
        self.count = 0

    def inc(self) -> None:
        self.count += 1
        if self.count % 10 == 1:
            print(self.count)


def format_dataset(datasetName: str, dataset: Dataset | Iterable) -> Dataset | IterableDataset:
    specialNames = datasetColumnNames[datasetName] if datasetName in datasetColumnNames else dict()
    dataset = (dataset.rename_columns({specialNames[i]: i for i in specialNames})
               .remove_columns(list(set(dataset.column_names) - {'audio', 'transcript'})))
    if dataset.features['audio'].sampling_rate != 16000:
        dataset.cast_column('audio', Audio(sampling_rate=16000))
    return dataset


def prepare_slice(datasetName: str, start: int | None, end: int | None) -> Dataset:
    dataset = format_dataset(datasetName, load_dataset(datasetName, **datasetParams[datasetName]))
    datasetSlice = dataset[start:end]
    # Possible: return LIST OF TUPLES or even TUPLE OF LISTS instead of database object
    return datasetSlice


def prepare_iter(datasetName: str) -> IterableDataset:
    dataset = format_dataset(datasetName,
                             load_dataset(datasetName, **datasetParams[datasetName], streaming=True))
    return dataset


def evaluate_on_iter(transcribe: Callable[[str | numpy.ndarray], str], datasetName: str, threads: int) -> float:
    preparedDataset = prepare_iter(datasetName)
    processText = (lambda x: x) if datasetName not in datasetFormatingFunction else datasetFormatingFunction[
        datasetName]

    counter = Counter()

    def f(x) -> Tuple[str, str]:
        counter.inc()
        return processText(transcribe(x['audio']['array'])), x['transcript']

    with ThreadPoolExecutor(max_workers=threads) as executor:
        result = list(executor.map(f, preparedDataset))

    predictedTranscript, trueTranscript = zip(*result)
    return wer.compute(predictions=predictedTranscript, references=trueTranscript)


def evaluate_on_slice(transcribe: Callable[[str], str], datasetName: str,
                      threads: int = 4, start: int | None = None, end: int | None = None) -> float:
    datasetSlice = prepare_slice(datasetName, start, end)
    print(type(datasetSlice))
    processText = (lambda x: x) if datasetName not in datasetFormatingFunction else datasetFormatingFunction[
        datasetName]

    with ThreadPoolExecutor(max_workers=threads) as executor:
        predictedTranscriptions = list(executor.map(lambda x: processText(transcribe(x['path'])),
                                                    datasetSlice['audio']))
    return wer.compute(predictions=predictedTranscriptions, references=datasetSlice['transcript'])


def evaluate(transcribe: Callable[[str | numpy.ndarray], str], streaming=False, threads: int = 4) -> None:
    for datasetName in availableDatasets:
        accuracy = evaluate_on_iter(transcribe, datasetName, threads) if streaming \
            else evaluate_on_slice(transcribe, datasetName, threads)
        print("WER on", datasetName, ":", accuracy)


if __name__ == "__main__":
    from moonshine.src.file_trans import transcribe as tr

    evaluate_on_slice(tr, "kensho/spgispeech", 4, 1)
