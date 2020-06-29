
#pragma once

#ifndef _MSC_VER 
#include "asserts.h"
#include "AudioMath.h"
#include "Divider.h"
#include "SubVCO.h"
#include <assert.h>
#include <memory>
#include "IComposite.h"
#include "LookupTableFactory.h"
#include "simd.h"
#include "SimpleQuantizer.h"

/**
 * 5/31 feature complete (almost)  48%
 * 5.25: cpu usage 1 chnael = 51%
 * 6/9 after re-write 40
 */

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;


template <class TBase>
class SubDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

template <class TBase>
class Sub : public TBase
{
public:

    Sub(Module * module) : TBase(module)
    {
    }
    Sub() : TBase()
    {
    }

    /**
    * re-calc everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
    void init();

    // 
    enum ParamIds
    {
        OCTAVE1_PARAM,
        OCTAVE2_PARAM,
        FINE1_PARAM,
        FINE2_PARAM,
        SUB1A_TUNE_PARAM,
        SUB2A_TUNE_PARAM,
        SUB1B_TUNE_PARAM,
        SUB2B_TUNE_PARAM,
        SUB1A_TUNE_TRIM_PARAM,
        SUB2A_TUNE_TRIM_PARAM,
        SUB1B_TUNE_TRIM_PARAM,
        SUB2B_TUNE_TRIM_PARAM,
        VCO1_LEVEL_PARAM,
        VCO2_LEVEL_PARAM,
        SUB1A_LEVEL_PARAM,
        SUB2A_LEVEL_PARAM,
        SUB1B_LEVEL_PARAM,
        SUB2B_LEVEL_PARAM,
        WAVEFORM1_PARAM,
        WAVEFORM2_PARAM,
        QUANTIZER_SCALE_PARAM,

        NUM_PARAMS
    };

    enum InputIds
    {
        VOCT_INPUT,
        SUB1A_TUNE_INPUT,
        SUB2A_TUNE_INPUT,
        SUB1B_TUNE_INPUT,
        SUB2B_TUNE_INPUT,

        NUM_INPUTS
    };

    enum OutputIds
    {
        MAIN_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<SubDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

    void stepn();
    void stepm();
    VoltageControlledOscillator<16, 16, rack::simd::float_4, rack::simd::int32_4>& _get(int n) {
        return oscillators[n];
    } 
private:

    std::shared_ptr<SimpleQuantizer> quantizer;
    VoltageControlledOscillator<16, 16, rack::simd::float_4, rack::simd::int32_4> oscillators[4];
    AudioMath::ScaleFun<float> divScaleFn = AudioMath::makeLinearScaler2<float>(1, 16, 1, 16);
     
    LookupTableParams<float> audioTaper;

    /**
     * number of oscillator pairs
     */
    int numDualChannels = 1;
    int numBanks = 1;

    Divider divn;
    Divider divm;

    float vco0Gain = .2;
    float subA0Gain = .2;
    float subB0Gain = .2;

    float vco1Gain = .2;
    float subA1Gain = .2;
    float subB1Gain = .2;


    int activeChannels_m[4] = {0};
    void computeGains();
    void computeDivisors(int32_4& divaOut, int32_4& divbOut);
    void setupWaveforms();
    void setupQuantizer();
};


template <class TBase>
inline void Sub<TBase>::init()
{
    LookupTableFactory<float>::makeAudioTaper(audioTaper);
    divn.setup(4, [this]() {
        this->stepn();
    });

    divm.setup(16, [this]() {
        this->stepm();
    });

    for (int i=0; i<4; ++i) {
        oscillators[i].index = i;
    }

    std::vector< SimpleQuantizer::Scales> scales = { SimpleQuantizer::Scales::_12Even };
    quantizer = std::make_shared<SimpleQuantizer>(scales,  SimpleQuantizer::Scales::_off);
}

template <class TBase>
inline void Sub<TBase>::computeGains()
{
    //LookupTable<float>::lookup(audioTaper, Sub<TBase>::params[Sub<TBase>::VCO1_LEVEL_PARAM].value);
    vco0Gain = LookupTable<float>::lookup(audioTaper,
        Sub<TBase>::params[Sub<TBase>::VCO1_LEVEL_PARAM].value);
    subA0Gain = LookupTable<float>::lookup(audioTaper, 
        Sub<TBase>::params[Sub<TBase>::SUB1A_LEVEL_PARAM].value);
    subB0Gain = LookupTable<float>::lookup(audioTaper,
        Sub<TBase>::params[Sub<TBase>::SUB1B_LEVEL_PARAM].value);

    vco1Gain = LookupTable<float>::lookup(audioTaper,
        Sub<TBase>::params[Sub<TBase>::VCO2_LEVEL_PARAM].value);
    subA1Gain = LookupTable<float>::lookup(audioTaper,
        Sub<TBase>::params[Sub<TBase>::SUB2A_LEVEL_PARAM].value);
    subB1Gain = LookupTable<float>::lookup(audioTaper,
        Sub<TBase>::params[Sub<TBase>::SUB2B_LEVEL_PARAM].value);
}

