
#pragma once

#include <assert.h>
#include <memory>
//#include "functions.hpp"
#include <simd/vector.hpp>
#include <simd/functions.hpp>
#include "IComposite.h"

using float_4 = rack::simd::float_4;

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;


template <class TBase>
class WVCODescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

class WVCODsp
{
public:
    float_4 step(float_4 freq) {
        static int times =0;
        times++;
       
        
        phaseAcc += freq;

        const int limit = 0;
         if (times < limit) {
            printf("freq = %.2f, %.2f, %.2f, %.2f\n", freq[0], freq[1], freq[2], freq[3]);
            printf("ph = %.2f, %.2f, %.2f, %.2f\n", phaseAcc[0], phaseAcc[1], phaseAcc[2], phaseAcc[3]);
        }

		// Wrap phase
		phaseAcc -= rack::simd::floor(phaseAcc);

        const __m128 twoPi = _mm_set_ps1(2 * 3.141592653589793238);
        float_4 s = rack::simd::sin(phaseAcc * twoPi);
      // float_4 s = phaseAcc;
         if (times < limit) {
            printf("s = %.2f, %.2f, %.2f, %.2f\n", s[0], s[1], s[2], s[3]);
         }
        return s;
    }
private:
    float_4 phaseAcc = float_4::zero();
};

template <class TBase>
class WVCO : public TBase
{
public:

    WVCO(Module * module) : TBase(module)
    {
    }
    WVCO() : TBase()
    {
    }

    /**
    * re-calc everything that changes with sample
    * rate. Also everything that depends on baseFrequency.
    *
    * Only needs to be called once.
    */
    void init();

    enum ParamIds
    {
        TEST_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
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
        return std::make_shared<WVCODescription<TBase>>();
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

private:

    WVCODsp dsp[4];

};


template <class TBase>
inline void WVCO<TBase>::init()
{
}


template <class TBase>
inline void WVCO<TBase>::step()
{
    for (int bank=0; bank<4; ++bank) {
        const int channel = 4 * bank;
        float_4 v = dsp[bank].step(.005);       // was .005
        WVCO<TBase>::outputs[MAIN_OUTPUT].setVoltageSimd(v, channel);
    }
    // why can't I set to 16?
    WVCO<TBase>::outputs[ WVCO<TBase>::MAIN_OUTPUT].setChannels(15);
}

template <class TBase>
int WVCODescription<TBase>::getNumParams()
{
    return WVCO<TBase>::NUM_PARAMS;
}

template <class TBase>
inline IComposite::Config WVCODescription<TBase>::getParam(int i)
{
    Config ret(0, 1, 0, "");
    switch (i) {
        case WVCO<TBase>::TEST_PARAM:
            ret = {-1.0f, 1.0f, 0, "Test"};
            break;
        default:
            assert(false);
    }
    return ret;
}

