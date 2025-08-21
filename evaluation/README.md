# Accuracy evaluation

This folder contains simple interface for accuracy evaluation of the models.

*Notes for arguments of functions:*

<code>transcribe</code> - a function that represents a model. The function must take audiofile path and
return transcribed speech, in format of <code>['Sentence 1','Sentence 2',...]</code>. 

<code>audio</code> - TODO


* <code>WER.evaluate(transcribe)</code> - prints WER of a model for some datasets 
(more details [here](#evaluation-datasets))


* <code>WER.py</code> can also be run as main file. This will benchmark
<code>moonshine</code>


* <code> RTF.evaluate(transcribe, audio)</code> - prints RTF of a model evaluated no the audio.


* <code>assets/</code> - folder containing audios used for RTF of some models. One can find it useful for evaluation
of their own models. You can trim an audio with <code>sox <input_file> <output_file> trim \<start> \<duration> </code>
*(Note: <code>sox</code> package is needed for that)*, or even record your own with <code>sox -d <output_file></code>.

## Metrics used

----

WER - word error rate, more details 
[here](https://huggingface.co/spaces/evaluate-metric/wer).

The evaluation implementation is <code>evaluate</code> library. 

## Evaluation Datasets
Three datasets were chosen for the model evaluation:
* [SPGISpeech](https://huggingface.co/datasets/kensho/spgispeech)
* [Earnings-22](https://huggingface.co/datasets/distil-whisper/earnings22)
* [AMI](https://huggingface.co/datasets/edinburghcstr/ami) - still to be
decided whether to use as uses capital case no punctuation.

SPGISpeech requires user's consent, so before using it make sure
you are using authentication token.

*Note: initially used revdotcom/earnings22 dataset but due issues with the dataset (no dataset card,
data being absent), a decision was made to switch to distil-whisper/earnings22*

