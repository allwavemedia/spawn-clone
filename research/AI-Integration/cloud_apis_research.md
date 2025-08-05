# Research Report: Analysis of Cloud APIs for Music Generation

**DATE:** 2025-08-05

## Executive Summary

This report provides a comprehensive analysis of cost-effective cloud-based Application Programming Interfaces (APIs) for music generation, with a primary focus on services from Google and OpenAI. The research objective is to evaluate these platforms as potential alternatives to Pozalabs ARIA for integration into SpawnClone's "Cloud Mode" feature. The evaluation is framed by SpawnClone's specific technical and business requirements, including a generation time of less than ten seconds, straightforward REST API integration with C++ clients, permissive commercial licensing, and the delivery of professional-quality MIDI output compatible with standard Digital Audio Workstations (DAWs).

Our analysis of Google's cloud offerings centers on the Gemini API, particularly its music generation capabilities powered by the Lyria model, accessible through Vertex AI. Google provides a sophisticated, audio-first music generation service with transparent, modality-based pricing. The Gemini 2.5 Flash Live API, for instance, offers real-time audio streaming with costs delineated per million tokens for audio input and output. While its capabilities for generating instrumental music from text prompts are advanced and its REST API architecture aligns with SpawnClone's integration needs, a critical deficiency was identified: the service does not natively support MIDI output. All music is generated as an audio stream, which would necessitate a complex and potentially unreliable audio-to-MIDI conversion process to meet SpawnClone's requirements, introducing significant development risk, latency, and potential quality degradation.

The evaluation of OpenAI's services reveals a more definitive incompatibility with the project's objectives. While OpenAI previously developed MuseNet, a model renowned for its MIDI generation capabilities, this tool and its associated API have been decommissioned and are no longer available. The current flagship models, including GPT-4, GPT-4o, and their variants, are fundamentally text- and vision-centric. Although some models feature audio processing APIs, these are designed for tasks like transcription and speech synthesis, not for the structured, polyphonic composition of music. Consequently, OpenAI's current platform lacks any viable mechanism for generating MIDI-based musical content, rendering it unsuitable as an alternative for SpawnClone's Cloud Mode.

In conclusion, neither Google nor OpenAI offers a direct, low-risk replacement for a MIDI-native music generation API. Google's Gemini API with the Lyria model stands as the only plausible, albeit flawed, option between the two. Its adoption would be entirely contingent on the successful development and validation of a secondary audio-to-MIDI conversion pipeline that satisfies SpawnClone's stringent quality and performance standards. OpenAI's current offerings are not recommended for consideration. This report recommends that SpawnClone proceed with a cautious proof-of-concept phase with the Google Gemini API to assess the feasibility of the required conversion workflow. Should this prove impractical, SpawnClone may need to expand its search to include more specialized, MIDI-native API providers that fall outside the scope of this initial investigation.

## Introduction

The SpawnClone project is developing an innovative VST plugin designed to provide musicians and producers with AI-powered MIDI generation capabilities. A key feature outlined in the product roadmap is the "Cloud Mode," which aims to leverage the immense computational power of cloud-based AI models to generate musical patterns of superior quality and complexity compared to its on-device "Quality Mode." The successful implementation of Cloud Mode hinges on the selection of a third-party API that is not only technologically advanced but also commercially viable and seamlessly integrable into the SpawnClone ecosystem. The current exploration seeks to identify and evaluate cost-effective alternatives to the specialized service offered by Pozalabs ARIA.

This research report is commissioned to conduct a rigorous evaluation of two of the most prominent players in the cloud AI landscape: Google and OpenAI. The primary objective is to determine their suitability as the backend service for SpawnClone's Cloud Mode. The analysis is guided by a strict set of technical and business constraints critical to the project's success. These include the ability to generate professional-quality MIDI patterns that are immediately usable in professional DAWs, a generation latency of under ten seconds to ensure a fluid user experience, and a REST API architecture that can be consumed by SpawnClone's C++-based HTTP client. Furthermore, the chosen API must offer a transparent and cost-effective pricing model and a commercial license that permits the distribution and use of generated content within the SpawnClone plugin.

This document presents a detailed comparative analysis of the relevant services from Google Cloud AI and the OpenAI API. It delves into their pricing structures, core music generation functionalities, native support for the MIDI format, and the technical requirements for integration. By juxtaposing these findings against SpawnClone's operational constraints, the report aims to provide a clear, evidence-based recommendation to guide the strategic decision-making process for the Cloud Mode feature.

