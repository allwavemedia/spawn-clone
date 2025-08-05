# Research Report: Evaluation of Open-Source Music Generation Models for SpawnClone Quality Mode

**DATE:** 2025-08-05

### **Executive Summary**

This report presents a comprehensive evaluation of open-source MIDI and music generation models to identify the most suitable candidate for the "Quality Mode" feature of SpawnClone, an AI-powered VST plugin. The evaluation framework is based on critical requirements outlined in the SpawnClone Product Requirements Document (PRD), including commercial licensing compatibility, seamless integration with ONNX Runtime for on-device inference, manageable model size for local caching, and the ability to generate professional-quality, musically coherent MIDI patterns.

The analysis covers several prominent model families: Google's Magenta models (MusicVAE, Performance RNN), the Music Transformer architecture, and OpenAI's MuseNet with its open-source derivatives. Each model was assessed against the established criteria using the provided research data.

The findings indicate that while Google Magenta models offer high-quality generation and a permissive Apache 2.0 license, they lack native ONNX support, requiring manual conversion and potentially significant development overhead. Conversely, OpenAI's MuseNet is unsuitable due to restrictive commercial use policies on the original model and licensing ambiguity in its derivatives.

The primary recommendation of this report is the adoption of a **Music Transformer** model, sourced from a repository like Hugging Face. This architecture demonstrates superior alignment with all project requirements. It offers state-of-the-art generation quality, is supported by a robust ecosystem for ONNX export and optimization, and benefits from ONNX Runtime's specific performance enhancements for transformer models, which have been benchmarked to provide significant speedups in C++ environments. Models are available under commercial-friendly licenses, and their size can be effectively managed through quantization. The Music Transformer architecture represents the most strategic, lowest-risk, and highest-potential option for implementing a sophisticated and performant Quality Mode in SpawnClone.

### **1. Introduction**

The SpawnClone project aims to develop a free, open-source alternative to commercial AI-powered MIDI generation plugins. A core component of its value proposition, as detailed in Epic 6 of the Product Requirements Document (PRD), is a multi-tiered AI generation system. This system includes a "Fast Mode" using rule-based algorithms and a more advanced "Quality Mode" designed to leverage on-device machine learning models for enhanced musical coherence and sophistication. According to requirement FR18, this Quality Mode must utilize MIDI-native Transformer models and deliver generated patterns within a 3-5 second timeframe.

The successful implementation of Quality Mode is contingent upon selecting an appropriate open-source model that not only generates high-quality musical patterns but also adheres to strict technical and legal constraints. These constraints, derived from the PRD, include compatibility with the ONNX Runtime for local inference (FR21), a model size suitable for user download and caching (FR23), and a commercial-use-compatible license that aligns with the project's open-source distribution goals.

This research report provides a detailed evaluation of leading open-source music generation models to determine the optimal choice for SpawnClone's Quality Mode. The analysis assesses each candidate model against a rigorous framework of technical feasibility, licensing permissiveness, and generation quality to provide a clear, evidence-based recommendation for the SpawnClone development team.

### **2. Evaluation Framework**

To ensure a thorough and relevant assessment, each candidate model was evaluated against a set of criteria derived directly from the SpawnClone PRD and the technical realities of developing a real-time audio plugin.

**Commercial Licensing Compatibility:** The model and its pre-trained weights must be available under a permissive open-source license, such as Apache 2.0 or MIT, that explicitly allows for modification, distribution, and inclusion in a commercial or freely distributed product without imposing viral licensing terms or use-based restrictions. This is critical for the project's goal of creating a free, open-source tool.

**ONNX Runtime Integration:** As per requirement FR21, the model must be compatible with ONNX Runtime for on-device inference. This criterion assesses the ease of converting the model to the ONNX format and the availability of tools and documentation to support this process. Models with native or well-supported ONNX export pipelines are heavily favored, as this significantly reduces development risk and integration time. Performance benchmarks related to ONNX Runtime are also considered.