template <class TBase>
inline void Sub<TBase>::computeDivisors(
    int32_4& divaOut, int32_4& divbOut)
{
    const float div1ARawf = divScaleFn(
        Sub<TBase>::inputs[SUB1A_TUNE_INPUT].getVoltage(0),      // TODO: poly mod
        Sub<TBase>::params[SUB1A_TUNE_PARAM].value,
        Sub<TBase>::params[SUB1A_TUNE_TRIM_PARAM].value
    );
   
    const float div1BRawf = divScaleFn(
        Sub<TBase>::inputs[SUB1B_TUNE_INPUT].getVoltage(0),      // TODO: poly mod
        Sub<TBase>::params[SUB1B_TUNE_PARAM].value,
        Sub<TBase>::params[SUB1B_TUNE_TRIM_PARAM].value
    );
    const float div2ARawf = divScaleFn(
        Sub<TBase>::inputs[SUB2A_TUNE_INPUT].getVoltage(0),      // TODO: poly mod
        Sub<TBase>::params[SUB2A_TUNE_PARAM].value,
        Sub<TBase>::params[SUB2A_TUNE_TRIM_PARAM].value
    );

    const float div2BRawf = divScaleFn(
        Sub<TBase>::inputs[SUB2B_TUNE_INPUT].getVoltage(0),      // TODO: poly mod
        Sub<TBase>::params[SUB2B_TUNE_PARAM].value,
        Sub<TBase>::params[SUB2B_TUNE_TRIM_PARAM].value
    );

    divaOut[0] = int(div1ARawf);
    divaOut[1] = int(div2ARawf);
    divaOut[2] = int(div1ARawf);
    divaOut[3] = int(div2ARawf);

    divbOut[0] = int(div1BRawf);
    divbOut[1] = int(div2BRawf);
    divbOut[2] = int(div1BRawf);
    divbOut[3] = int(div2BRawf);
}

template <class TBase>
inline void Sub<TBase>::setupQuantizer()
{
    int scale = int(std::round(Sub<TBase>::params[QUANTIZER_SCALE_PARAM].value));
    quantizer->setScale( SimpleQuantizer::Scales(scale));
}



inline void parseWF(int wf, bool& mainIsSaw, bool& subIsSaw) {
    switch(wf) {
        case 0:
            mainIsSaw = true;
            subIsSaw = true;
            break;
        case 1:
            mainIsSaw = false;
            subIsSaw = false;
            break;
        case 2:
            mainIsSaw = false;
            subIsSaw = true;
            break;
        default:
            assert(0);
    }
}


// TODO: implement this smart (SIMD) and move
inline float_4 bitfieldToMask(int x)
{
    float_4 y = float_4::zero();
    int bit = 1;
    float_4 mask = float_4::mask();
    for (int index = 0 ; index < 4; ++index) {
        if (x & bit) {
            y[index] = mask[0];
        }
        bit <<= 1;
    }
    return y;
}

template <class TBase>
inline void Sub<TBase>::setupWaveforms() {
    // for now use vco1 for both
    int wf = int (std::round(Sub<TBase>::params[Sub<TBase>::WAVEFORM1_PARAM].value));
    bool mainAIsSaw = true;
    bool subAIsSaw = true;
    parseWF(wf, mainAIsSaw, subAIsSaw);

    wf = int (std::round(Sub<TBase>::params[Sub<TBase>::WAVEFORM2_PARAM].value));
    bool mainBIsSaw = true;
    bool subBIsSaw = true;
    parseWF(wf, mainBIsSaw, subBIsSaw);

 //   printf("amain=%d asub=%d bmain=%d bsub=%d\n", mainAIsSaw, subAIsSaw, mainBIsSaw, subBIsSaw);
 //   fflush(stdout);

    // ok, remember A and B 0 are two vcos, so each bank is only 2 voices
    int mainIsSawBitMask = 0;
    int subIsSawBitMask = 0;
    if (mainAIsSaw) {
        mainIsSawBitMask |= (1 | 4);
    }
    if (mainBIsSaw) {
        mainIsSawBitMask |= (2 | 8);
    }
    if (subAIsSaw) {
        subIsSawBitMask |= (1 | 4);
    }
    if (subBIsSaw) {
        subIsSawBitMask |= (2 | 8);
    }

 //   printf("in setup waveform mainIsSawBitMask=%x\n subIsSawBitMask=%x\n",mainIsSawBitMask,     subIsSawBitMask);


    float_4 mainIsSawMask = bitfieldToMask(mainIsSawBitMask);
    float_4 subIsSawMask = bitfieldToMask(subIsSawBitMask);
    // printf("in setup waveform main is saw mask: %s\n subIsSawMask: %s\n",    toStr(mainIsSawMask).c_str(),toStr(subIsSawMask).c_str());

    for (int bank = 0; bank < 4; ++bank) {
        oscillators[bank].setWaveform(mainIsSawMask, subIsSawMask);
    }
}