## Evaluation Framework

To ensure a systematic and objective assessment, each candidate API was evaluated against a standardized framework of criteria derived from SpawnClone's core requirements. This framework prioritizes technical feasibility, financial viability, and alignment with the product's creative goals. Each criterion is detailed below to provide clarity on the basis of the subsequent analysis.

### Pricing and Cost-Effectiveness

This criterion examines the complete cost structure of the API service. The analysis includes an evaluation of pay-as-you-go pricing models, such as costs per million tokens, per character, per second of generated audio, or per API request. The existence and limitations of any free tiers or trial credits available for initial development and testing are also considered. The goal is to understand the total cost of ownership at scale and to identify the most cost-effective solution for generating short musical clips, ensuring financial sustainability for the SpawnClone Cloud Mode feature. The transparency of the pricing model and the availability of cost-estimation tools are also key factors.

### Music Generation Capabilities and Quality

This area assesses the core function of the API: its ability to generate music. The evaluation focuses on the sophistication of the underlying AI models, such as Google's Lyria or OpenAI's historical work with MuseNet. Key capabilities under review include text-to-music generation, the ability to control stylistic elements (e.g., genre, mood), and the capacity to generate coherent multi-instrumental arrangements. The quality of the output is paramount; the generated music must be musically coherent, complex, and of a professional standard suitable for use in genres such as Hip Hop, Pop, and EDM.

### MIDI Output Support and DAW Compatibility

This is a critical, non-negotiable requirement for SpawnClone. This criterion evaluates the API's native ability to generate and deliver output in the Musical Instrument Digital Interface (MIDI) format. An ideal API would directly produce MIDI files or data streams that contain note, velocity, timing, and instrument information. The quality of this MIDI output is scrutinized for its compatibility and usability within standard Digital Audio Workstations (DAWs) like Ableton Live, Logic Pro, or FL Studio. The absence of native MIDI support is considered a major deficiency, as it would necessitate a secondary, complex conversion process.

### Integration Feasibility and Performance

This criterion assesses the technical aspects of integrating the API into the SpawnClone VST plugin. A primary requirement is the availability of a well-documented REST API, which is compatible with standard C++ HTTP client libraries. The complexity of the API's request/response structure and the ease of implementing authentication are also considered. Performance is evaluated against the requirement for a generation time of less than ten seconds, from the initial request to the receipt of the generated content. This includes any inherent model inference latency and network transfer time.

### Commercial Licensing

The legal framework governing the use of the API and its generated content is a crucial consideration. This criterion evaluates the terms of the commercial license offered by the provider. The license must explicitly permit the use of the API within a commercial or freely distributed product like SpawnClone. Furthermore, it must grant SpawnClone and its end-users the necessary rights to use, modify, and distribute the AI-generated musical compositions without prohibitive royalties or restrictive attribution requirements. Any ambiguity or restriction in the commercial use policy is flagged as a significant business risk.

## Analysis of Google Cloud AI Services

Google's offerings in the generative AI space are consolidated under its Vertex AI platform and the Gemini API, which provide access to a suite of powerful models, including those specialized for music. The analysis focuses on the Gemini family of models, particularly those incorporating the Lyria technology for music generation.

### Pricing and Cost-Effectiveness

Google Cloud provides a highly detailed and transparent pricing structure for its Gemini models, primarily based on token consumption or modality-specific units like characters or seconds of audio. For music generation, the most relevant pricing is for models that handle audio input and output. The `Gemini 2.5 Flash Live API` is a prime example, with costs of $3.00 per million input audio tokens and $12.00 per million output audio tokens. A modality-based pricing reference indicates that audio is tokenized at a rate of approximately 25 tokens per second. This allows for predictable cost estimation; for instance, generating a 10-second audio clip would consume roughly 250 output tokens, making the per-clip cost exceptionally low. For non-real-time tasks, Google offers a Batch API, which provides a 50% discount on standard rates, further enhancing cost-effectiveness for asynchronous generation. Google also provides a free tier with daily limits and a substantial $300 credit for new customers, enabling extensive testing and prototyping without initial financial commitment. This granular, usage-based pricing model is well-suited for SpawnClone's needs, allowing costs to scale directly with user engagement.

