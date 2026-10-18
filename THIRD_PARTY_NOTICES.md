# Third-party notices

Lowline's original code is AGPL-3.0-only. Copyright 2026 Massimo Danieli.

The complete source ZIP contains these dependencies; repository builds fetch them at the pinned revisions:

- JUCE 8.0.6, copyright Raw Material Software Limited and contributors, used under AGPLv3. The unmodified upstream `third_party/JUCE/LICENSE.md` includes module and dependency licensing notices; individual bundled codecs, SDKs and libraries retain their own terms.
- NeuralAmpModelerCore commit 0b3d3c97b0859a3a8c92a8628c4dd89a25eb5842, copyright Steven Atkinson and contributors, MIT. Full notice: `third_party/NAM/LICENSE`.
- Eigen at NAM's pinned submodule commit bc3b39870ecb690a623a3f49149a358b95c5781d, primarily MPL-2.0; see the COPYING files in `third_party/NAM/Dependencies/eigen`.
- nlohmann JSON, MIT; see `third_party/NAM/Dependencies/nlohmann/json.hpp`.

JUCE VST3/AU platform SDKs and other dependencies are provided as upstream source with their notices intact. The `third_party` contents were copied without modification, excluding Git administrative metadata and unused NAM tools/example assets.

NAM, A2 and TONE3000 names describe interoperability and do not imply endorsement. Models and impulse responses downloaded separately carry their own licences and are not part of this package.