template <class TBase>
inline void Sub<TBase>::stepm()
{
    setupQuantizer();
    setupWaveforms();
    numDualChannels = std::max<int>(1, TBase::inputs[VOCT_INPUT].channels);
    Sub<TBase>::outputs[ Sub<TBase>::MAIN_OUTPUT].setChannels(numDualChannels);

    const int numVCO = numDualChannels * 2;
    numBanks = numVCO / 4;
    if (numVCO > numBanks * 4) {
        numBanks++;
    }

    // Figure out how many active channels per VCO
    // TODO: imp smarter and/or do less often
    activeChannels_m[0] = 0;
    activeChannels_m[1] = 0;
    activeChannels_m[2] = 0;
    activeChannels_m[3] = 0;

    if (numDualChannels <= 2 ) {
        activeChannels_m[0] = numDualChannels * 2;    
    } else if (numDualChannels <= 4) {
        activeChannels_m[0] = 4;
        activeChannels_m[1] = (numDualChannels-2) * 2;
    } else if (numDualChannels <= 6) {
        activeChannels_m[0] = 4;
        activeChannels_m[1] = 4;
        activeChannels_m[2] = (numDualChannels-4) * 2;
    } else if (numDualChannels <= 8) {
        activeChannels_m[0] = 4;
        activeChannels_m[1] = 4;
        activeChannels_m[2] = 4;
        activeChannels_m[3] = (numDualChannels-6) * 2;
    } else {
        assert(false);
    }
}  

template <class TBase>
inline void Sub<TBase>::stepn()
{
    computeGains();

    // get the base pitch in volts from the 2X2 pitch knobs.
    // Jam it into one float_4 <A,B,A,B>
    const float basePitch1 = Sub<TBase>::params[OCTAVE1_PARAM].value + Sub<TBase>::params[FINE1_PARAM].value - 4;
    const float basePitch2 = Sub<TBase>::params[OCTAVE2_PARAM].value + Sub<TBase>::params[FINE2_PARAM].value - 4;
    float_4 combinedPitch(0);

    combinedPitch[0] = basePitch1;
    combinedPitch[1] = basePitch2;

    combinedPitch[2] = basePitch1;
    combinedPitch[3] = basePitch2;

    rack::simd::int32_4 divisorA;
    rack::simd::int32_4 divisorB;
    computeDivisors(divisorA, divisorB);

    // Now loop thought all VCOs, combining the individual CV with the
    int channel = 0;
    for (int bank = 0; bank < numBanks; ++bank) {
        const float cv0 = quantizer->quantize(Sub<TBase>::inputs[VOCT_INPUT].getVoltage(channel));
        ++channel;
        float_4 pitch = combinedPitch;
        pitch[0] += cv0;
        pitch[1] += cv0;

        const float cv1 = quantizer->quantize(Sub<TBase>::inputs[VOCT_INPUT].getVoltage(channel));
        ++channel;
        pitch[2] += cv1;
        pitch[3] += cv1;
        
        oscillators[bank].setupSub(activeChannels_m[bank], pitch, divisorA, divisorB);
    }

    for (int bank = numBanks; bank < 4; ++bank) {
        oscillators[bank].setupSub(activeChannels_m[bank], float_4(0), 4, 4);
    }
}

