
#pragma once

#include "BasicVCO.h"
#include "Divider.h"
#include "IComposite.h"

#include "engine/Port.hpp"

#include <assert.h>
#include <memory>

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;


template <class TBase>
class BasicDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 * with switch:
 *      1 saw 6.16
 *      1 tri 4.78
 *   with jumo:
 *      1 tri : 3.73
 *      1 saw, 6.2
 */
template <class TBase>
class Basic : public TBase
{
public:

    enum class Waves
    {
        SIN,
        TRI,
        SAW,
        SQUARE,
        EVEN,
        SIN_CLEAN,
        TRI_CLEAN,
        END     // just a marker
    };

    Basic(Module * module) : TBase(module)
    {
    }
    Basic() : TBase()
    {
    }

    /**
    * re-calc everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
    void init();

    static std::string getLabel(Waves);

    enum ParamIds
    {
        OCTAVE_PARAM,
        SEMITONE_PARAM,
        FINE_PARAM,
        FM_PARAM,
        PW_PARAM,
        PWM_PARAM,
        WAVEFORM_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        VOCT_INPUT,
        PWM_INPUT,
        FM_INPUT,
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
        return std::make_shared<BasicDescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    //void step() override;
    void process(const typename TBase::ProcessArgs& args) override;

private:
    BasicVCO vcos[4];
    int numChannels_m = 1;      // 1..16
    int numBanks_m = 0;
    float basePitch_m = 0;
    float basePitchMod_m = 0;
    float basePw_m = 0;
    float basePwm_m = 0;
    
    BasicVCO::processFunction pProcess = nullptr;
    std::shared_ptr<LookupTableParams<float>> bipolarAudioLookup = ObjectCache<float>::getBipolarAudioTaper();

    Divider divn;
    Divider divm;

    void stepn();
    void stepm();

    void updatePitch();
    void updateBasePitch();
    void updatePwm();
    void updateBasePwm();
};


template <class TBase>
inline std::string Basic<TBase>::getLabel(Waves wf)
{
   // printf("get label %d\n", wf);
    switch(wf) {
        case Waves::SIN: return "sin";
        case Waves::TRI: return "tri";
        case Waves::SAW: return "saw";
        case Waves::SQUARE: return "square";
        case Waves::EVEN: return "even";
        case Waves::SIN_CLEAN: return "sin clean";
        case Waves::TRI_CLEAN: return "tri clean";
        case Waves::END:
        default:  assert(false); return "unk";
    }
}


template <class TBase>
inline void Basic<TBase>::init()
{
    divn.setup(4, [this]() {
        this->stepn();
    });
    divm.setup(16, [this]() {
        this->stepm();
    });
}

template <class TBase>
inline void Basic<TBase>::stepm()
{
    numChannels_m = std::max<int>(1, TBase::inputs[VOCT_INPUT].channels);
    Basic<TBase>::outputs[MAIN_OUTPUT].setChannels(numChannels_m);

    numBanks_m = (numChannels_m / 4);
    numBanks_m +=((numChannels_m %4) == 0) ? 0 : 1;

#ifdef _VCOJUMP
    auto wf = BasicVCO::Waveform((int)TBase::params[WAVEFORM_PARAM].value);
    pProcess = vcos[0].getProcPointer(wf);
#else
    for (int i=0; i<numBanks_m; ++i) {
        vcos[i].setWaveform((BasicVCO::Waveform)(int)TBase::params[WAVEFORM_PARAM].value);
    }
#endif
    updateBasePitch();
    updateBasePwm();
}

template <class TBase>
inline void Basic<TBase>::updateBasePwm()
{
    // 0..1
    basePw_m = Basic<TBase>::params[PW_PARAM].value / 100.f;

    // -1..1
    auto rawTrim = Basic<TBase>::params[PWM_PARAM].value / 100.f;
    auto taperedTrim = LookupTable<float>::lookup(*bipolarAudioLookup, rawTrim);
    basePwm_m = taperedTrim; 
}


template <class TBase>
inline void Basic<TBase>::updateBasePitch()
{
    basePitch_m = 
        Basic<TBase>::params[OCTAVE_PARAM].value +
        Basic<TBase>::params[SEMITONE_PARAM].value / 12.f +
        Basic<TBase>::params[FINE_PARAM].value / 12 - 4;

    auto rawTrim =  Basic<TBase>::params[FM_PARAM].value / 100;
    auto taperedTrim = LookupTable<float>::lookup(*bipolarAudioLookup, rawTrim);
    basePitchMod_m = taperedTrim; 
  //  printf("basePitch = %f, mod = %f\n", basePitch_m, basePitchMod_m);

        // they both seem to key off fm - no one watch voct?
#if 0
    printf("voct connected = %d, fm=%d\n", 
        TBase::inputs[VOCT_INPUT].isConnected(),
        TBase::inputs[FM_INPUT].isConnected());
#endif
}

template <class TBase>
inline void Basic<TBase>::stepn()
{
    updatePitch();
    updatePwm();
}

template <class TBase>
inline void Basic<TBase>::updatePwm()
{
    for (int bank = 0; bank < numBanks_m; ++ bank) {
        const int baseIndex = bank * 4;
        Port& p = TBase::inputs[PWM_INPUT];

        const float_4 pwmSignal = p.getPolyVoltageSimd<float_4>(baseIndex) * .1;
        float_4 combinedPW = pwmSignal * basePwm_m + basePw_m;
#if 0
  if (bank == 0) {
            printf("\ncombinedPW = %f basePWM_m = %f\n", combinedPW[0], basePwm_m);
            printf("signal = %f basePw_m = %f\n", pwmSignal[0], basePw_m);
        }
#endif
        combinedPW = rack::simd::clamp(combinedPW, 0, 1);
        vcos[bank].setPw(combinedPW);
    }
}

template <class TBase>
inline void Basic<TBase>::updatePitch()
{
    const float sampleTime = TBase::engineGetSampleTime();
    for (int bank = 0; bank < numBanks_m; ++ bank) {
        const int baseIndex = bank * 4;
        Port& pVoct = TBase::inputs[VOCT_INPUT];
        const float_4 pitchCV = pVoct.getVoltageSimd<float_4>(baseIndex);

        Port& pFM = TBase::inputs[FM_INPUT];
        const float_4 fmInput = pFM.getPolyVoltageSimd<float_4>(baseIndex) * basePitchMod_m;
        const float_4 totalCV = pitchCV + basePitch_m + fmInput;
        vcos[bank].setPitch(totalCV, sampleTime);
    }
}
template <class TBase>
inline void Basic<TBase>::process(const typename TBase::ProcessArgs& args)
{
    divn.step();
    divm.step();

#ifdef _VCOJUMP
    for (int bank = 0; bank < numBanks_m; ++ bank) {
        //  float_4 output = vcos[bank].process(args.sampleTime);
        float_4 output = ((&vcos[bank])->*pProcess)(args.sampleTime);
        Basic<TBase>::outputs[MAIN_OUTPUT].setVoltageSimd(output, bank * 4);
    }
#else
    for (int bank = 0; bank < numBanks_m; ++ bank) {
        float_4 output = vcos[bank].process(args.sampleTime);
        Basic<TBase>::outputs[MAIN_OUTPUT].setVoltageSimd(output, bank * 4);
    }
#endif
}

template <class TBase>
int BasicDescription<TBase>::getNumParams()
{
    return Basic<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config BasicDescription<TBase>::getParam(int i)
{
    const float numWaves = (float) Basic<TBase>::Waves::END;
    const float defWave = (float) Basic<TBase>::Waves::SIN;
    Config ret(0, 1, 0, "");
    switch (i) {
        case Basic<TBase>::OCTAVE_PARAM:
            ret = {0, 10, 4, "Octave"};
            break;
        case Basic<TBase>::SEMITONE_PARAM:
             ret = {-11.f, 11.0f, 0.f, "Semitone transpose"};
            break;
        case Basic<TBase>::FINE_PARAM:
            ret = {-1.0f, 1, 0, "fine tune"};
            break;
        case Basic<TBase>::FM_PARAM:
            ret = {-100.0f, 100, 0, "FM"};
            break;
        case Basic<TBase>::WAVEFORM_PARAM:
            ret = {0.0f, numWaves-1, defWave, "Waveform"};
            break;  
        case Basic<TBase>::PW_PARAM:
            ret = {0.0f, 100, 50, "pulse width"};
            break;
        case Basic<TBase>::PWM_PARAM:
            ret = {-100.0f, 100, 0, "pulse width modulation depth"};
            break;
        default:
            assert(false);
    }
    return ret;
}