### Music Generation Capabilities and Quality

Google's primary music generation technology is a model named Lyria, which is integrated into the Gemini API ecosystem. Research indicates that Lyria is a state-of-the-art model capable of generating high-quality instrumental music in real-time from text-based prompts. It allows for a significant degree of user control, with the ability to steer generation by specifying parameters such as tempo, scale, and musical density. The model is designed for creating continuous streams of music via WebSockets, making it suitable for interactive applications. The quality of the generated audio is professional-grade, designed to produce coherent and stylistically appropriate musical pieces. This aligns well with SpawnClone's objective of offering a premium generation experience in its Cloud Mode. The underlying technology appears robust and capable of handling the creative demands of music producers across various genres.

### MIDI Output Support and DAW Compatibility

This criterion reveals the most significant challenge with Google's current offering. The provided research data consistently indicates that the Gemini API and the underlying Lyria model are fundamentally audio-centric. The output of the music generation service is an audio stream (e.g., WAV or MP3), not a MIDI file. While Google AI Studio has hosted experimental beta tools like "MIDI DJ," this functionality is not a standard feature of the production-level Gemini API. This lack of native MIDI support represents a critical misalignment with SpawnClone's core requirement. To utilize Google's service, SpawnClone would be required to implement an additional, non-trivial technical pipeline for audio-to-MIDI conversion. This process introduces substantial complexity, potential for quality loss in the conversion, and added latency, which could compromise the user experience and the professional quality of the final MIDI output.

### Integration Feasibility and Performance

From an integration standpoint, Google Cloud's services are well-structured for developers. The Gemini API is a standard RESTful service, which is fully compatible with C++ HTTP client libraries, meeting SpawnClone's technical requirement. Google provides extensive documentation, client libraries, and quickstart guides to facilitate the integration process. The performance of the Lyria model, particularly its implementation for real-time streaming, suggests that it is highly optimized for low-latency inference. Achieving the sub-10-second generation target for short musical clips is highly probable, especially for audio output. However, the total latency would be impacted by the necessary, additional audio-to-MIDI conversion step, the performance of which is unknown and would need to be benchmarked separately.

### Commercial Licensing

Google Cloud's terms of service are designed for enterprise and commercial use. The pay-as-you-go pricing model inherently covers a commercial license to use the API within an application like SpawnClone. The paid tiers ensure that customer data is not used for training Google's models, which is a crucial consideration for data privacy and intellectual property. The generated content's ownership typically resides with the customer, subject to compliance with Google's acceptable use policies. This licensing framework is robust and suitable for SpawnClone's business model, posing no significant legal or commercial barriers.

## Analysis of OpenAI Services

OpenAI is a leader in the field of generative AI, widely known for its GPT series of language models. This analysis investigates the suitability of its current API offerings for the specific task of music and MIDI generation required by SpawnClone.

### Pricing and Cost-Effectiveness

OpenAI's API pricing is also token-based, with clear distinctions between different models and modalities. For tasks involving audio, the `GPT-4o` model offers a `Realtime API` with audio processing capabilities. The cost is $40.00 per million input audio tokens and $80.00 per million output audio tokens. While these prices are competitive, it is crucial to note that OpenAI's audio capabilities are primarily designed for speech-to-text and text-to-speech applications, not for polyphonic music generation. Therefore, a direct cost comparison with Google's music-specific API is challenging. OpenAI also offers fine-tuning services, which come with additional training and inference costs. For example, fine-tuning `GPT-4.1` costs $25.00 per million training tokens, with higher per-token rates for inference. While the pricing is transparent, its applicability to SpawnClone's use case is questionable given the model's core capabilities.

### Music Generation Capabilities and Quality

This is the definitive point of failure for OpenAI as a candidate. The research confirms that OpenAI's historical music generation model, MuseNet, is no longer publicly available or supported via an API. MuseNet was renowned for its ability to generate long-form, multi-instrumental compositions and output them as MIDI files. Its decommissioning leaves a significant void in OpenAI's portfolio. The current flagship models, such as GPT-4 and GPT-4o, are not designed for music composition. They lack the underlying architecture and training to understand and generate complex musical structures, harmony, and rhythm. While one could theoretically prompt these models to generate a textual representation of music (e.g., a list of notes), this is an impractical and unreliable workaround that would not produce professional-quality results. There is currently no dedicated music generation service offered through the OpenAI API.