**Model Size and Performance:** The model's file size is a crucial factor for a plugin that requires users to download and cache models for offline use (FR23). The evaluation considers the on-disk size of pre-trained models and the potential for size reduction through techniques like quantization. Furthermore, the model's inference speed is assessed against the 3-5 second generation time target specified in FR18, with a focus on performance within a C++ environment typical for VST plugins.

**MIDI Generation Quality:** The primary function of the model is to generate musically coherent and professional-quality MIDI patterns suitable for genres like Hip Hop, Pop, and EDM (FR14). This criterion evaluates the model's architecture, its training data, and its demonstrated ability to produce structured, expressive, and musically interesting melodies, chords, and basslines.

**VST Plugin Integration Suitability:** This considers the overall fitness of the model for integration into a C++ JUCE application. This includes the availability of C++ APIs for inference, the complexity of the pre-processing and post-processing pipelines, and the model's memory footprint during operation, which must not compromise the real-time safety of the audio thread (NFR3, NFR6).

### **3. Analysis of Candidate Models**

This section provides a detailed analysis of each potential model family, assessed against the evaluation framework.

#### **3.1. Google Magenta: MusicVAE**

MusicVAE is a hierarchical recurrent variational autoencoder developed by the Google Magenta project, designed to learn latent spaces of musical sequences. It excels at tasks like interpolation between musical styles, generating melodies, and creating drum patterns.

Regarding **commercial licensing**, MusicVAE is part of the open-source Magenta project and is governed by the Apache 2.0 license. This permissive license allows for commercial use, modification, and distribution, making it fully compatible with SpawnClone's objectives. The open-source nature of the project encourages broad adoption in both academic and commercial applications.

The model's integration with **ONNX Runtime** presents a notable challenge. The research indicates no official or community-provided ONNX version of MusicVAE. The model is primarily implemented in TensorFlow. While conversion is theoretically possible using tools like `tf2onnx`, this would be a manual process requiring the development team to manage the export, validate the converted model's outputs, and potentially debug unsupported TensorFlow operations. This contrasts with models that have a streamlined, officially supported export pipeline and introduces a significant development and maintenance burden.

In terms of **model size and performance**, specific file sizes in megabytes are not explicitly stated in the provided data. However, based on analogous quantized LSTM models, it can be inferred that checkpoints are likely in the range of tens of megabytes, such as 39 MB for a comparable model. This size is generally acceptable for the download and caching mechanism planned for SpawnClone. Performance-wise, as a VAE with recurrent components, its inference speed would need to be benchmarked, but its use in real-time web demos suggests it is reasonably efficient. However, high memory consumption during inference with ONNX Runtime has been noted for similar models, which could be a concern for VST plugin stability.

The **MIDI generation quality** of MusicVAE is well-regarded. Its architecture, featuring bidirectional LSTM encoders and hierarchical decoders, is specifically designed to handle long-term musical structures, enabling the generation of coherent 2-bar and 16-bar melodies, drum loops, and multi-instrument arrangements. It has been trained on large datasets like the Lakh MIDI Dataset, ensuring a broad understanding of musical styles. This level of quality would be a significant step up from a simple rule-based system and is suitable for the "Quality Mode" objective.

#### **3.2. Google Magenta: Performance RNN**

Performance RNN is another model from the Google Magenta project, specifically an LSTM-based recurrent neural network designed to generate expressive, polyphonic piano music that captures nuanced timing and dynamics.

The **commercial licensing** for Performance RNN is consistent with other Magenta projects, falling under the permissive Apache 2.0 license. This is explicitly confirmed by adaptations of the model and aligns with Magenta's open-source philosophy, making it a legally sound choice for SpawnClone.

Similar to MusicVAE, there is no evidence of an official or community-supported **ONNX conversion** for Performance RNN. The model is available as TensorFlow checkpoints and has been ported to TensorFlow.js for web use, but no mention of ONNX is made. This again places the burden of conversion, validation, and optimization on the development team. While tools exist, the lack of a dedicated pathway increases implementation risk and complexity for integration into a C++ application using ONNX Runtime.

