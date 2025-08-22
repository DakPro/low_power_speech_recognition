# Moonshine

Moonshine is a family of speech-to-text models
optimized for fast and accurate automatic speech 
recognition (ASR) on resource-constrained devices. 
please refer to the [project repo on GitHub](https://github.com/usefulsensors/moonshine).

This package uses <code>moonshine-onnx</code> pipeline and <code>moonshine/base</code>
model by default.

## Structure

<code>src</code> contains code for running on low-power devices

<code>apple_src</code> contains code for running on PCs, which is used for
evaluation of WER on a database. *The directory contains additional requirements
to be installed*.