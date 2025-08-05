---
license: apache-2.0
datasets:
- projectlosangeles/Los-Angeles-MIDI-Dataset
library_name: transformers
tags:
- music
- midi
- music generation
- midi generation
---
# Model Card for midi-model

Midi event transformer for music generation

### Model Updates

- tv2o-medium model: MidiTokenizerV2. [skytnt/midi-model-tv2o-medium](https://huggingface.co/skytnt/midi-model-tv2o-medium)
- v1.2 : Optimise the tokenizer and dataset. The dataset was filtered by MIDITokenizer.check_quality. Using the higher quality dataset to train the model, the performance of the model is significantly improved.

## Model Details

### Model Description

- **Developed by:** SkyTNT
- **Model type:** Transformer
- **License:** apache-2.0

### Model Sources

- **Repository:** https://github.com/SkyTNT/midi-model
- **Demo:** https://huggingface.co/spaces/skytnt/midi-composer


## Training Details

### Training Data

- [projectlosangeles/Los-Angeles-MIDI-Dataset](https://huggingface.co/datasets/projectlosangeles/Los-Angeles-MIDI-Dataset)

#### Training Hyperparameters

- basemodel: v1.1
- lr: 1e-4
- weight-decay: 0.01
- batch: 2x2x2
- fp16 mixed precision

####  Loss
 - val loss: 0.238

## Files

- model.ckpt: latest model checkpoint
- soundfont.sf2: soundfont file, i put it here for convenient downloading , [source](https://github.com/musescore/MuseScore/tree/master/share/sound)
- onnx/*.onnx: onnx format model