The **model size and performance** characteristics are tied to its architecture. The number of parameters is determined by factors like the hidden unit size, which typically ranges from 128 to 1024 in benchmarked RNNs. While specific file sizes are not provided, the model is designed for real-time interaction, as demonstrated in browser demos. General RNN inference speed benchmarks show that forward pass latency can be in the microsecond range on a GPU, but this is highly dependent on the model size, batch size, and hardware. The model's focus on expressive performance implies a more complex event sequence than standard MIDI, which could impact processing time.

For **MIDI generation quality**, Performance RNN is exceptional in its domain. It models music as a sequence of fine-grained events, including note-on, note-off, time-shift, and velocity, allowing it to produce performances with human-like expressiveness. Trained on the Yamaha e-Piano Competition dataset, it is capable of generating virtuosic and dynamic polyphonic music, which would offer a unique and high-quality output for users of SpawnClone's Quality Mode.

#### **3.3. Music Transformer**

The Music Transformer is an architecture that applies the successful transformer model, widely used in natural language processing, to the domain of symbolic music generation. It uses attention mechanisms to capture long-range dependencies in music, leading to compositions with high structural coherence.

The **commercial licensing** for a Music Transformer model depends on the specific implementation and pre-trained weights chosen. Many such models are available on platforms like Hugging Face under permissive licenses like Apache 2.0 or MIT. The ONNX Runtime itself, which is critical for deployment, is available under the MIT license, making the entire toolchain commercially friendly, provided a properly licensed model is selected.

**ONNX Runtime integration** is a key strength of the Music Transformer architecture. The Hugging Face ecosystem provides robust tools, most notably the `optimum` library, designed to streamline the process of exporting transformer models to ONNX. This process is well-documented and supports optimizations like quantization and graph fusion during export. Furthermore, ONNX Runtime has dedicated, highly optimized kernels for transformer operations like attention, which can lead to significant performance gains. Benchmarks show that ONNX Runtime can accelerate transformer inference by up to 17x on CPU and 5x on GPU, with latencies in the low millisecond range for C++ implementations. This established and optimized pipeline dramatically reduces development risk and directly supports the performance requirements of SpawnClone's Quality Mode.

Regarding **model size and performance**, transformers can be large, but the ONNX ecosystem provides powerful solutions. Quantization can drastically reduce model size—for example, from 46.8 MB to 11.5 MB in one documented case—with minimal impact on output quality. This makes the models suitable for the download-and-cache requirement (FR23). The benchmarked performance improvements with ONNX Runtime in C++ are substantial, suggesting that the 3-5 second generation target (FR18) is highly achievable, even on consumer-grade CPUs.

The **MIDI generation quality** of the Music Transformer is considered state-of-the-art for symbolic music. Its ability to model long-term structure allows it to generate musically coherent and complex pieces that surpass the capabilities of many RNN-based models. This level of sophistication is precisely what is envisioned for a "Quality Mode" intended to be a premium, high-fidelity generation option.

#### **3.4. OpenAI: MuseNet and Derivatives**

MuseNet is a deep neural network from OpenAI capable of generating long-form musical compositions with multiple instruments, drawing on a wide range of musical styles. Open-source derivatives, such as OpenMusenet2, have emerged to replicate its functionality.

The **commercial licensing** of MuseNet is highly restrictive and presents a major obstacle. OpenAI's policy explicitly discourages charging for music generated by the model, effectively prohibiting its use in a commercial context. While derivatives like OpenMusenet2 are described as "open source," the provided data does not specify the exact license, creating significant legal ambiguity and risk. Using a derivative of a non-commercially licensed model could lead to legal challenges, making it an unsuitable choice for SpawnClone.

There is no mention of **ONNX compatibility** for either the original MuseNet or its derivatives in the provided research. As with the Magenta models, any integration with ONNX Runtime would require a manual conversion process, with the added complexity of a less-documented, community-driven codebase.

Information on **model size and performance** is not available in the research data. Given the model's complexity and ability to generate long compositions, it is likely to be a large model, but without concrete numbers or performance benchmarks, it is difficult to assess its suitability for on-device inference within the constraints of the SpawnClone project.

