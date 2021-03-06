#pragma once

#include <assert.h>

#include "SamplerPlayback.h"

/**
 * a node in the instrument tree that can branch based on pitch
 */
class PitchSwitch : public ISamplerPlayback {
public:
    void play(VoicePlayInfo&, const VoicePlayParameter&, WaveLoader* loader, float sampleRate) override;
    void _dump(int depth) const override;
    void _setTestMode() { testMode = true; }
    void addEntry(int pitch, ISamplerPlaybackPtr data) {
        auto test = pitchMap[pitch];
        assert(!test);
        pitchMap[pitch] = data;
    }
    PitchSwitch(int line) : lineNumber(line) {}

private:
    bool testMode = false;
    std::vector<ISamplerPlaybackPtr> pitchMap = std::vector<ISamplerPlaybackPtr>(128);
    const int lineNumber;
};

inline void PitchSwitch::play(VoicePlayInfo& info, const VoicePlayParameter& params, WaveLoader* loader, float sampleRate) {
    if (testMode) {
        info.sampleIndex = 1;
        info.needsTranspose = false;
        info.transposeAmt = 1;
        info.valid = true;
        return;
    }

    info.valid = false;
    const ISamplerPlaybackPtr entry = pitchMap[params.midiPitch];
    if (entry) {
        entry->play(info, params, loader, sampleRate);
    }
}

inline void PitchSwitch::_dump(int depth) const {
    indent(depth);
    printf("begin pitch switch %p\n", this);
    for (int pitch = 0; pitch < 128; ++pitch) {
        auto entry = pitchMap[pitch];
        if (entry) {
            indent(depth + 1);
            printf("pitch entry[%d]\n", pitch);
            entry->_dump(depth + 1);
        }
    }
    indent(depth);
    printf("end pitch switch %p\n", this);
}

using PitchSwitchPtr = std::shared_ptr<PitchSwitch>;