### MIDI Output Support and DAW Compatibility

Given the absence of a music generation model, there is consequently no support for MIDI output. The OpenAI API cannot be used to generate MIDI files directly. This represents a complete failure to meet one of SpawnClone's most critical technical requirements. The legacy of MuseNet demonstrates that OpenAI possesses the institutional knowledge to build such a system, but it is not part of their current commercial strategy or product lineup.

### Integration Feasibility and Performance

Similar to Google, the OpenAI API is a modern RESTful service that is well-documented and straightforward to integrate using standard C++ HTTP clients. The performance of their models for their intended tasks (text and speech processing) is state-of-the-art. However, since the API does not perform the required function of music generation, its integration feasibility and performance for this specific use case are moot points.

### Commercial Licensing

OpenAI's API is intended for commercial use, and its standard terms of service allow for the integration of the API into commercial products. The pricing structure reflects this commercial availability. Users retain ownership of the content they generate, provided it adheres to OpenAI's usage policies. The licensing model itself is sound and would be compatible with SpawnClone's needs if the underlying technology were suitable for the task.

## Comparative Analysis and Risk Assessment

A direct comparison between Google and OpenAI for SpawnClone's Cloud Mode reveals a stark contrast in suitability. Google offers a dedicated, albeit imperfect, solution, while OpenAI currently offers no viable path forward.

In terms of **music generation capability**, Google is the clear and only contender. Its Lyria model, accessible via the Gemini API, is explicitly designed for creating instrumental music. OpenAI, having retired its MuseNet model, has no comparable offering in its public API. This fundamental difference makes Google the only one of the two platforms to even partially meet the project's core objective.

The most critical point of comparison is **MIDI support**. Here, both platforms fall short of the ideal, but in different ways. OpenAI offers no support whatsoever, making it entirely unsuitable. Google offers a high-quality music generation engine that unfortunately outputs audio instead of MIDI. This positions Google as a high-risk, high-reward candidate. The risk lies in the significant engineering effort required to build a reliable audio-to-MIDI conversion pipeline. This introduces new dependencies, potential points of failure, and a risk of degrading the musical quality and timing precision of the generated content. The success of this conversion process is not guaranteed and would require a dedicated proof-of-concept phase to validate.

Regarding **pricing**, both platforms offer transparent, usage-based models. Google's modality-based pricing per second of audio is arguably more intuitive and predictable for this specific use case than OpenAI's token-based audio pricing, which is geared towards speech. A hypothetical 10-second clip generated via Google's Gemini 2.5 Flash Live API would cost approximately $0.00003, an extremely competitive rate. The cost-effectiveness of Google's solution is high, assuming the technical hurdles can be overcome.

From an **integration and licensing** perspective, both platforms are on equal footing. They both provide modern REST APIs that are compatible with SpawnClone's C++ architecture and offer standard commercial licenses suitable for a production environment.

The primary risk in selecting Google is technical: the challenge and uncertainty of the audio-to-MIDI conversion. Mitigation would involve researching and prototyping third-party or open-source audio-to-MIDI libraries, carefully evaluating their accuracy, performance, and ability to handle polyphonic, multi-instrumental audio. The risk in selecting OpenAI is absolute: it is a guaranteed failure to meet the project's requirements.

## Conclusion and Recommendation

After a thorough analysis of the cloud API offerings from Google and OpenAI, it is evident that neither provides a perfect, off-the-shelf solution that fully aligns with all of SpawnClone's requirements for its Cloud Mode feature. The critical need for professional-quality, native MIDI output remains the most significant unmet challenge.

Of the two platforms evaluated, **Google's Gemini API, powered by the Lyria model, is the only viable, albeit high-risk, candidate.** Its strength lies in its dedicated, high-quality music generation engine and its cost-effective, transparent pricing model. However, its critical weakness is its audio-only output. The feasibility of using this service is entirely dependent on SpawnClone's ability to successfully implement a supplementary audio-to-MIDI conversion pipeline that meets the project's stringent standards for quality, latency, and reliability.

**OpenAI's current API offerings are not a suitable alternative for this project.** The decommissioning of MuseNet and the text-centric focus of its current models mean that it lacks the fundamental capability to generate structured musical content, rendering it incompatible with the core objectives of the Cloud Mode feature.

Therefore, this report puts forth the following recommendations:

