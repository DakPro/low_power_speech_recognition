# Accuracy evaluation

This folder contains simple interface for accuracy evaluation of the models.

<code>main(transcribe)</code> - argument: a function taking
***numpy arrays*** and returning transcribed speech, in format of
<code>[['Sentence 1'],['Sentence 2'],...]</code>. TODO: change
desired format of the transcription. 

<code>eval.py</code> can also be run as main file. This will benchmark
<code>moonshine</code>


## Metrics used

----

WER - word error rate, more details 
[here](https://huggingface.co/spaces/evaluate-metric/wer).

The evaluation implementation is <code>evaluate</code> library. 

## Evaluation Datasets
Three datasets were chosen for the model evaluation:
* [SPGISpeech](https://huggingface.co/datasets/kensho/spgispeech) (currently not used due to usage issues)
* [Earnings-22](https://huggingface.co/datasets/distil-whisper/earnings22)
* [AMI](https://huggingface.co/datasets/edinburghcstr/ami)

(useless for now) SPGISpeech requires user's consent, so before using it make sure
you are using authentication token.

*Note: initially used revdotcom/earnings22 dataset but due issues with the dataset (no dataset card,
data being absent), a decision was made to switch to distil-whisper/earnings22*