template <class TBase>
inline void Sub<TBase>::step()
{
    divm.step();
    divn.step();
    // look at controls and update VCO

    // run the audio
    const float sampleTime = TBase::engineGetSampleTime();
    
    // Prepare the mixed output and send it out.
    int channel = 0; 
    for (int bank=0; bank < numBanks; ++bank) {
        oscillators[bank].process(sampleTime, 0);

        // now, what do do with the output? to now lets grab pairs
        // of saws and add them.
        // TODO: make poly so it works
        float_4 mains = oscillators[bank].main();
        float_4 subs0 = oscillators[bank].sub(0);
        float_4 subs1 = oscillators[bank].sub(1);

        const float mixed0 = mains[0] * vco0Gain +
            mains[1] * vco1Gain +
            subs0[0] * subA0Gain +
            subs0[1] * subA1Gain +
            subs1[0] * subB0Gain +
            subs1[1] * subB1Gain;

         const float mixed1 = mains[2] * vco0Gain +
            mains[3] * vco1Gain +
            subs0[2] * subA0Gain +
            subs0[3] * subA1Gain +
            subs1[2] * subB0Gain +
            subs1[3] * subB1Gain;

        static bool firstTime = true;
        const float limit = 10;
        if ( (mixed0 >= limit/2) ||
            (mixed0 <= -limit/2) ||
            (mixed1 >= limit/2) ||
            (mixed1 <= -limit/2) ||
            firstTime) {
                printf(firstTime ?  "fist time\n" : "about to crash\n");
                printf("mixed0 = %f, mixed1 = %f\n", mixed0, mixed1);
                printf("mains = %f, %f %f %f\n", mains[0],  mains[1],  mains[2],  mains[3]);
                printf("SUBS0= %s\n", toStr(subs0).c_str());
                printf("SUBS1= %s\n", toStr(subs1).c_str());
                printf("gains = %f %f, %f,%f,%f,%f\n", vco0Gain, vco1Gain, 
                    subA0Gain, subA1Gain, subB0Gain, subB1Gain);
                fflush(stdout);
                firstTime = false;
            }

        assert(mixed0 < limit);
        assert(mixed0 > -limit);
        assert(mixed1 < limit);
        assert(mixed1 > -limit);

        Sub<TBase>::outputs[MAIN_OUTPUT].setVoltage(mixed0, channel++);
        Sub<TBase>::outputs[MAIN_OUTPUT].setVoltage(mixed1, channel++); 
    }
}

template <class TBase>
int SubDescription<TBase>::getNumParams()
{
    return Sub<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config SubDescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case Sub<TBase>::OCTAVE1_PARAM:
            ret = {0, 10, 4, "VCO 1 octave"};
            break;
        case Sub<TBase>::OCTAVE2_PARAM:
            ret = {0, 10, 4, "VCO 2 octave"};
            break;
        case Sub<TBase>::FINE1_PARAM:
            ret = {-1, 1, 0, "VCO 1 fine tune"};
            break;
        case Sub<TBase>::FINE2_PARAM:
            ret = {-1, 1, 0, "VCO 2 fine tune"};
            break;
        case Sub<TBase>::SUB1A_TUNE_PARAM:
            ret = {1, 16, 4, "VCO 1 subharmonic A divisor"};
            break;
        case Sub<TBase>::SUB2A_TUNE_PARAM:
            ret = {1, 16, 4, "VCO 2 subharmonic A divisor"};
            break;
        case Sub<TBase>::SUB1B_TUNE_PARAM:
            ret = {1, 16, 4, "VCO 1 subharmonic B divisor"};
            break;
        case Sub<TBase>::SUB2B_TUNE_PARAM:
            ret = {1, 16, 4, "VCO 2 subharmonic B divisor"};
            break;
        case Sub<TBase>::VCO1_LEVEL_PARAM:
            ret = {0, 1, .5, "VCO 1 level"};
            break;
        case Sub<TBase>::VCO2_LEVEL_PARAM:
            ret = {0, 1, .5, "VCO 2 level"};
            break;
        case Sub<TBase>::SUB1A_LEVEL_PARAM:
            ret = {0, 1, 0, "VCO 1 subharmonic A level"};
            break;
        case Sub<TBase>::SUB2A_LEVEL_PARAM:
            ret = {0, 1, 0, "VCO 2 subharmonic A level"};
            break;
        case Sub<TBase>::SUB1B_LEVEL_PARAM:
            ret = {0, 1, 0, "VCO 1 subharmonic B level"};
            break;
        case Sub<TBase>::SUB2B_LEVEL_PARAM:
            ret = {0, 1, 0, "VCO 2 subharmonic B level"};
            break;
        case Sub<TBase>::WAVEFORM1_PARAM:
            ret = {0, 2, 0, "VCO 1 waveform"};
            break;
        case Sub<TBase>::WAVEFORM2_PARAM:
            ret = {0, 2, 0, "VCO 2 waveform"};
            break;
        case Sub<TBase>::SUB1A_TUNE_TRIM_PARAM:
            ret = {-1, 1, 0, "VCO 1 sub A trim"};
            break;
        case Sub<TBase>::SUB1B_TUNE_TRIM_PARAM:
            ret = {-1, 1, 0, "VCO 1 sub V trim"};
            break;
        case Sub<TBase>::SUB2A_TUNE_TRIM_PARAM:
            ret = {-1, 1, 0, "VCO 2 sub A trim"};
            break;
        case Sub<TBase>::SUB2B_TUNE_TRIM_PARAM:
            ret = {-1, 1, 0, "VCO 2 sub B trim"};
            break;
        case Sub<TBase>::QUANTIZER_SCALE_PARAM:
            ret = {0, 4, 0, "Quantizer Scale"};
            break;
        default:
            assert(false);
    }
    return ret;
}
#endif