1.  **Primary Recommendation:** Initiate a time-boxed, technical proof-of-concept (PoC) with the **Google Gemini API**. The sole objective of this PoC should be to build and evaluate an audio-to-MIDI conversion workflow. This will determine if the quality and performance of the resulting MIDI are acceptable for a professional music production tool. If the PoC is successful, the Google Gemini API represents a powerful and cost-effective foundation for the Cloud Mode feature.

2.  **Secondary Recommendation:** If the proof-of-concept with Google's API fails to meet requirements, it is recommended that SpawnClone **expand its research to include specialized, MIDI-native music generation APIs.** While this report focused on Google and OpenAI as per the initial objective, the broader market contains vendors like AIVA or HookTheory (with its Aria model) that prioritize MIDI output and may offer a more direct, lower-risk path to achieving SpawnClone's goals.

Pursuing OpenAI for this feature is strongly discouraged as it would divert resources with no realistic prospect of success. The strategic path forward for SpawnClone's Cloud Mode lies in either solving the audio-to-MIDI challenge with Google's powerful engine or pivoting to a more specialized, MIDI-first API provider.

## References

[Google AI - Gemini API Docs - Music Generation](https://ai.google.dev/gemini-api/docs/music-generation)
[Google AI - Gemini API Docs - Pricing](https://ai.google.dev/gemini-api/docs/pricing)
[Google AI - Gemini API Docs - Billing](https://ai.google.dev/gemini-api/docs/billing)
[Google AI - Gemini API Docs - Models](https://ai.google.dev/gemini-api/docs/models)
[Google AI Studio - Welcome](https://aistudio.google.com/welcome)
[Google Cloud - Vertex AI - Generative AI Pricing](https://cloud.google.com/vertex-ai/generative-ai/pricing)
[Google Cloud - Vertex AI - Pricing](https://cloud.google.com/vertex-ai/pricing)
[Google Cloud - Products - Gemini Pricing](https://cloud.google.com/products/gemini/pricing)
[Google Cloud - Generative AI Studio](https://cloud.google.com/generative-ai-studio)
[Google Cloud - Vertex AI - Deployment Docs](https://cloud.google.com/vertex-ai/docs/general/deployment)
[Google Cloud - Vertex AI - Generative AI Docs](https://cloud.google.com/vertex-ai/generative-ai/docs)
[Google Cloud - Generative AI App Builder Pricing](https://cloud.google.com/generative-ai-app-builder/pricing)
[Google Cloud - AI Platform Pricing](https://cloud.google.com/ai-platform/pricing)
[Google Cloud - Pricing Calculator](https://cloud.google.com/products/calculator)
[Google DeepMind - Music AI Sandbox](https://deepmind.google/discover/blog/music-ai-sandbox-now-with-new-features-and-broader-access/)
[Google Labs](https://labs.google/)
[Google Labs - MusicFX DJ](https://labs.google/fx/tools/music-fx-dj)
[Google Labs - FX](https://labs.google/fx/)
[Google Developers Blog - Gemini API I/O Updates](https://developers.googleblog.com/en/gemini-api-io-updates/)
[OpenAI - API Pricing](https://openai.com/api/pricing/)
[OpenAI - Platform Docs - Pricing](https://platform.openai.com/docs/pricing)
[OpenAI - Platform Docs - Fine-tuning Guide](https://platform.openai.com/docs/guides/fine-tuning/)
[OpenAI - ChatGPT Pricing](https://openai.com/chatgpt/pricing/)
[OpenAI - MuseNet](https://openai.com/index/musenet/)
[OpenAI - Jukebox](https://openai.com/index/jukebox/)
[Azure - OpenAI Service Pricing](https://azure.microsoft.com/en-us/pricing/details/cognitive-services/openai-service/)
[Microsoft Learn - Azure AI Foundry Fine-tuning Overview](https://learn.microsoft.com/en-us/azure/ai-foundry/concepts/fine-tuning-overview)
[Windows Dev Blogs - Windows MIDI Services Oct 2024 Update](https://devblogs.microsoft.com/windows-music-dev/windows-midi-services-oct-2024-update/)
[AIVA](https://www.aiva.ai/)
[AudioCipher - AI Music App](https://www.audiocipher.com/post/ai-music-app)
[AudioCipher - AI Music Producer](https://www.audiocipher.com/post/ai-music-producer)
[AudioCipher - MuseNet](https://www.audiocipher.com/post/musenet)
[Beatoven.ai - Best APIs for Music Generation](https://www.beatoven.ai/blog/best-apis-for-music-generation/)
[Beatoven.ai - Best Text to Music APIs](https://www.beatoven.ai/blog/best-text-to-music-apis/)
[CloudChipr - Vertex AI Blog](https://cloudchipr.com/blog/vertex-ai)
[Creativity With AI - MuseNet](https://creativitywith.ai/musenet/)
[Custom TypingMind - LLM Cost Estimator](https://custom.typingmind.com/tools/estimate-llm-usage-costs/gemini-1.5-pro)
[Denovo Agency - Best AI Tools for Music Makers](https://denovoagency.com/blogs/insights-and-strategies-for-the-modern-musician/best-ai-tools-for-music-makers-top-20-picks-for-2024)
[dev.to - Understanding Google Cloud Platform Pricing](https://dev.to/ddeveloperr/understanding-google-cloud-platform-pricing-gcp-pricing-59h4)
[DIDA - OpenAI API Pricing Breakdown](https://dida.do/openai-s-api-pricing-cost-breakdown-for-gpt-3-5-gpt-4-and-gpt-4o)
[DigitalOcean - AI Music Generators](https://www.digitalocean.com/resources/articles/ai-music-generators)
[FutureTools - MuseNet (OpenAI)](https://www.futuretools.io/tools/musenet-openai)
[Helicone - LLM Cost Calculator](https://www.helicone.ai/llm-cost/provider/openai/model/gpt-4o)
[Hyscaler - Google Music AI MusicFX](https://hyscaler.com/insights/google-music-ai-musicfx/)
[Inverted Stone - Gemini Pricing Calculator](https://invertedstone.com/calculators/gemini-pricing)
[Levysoft Medium - Analysis of OpenAI API Costs](https://levysoft.medium.com/analysis-of-openai-api-costs-gpt-4-vs-chatgpt-d1c4bce247bf)
[MIDI.org - 3 Best AI Music Generators for MIDI Creation](https://midi.org/3-best-ai-music-generators-for-midi-creation)
[NewsAPI - Pricing](https://newsapi.org/pricing)
[9meters - Google Gemini Costs](https://9meters.com/technology/ai/google-gemini-costs)
[OpenAI Community - Music Model for ChatGPT](https://community.openai.com/t/music-model-for-chatgpt-that-allows-for-midi/420529)
[OpenAI Community - Fine-tuned Model Cost](https://community.openai.com/t/doesnt-understand-fine-tuned-model-cost/80605)
[OpenAI Community - Cost of Using a Fine-tuned Model](https://community.openai.com/t/cost-of-using-a-fine-tuned-model-compared-to-the-base-model/899102)
[Reddit - r/google - MusicFX Launch](https://www.reddit.com/r/google/comments/18gtoc2/hello_reddit_im_a_designer_at_google_and_today_my/)
[Reddit - r/singularity - MusicFX by Google](https://www.reddit.com/r/singularity/comments/18gxmot/musicfx_by_google_texttomusic_ai/)
[Reddit - r/OpenAI - Fine-tuning Costs](https://www.reddit.com/r/OpenAI/comments/1hx28fo/has_anyone_tried_finetuning_openai_models_to/)
[Staccato.ai - MuseNet Alternative](https://staccato.ai/musenet-alternative-staccato)
[Tekpon - Google Cloud Vertex AI Pricing](https://tekpon.com/software/google-cloud-vertex-ai/pricing/)
[The Verge - OpenAI MuseNet](https://www.theverge.com/2019/4/26/18517803/openai-musenet-artificial-intelligence-ai-music-generation-lady-gaga-harry-potter-mozart)
[TrustRadius - Google Cloud Vertex AI Pricing](https://www.trustradius.com/products/google-cloud-vertex-ai/pricing)
[VentureBeat - Google's New AI Tool MusicFX](https://venturebeat.com/ai/googles-new-ai-tool-musicfx-composes-music-with-just-a-few-words/)
[APIDog - Google Gemini 2.0 API](https://apidog.com/blog/google-gemini-2-0-api/)
[Google Developers - Maps Platform - Places API Usage and Billing](https://developers.google.com/maps/documentation/places/web-service/usage-and-billing)