The **MIDI generation quality** of MuseNet is known to be very high. It can generate 4-minute musical pieces with 10 different instruments, blending styles from country to Mozart. This level of quality is certainly desirable, but it cannot be considered in isolation from the prohibitive licensing and technical uncertainties.

### **4. Comparative Analysis and Recommendations**

A direct comparison of the viable candidates—the Magenta models and the Music Transformer—reveals a clear path forward for SpawnClone's Quality Mode.

In the domain of **commercial licensing**, both the Magenta models (MusicVAE, Performance RNN) and appropriately selected Music Transformer models are excellent choices. Both are available under permissive licenses like Apache 2.0, which fully supports the project's open-source and distribution goals. MuseNet is definitively ruled out on this criterion.

The most significant point of divergence is **ONNX Runtime integration**. The Music Transformer architecture is the unequivocal leader in this area. The existence of dedicated tooling like Hugging Face's `optimum` library, combined with specific, high-performance optimizations within ONNX Runtime for transformers, creates a streamlined, low-risk, and high-reward path to implementation. The documented C++ inference speedups are compelling and align directly with the PRD's performance targets. In contrast, the Magenta models would require a manual, and potentially fragile, `tf2onnx` conversion process, adding significant development time and risk with no guarantee of achieving the same level of performance optimization.

Regarding **model size and performance**, all viable options appear manageable. The Magenta models are likely reasonably sized from the outset. While transformers can be larger, the proven effectiveness of ONNX quantization mitigates this concern, making them suitable for the required download-and-cache workflow. The performance benchmarks for ONNX-optimized transformers provide a level of confidence in meeting the 3-5 second generation window that is absent for the Magenta models, which would require their own extensive benchmarking post-conversion.

Finally, in terms of **generation quality**, all three viable models (MusicVAE, Performance RNN, Music Transformer) are capable of producing professional-grade MIDI. Performance RNN offers unique expressiveness, while MusicVAE provides strong structural coherence. However, the Music Transformer architecture is widely considered the state-of-the-art for sequence generation tasks, offering the highest potential for creating complex, long-form, and musically sophisticated patterns that would create a distinct and valuable differentiation for the "Quality Mode" feature.

### **5. Conclusion and Final Recommendation**

After a thorough evaluation of the candidate models against the critical requirements of the SpawnClone project, this report provides a definitive recommendation.

**Primary Recommendation: Music Transformer**
The Music Transformer architecture is the most suitable choice for SpawnClone's Quality Mode. It uniquely satisfies all evaluation criteria with the highest degree of confidence. Its state-of-the-art generation quality aligns with the goal of a premium user experience. Its robust and well-supported ecosystem for ONNX export and inference drastically reduces development risk and effort. The proven performance gains from ONNX Runtime optimizations ensure that the 3-5 second generation target is achievable. Finally, a wide selection of models with commercial-friendly licenses is readily available.

**Secondary Recommendation: Google Magenta Models (Performance RNN / MusicVAE)**
The Google Magenta models serve as a viable but less optimal alternative. Their primary strengths are their permissive Apache 2.0 license and proven generation quality. However, the lack of a streamlined ONNX integration pipeline presents a significant technical hurdle that would increase development time and complexity. They should be considered a fallback option if a suitable Music Transformer model cannot be successfully integrated.

The adoption of a Music Transformer model, integrated via the Hugging Face and ONNX Runtime ecosystem, represents the most strategic and technically sound path for delivering a powerful, performant, and legally compliant Quality Mode for the SpawnClone VST plugin.

### **Sources**

