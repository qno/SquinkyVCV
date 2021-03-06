#pragma once

#include <memory>
#include <string>
#include <vector>

class WaveLoader {
public:
    class WaveInfo {
    public:
        WaveInfo(const std::string& fileName);
        ~WaveInfo();
        void load();

        bool valid = false;
        float* data = nullptr;
        unsigned int numChannels = 0;
        unsigned int sampleRate = 0;
        uint64_t totalFrameCount = 0;
        const std::string fileName;
    private:
        static float* convertToMono(float* data, uint64_t frames, int channels);
    };
    using WaveInfoPtr = std::shared_ptr<WaveInfo>;

    void addNextSample(const std::string& fileName);
    void load();

    /**
     * Index is one based. 
     */
    WaveInfoPtr getInfo(int index) const;

    static char nativeSeparator();
    static char foreignSeparator();
    static void makeAllSeparatorsNative(std::string& s);

private:
    std::vector<std::string> filesToLoad;
    std::vector<WaveInfoPtr> finalInfo;
    void clear();
    bool didLoad = false;
};

using WaveLoaderPtr = std::shared_ptr<WaveLoader>;