[Google Magenta MusicVAE](https://magenta.tensorflow.org/music-vae)
[Google Magenta Performance RNN](https://magenta.tensorflow.org/performance-rnn)
[Google Magenta Piano Genie](https://magenta.tensorflow.org/pianogenie)
[Google Magenta](https://magenta.tensorflow.org/)
[Google Research Researchers Release Magenta RealTime: An Open-Weight Model for Real-Time AI Music Generation](https://www.marktechpost.com/2025/06/22/google-researchers-release-magenta-realtime-an-open-weight-model-for-real-time-ai-music-generation/)
[Hugging Face - Convert Transformers to ONNX](https://huggingface.co/blog/convert-transformers-to-onnx)
[Hugging Face - google/magenta-realtime](https://huggingface.co/google/magenta-realtime)
[Hugging Face Docs - Optimum Exporters](https://huggingface.co/docs/optimum/v1.16.0/en/exporters/onnx/usage_guides/export_a_model)
[Hugging Face Docs - Transformers Serialization](https://huggingface.co/docs/transformers/serialization)
[Hugging Face Docs - Transformers ONNX](https://huggingface.co/docs/transformers/en/main_classes/onnx)
[Hugging Face Docs - Transformers Serialization v4.17.0](https://huggingface.co/docs/transformers/v4.17.0/en/serialization)
[Hugging Face Docs - Transformers Serialization v4.29.1](https://huggingface.co/docs/transformers/v4.29.1/serialization)
[I AM AI - Piano Genie](https://www.i-am.ai/piano-genie.html)
[Medium - Convert your bulky transformer models into lightweight, high-performance ONNX models](https://medium.com/@nabarun.barua/convert-your-bulky-transformer-models-into-lightweight-high-performance-onnx-models-5b18bc25ee06)
[Medium - MusicVAE: Understanding of the Google’s work for interpolating two music sequences](https://medium.com/@musicvaeubcse/musicvae-understanding-of-the-googles-work-for-interpolating-two-music-sequences-621dcbfa307c)
[Medium - Production-Ready Transformer Models Optimization with ONNX](https://medium.com/@himsgpt/production-ready-transformer-models-optimization-with-onnx-a32b5f3cfe92)
[Microsoft Open Source Blog - Journey to optimize large-scale transformer model inference with ONNX Runtime](https://opensource.microsoft.com/blog/2021/06/30/journey-to-optimize-large-scale-transformer-model-inference-with-onnx-runtime/)
[Microsoft Open Source Blog - Performant on-device inferencing with ONNX Runtime](https://opensource.microsoft.com/blog/2023/02/08/performant-on-device-inferencing-with-onnx-runtime/)
[Microsoft Azure Blog - Accelerate your NLP pipelines using Hugging Face Transformers and ONNX Runtime](https://medium.com/microsoftazure/accelerate-your-nlp-pipelines-using-hugging-face-transformers-and-onnx-runtime-2443578f4333)
[Microsoft Cloud Blogs - Microsoft and ONNX open source optimizations for transformer model inference on GPU and CPU](https://cloudblogs.microsoft.com/opensource/2020/01/21/microsoft-onnx-open-source-optimizations-transformer-inference-gpu-cpu/)
[NVIDIA Developer - Deep Learning Performance Training Inference](https://developer.nvidia.com/deep-learning-performance-training-inference/ai-inference)
[NVIDIA - MLPerf Benchmarks](https://www.nvidia.com/en-us/data-center/resources/mlperf-benchmarks/)
[ONNX Runtime](https://onnxruntime.ai/)
[ONNX Runtime - Accelerating Generative AI with ONNX Runtime](https://onnxruntime.ai/generative-ai)
[ONNX Runtime - Accelerating Phi-2, Mistral, Gemma and other models with ONNX Runtime](https://onnxruntime.ai/blogs/accelerating-phi-2)
[ONNX Runtime Docs](https://onnxruntime.ai/docs/)
[ONNX Runtime Docs - Extensions](https://onnxruntime.ai/docs/extensions/)
[ONNX Runtime Docs - Model Optimizations](https://onnxruntime.ai/docs/performance/model-optimizations/)
[ONNX Runtime Docs - TensorFlow Get Started](https://onnxruntime.ai/docs/tutorials/tf-get-started.html)
[ONNX Runtime Docs - Transformers Optimization](https://onnxruntime.ai/docs/performance/transformers-optimization.html)
[ONNX Runtime - Hugging Face](https://onnxruntime.ai/huggingface)
[ONNX.ai](https://onnx.ai/)
[OpenAI - MuseNet](https://openai.com/index/musenet/)
[OpenBenchmarking.org - ONNX Runtime 1.17 Alpha](https://openbenchmarking.org/test/pts/onnx&eval=13c3b2908d7d0186ce944119892bbb97759a0f68)
[Splunk - Open Neural Network Exchange (ONNX)](https://www.splunk.com/en_us/blog/learn/open-neural-network-exchange-onnx.html)
[Spare Cores - LLM Inference Speed & Cost Benchmarks](https://sparecores.com/article/llm-inference-speed)
[Stanford Vision and Learning Lab - RNN Performance](https://svail.github.io/rnn_perf/)
[arXiv - MLPerf Inference Benchmark](https://arxiv.org/pdf/1911.02549)
[arXiv - Run-Time Efficient RNN Compression for Inference on Mobile and Embedded Devices](https://arxiv.org/pdf/1906.04886)
[AudioCipher - MuseNet](https://www.audiocipher.com/post/musenet)
[Colab - Performance RNN](https://colab.research.google.com/notebooks/magenta/performance_rnn/performance_rnn.ipynb)
[Colab - Piano Transformer](https://colab.research.google.com/notebooks/magenta/piano_transformer/piano_transformer.ipynb)
[FutureTools - MuseNet (OpenAI)](https://www.futuretools.io/tools/musenet-openai)
[GitHub - glample/rnn-benchmarks](https://github.com/glample/rnn-benchmarks)
[GitHub - hidude562/OpenMusenet2](https://github.com/hidude562/OpenMusenet2)
[GitHub - IMAGINARY/muski-performance-rnn](https://github.com/IMAGINARY/muski-performance-rnn)
[GitHub - IMAGINARY/piano-genie](https://github.com/IMAGINARY/piano-genie)
[GitHub - magenta/magenta-demos/performance_rnn](https://github.com/magenta/magenta-demos/blob/main/performance_rnn/index.html)
[GitHub - magenta/magenta/music_vae/base_model.py](https://github.com/magenta/magenta/blob/main/magenta/models/music_vae/base_model.py)
[GitHub - magenta/magenta/music_vae/README.md](https://github.com/magenta/magenta/blob/main/magenta/models/music_vae/README.md)
[GitHub - magenta/magenta/performance_rnn/README.md](https://github.com/magenta/magenta/blob/main/magenta/models/performance_rnn/README.md)
[GitHub - magenta/magenta-js/music/piano_genie/model.ts](https://github.com/magenta/magenta-js/blob/master/music/src/piano_genie/model.ts)
[GitHub - microsoft/onnxruntime/issues/3452](https://github.com/microsoft/onnxruntime/issues/3452)
[GitHub - microsoft/onnxruntime/issues/677](https://github.com/microsoft/onnxruntime/issues/677)
[GitHub - onnx/models](https://github.com/onnx/models)
[GitHub - onnx/tensorflow-onnx](https://github.com/onnx/tensorflow-onnx)
[Magenta RealTime](https://magenta.withgoogle.com/magenta-realtime)
[Magenta.js Music](https://magenta.github.io/magenta-js/music/)
[Magenta - MIDI-Me](https://magenta.tensorflow.org/midi-me)
[Magenta - Multitrack MusicVAE](https://magenta.tensorflow.org/multitrack)
[Magenta - Performance RNN in your browser](https://magenta.tensorflow.org/performance-rnn-browser)
[Magenta - Performance RNN Demo](https://magenta.tensorflow.org/demos/performance_rnn/index.html)
[Piano Genie Demo](http://piano-genie.glitch.me/)
[Reddit - r/musenet](https://www.reddit.com/r/musenet/comments/14prm0t/first_version_of_my_freeopen_source_recreation_of/)
[Stack Overflow - Magenta Performance RNN with own MIDI data](https://stackoverflow.com/questions/50909982/magenta-performance-rnn-demo-with-own-midi-data)