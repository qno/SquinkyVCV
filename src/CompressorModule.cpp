
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"


#include "Compressor.h"
#include "ctrl/PopupMenuParamWidgetv1.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"
#include "ctrl/SqTooltips.h"
#include "ctrl/SqWidgets.h"

using Comp = Compressor<WidgetComposite>;



class LambdaQuantity : public SqTooltips::SQParamQuantity {
public:
    LambdaQuantity(const ParamQuantity& other) : 
        SqTooltips::SQParamQuantity(other) 
    {
    }
    
    std::string getDisplayValueString() override {
        auto value = getValue();
        auto expValue = expFunction(value);
        std::stringstream str;
        str << expValue;
        if (!suffix.empty()) {
            str << suffix;
        }
        return str.str();
    }
protected:
    std::function<double(double)> expFunction;
    std::string suffix;
};

class AttackQuantity : public LambdaQuantity {
public:
    AttackQuantity(const ParamQuantity& other) : LambdaQuantity(other)
    {
        expFunction = Comp::getSlowAttackFunction();
        suffix = " mS";
    }
};

class ReleaseQuantity : public LambdaQuantity {
public:
    ReleaseQuantity(const ParamQuantity& other) : LambdaQuantity(other)
    {
        expFunction = Comp::getSlowReleaseFunction();
        suffix = " mS";
    }
};

class ThresholdQuantity : public LambdaQuantity {
public:
    ThresholdQuantity(const ParamQuantity& other) : LambdaQuantity(other)
    {
        expFunction = Comp::getSlowThresholdFunction();
        suffix = " V";
    }
};

class MakeupGainQuantity : public LambdaQuantity {
public:
    MakeupGainQuantity(const ParamQuantity& other) : LambdaQuantity(other)
    {
        expFunction = [](double x) {
            return x;
        };
        suffix = " dB";
    }
};

/**
 */
struct CompressorModule : Module
{
public:
    CompressorModule();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> compressor;
private:
};

CompressorModule::CompressorModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    compressor = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    // customize tooltips for some params.
    SqTooltips::changeParamQuantity<AttackQuantity>(this, Comp::ATTACK_PARAM);
    SqTooltips::changeParamQuantity<ReleaseQuantity>(this, Comp::RELEASE_PARAM);
    SqTooltips::changeParamQuantity<ThresholdQuantity>(this, Comp::THRESHOLD_PARAM);
    SqTooltips::changeParamQuantity<MakeupGainQuantity>(this, Comp::MAKEUPGAIN_PARAM);

    onSampleRateChange();
    compressor->init();
}

void CompressorModule::process(const ProcessArgs& args)
{
    compressor->process(args);
}

void CompressorModule::onSampleRateChange()
{
    compressor->onSampleRateChange();
}

////////////////////
// module widget
////////////////////

struct CompressorWidget : ModuleWidget
{
    CompressorWidget(CompressorModule *);
    DECLARE_MANUAL("Compressor Manual", "https://github.com/squinkylabs/SquinkyVCV/blob/f2/docs/compressor.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void addJacks(CompressorModule *module, std::shared_ptr<IComposite> icomp);
    void addControls(CompressorModule *module, std::shared_ptr<IComposite> icomp);
};

void CompressorWidget::addControls(CompressorModule *module, std::shared_ptr<IComposite> icomp)
{
    const float knobX = 10;
    const float knobX2 = 47;
    const float knobY = 46;
    const float labelY = knobY - 20;
    const float dy = 54;

    addLabel(
        Vec(knobX - 4, labelY + 0 * dy),
        "Atck");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 0 * dy),
        module,  Comp::ATTACK_PARAM));

    addLabel(
        Vec(knobX2 - 2, labelY + 0 * dy),
        "Rel");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX2, knobY + 0 * dy),
        module,  Comp::RELEASE_PARAM));

    addLabel(
        Vec(knobX - 12, labelY + 1 * dy),
        "Thrsh");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 1 * dy),
        module,  Comp::THRESHOLD_PARAM));
    
    addLabel(
        Vec(knobX2 - 13, labelY + 1 * dy),
        "Dry/wet");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX2, knobY + 1 * dy),
        module,  Comp::WETDRY_PARAM));

     addLabel(
        Vec(knobX - 10, labelY + 2 * dy),
        "Makeup");
    addParam(SqHelper::createParam<Blue30Knob>(
        icomp,
        Vec(knobX, knobY + 2 * dy),
        module,  Comp::MAKEUPGAIN_PARAM));    

    std::vector<std::string> labels = Comp::ratios();
    PopupMenuParamWidget* p = SqHelper::createParam<PopupMenuParamWidget>(
        icomp,
        Vec(knobX,  knobY + 3 * dy),
        module,
        Comp::RATIO_PARAM);
    p->box.size.x = 70;  // width
    p->box.size.y = 22;   
    p->text = labels[0];
    p->setLabels(labels);
    addParam(p);

    addLabel(
        Vec(knobX + 18, labelY + 4 * dy- 6),
        "< IM");
    addParam(SqHelper::createParam<CKSS>(
        icomp,
        Vec(knobX + 28, 5 + knobY + 4 * dy - 6),
        module,  Comp::REDUCEDISTORTION_PARAM));
}

void CompressorWidget::addJacks(CompressorModule *module, std::shared_ptr<IComposite> icomp)
{
    const float jackX = 10;
    const float jackX2 = 50;
    const float labelX = jackX - 6;
    const float label2X = jackX2 - 6;

    const float jackY = 300;
    const float labelY = jackY - 17;
    const float dy = 40;

    addLabel(
        Vec(labelX+6, labelY + 0 * dy),
        "In");
    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY),
        module,
        Comp::AUDIO_INPUT));

    addLabel(
        Vec(label2X, labelY + 0 * dy),
        "Out");
    addOutput(createOutput<PJ301MPort>(
        Vec(jackX2, jackY + 0 * dy),
        module,
        Comp::AUDIO_OUTPUT));
#if 0
    addLabel(
        Vec(labelX, labelY + 2 * dy),
        "dbg");
     addOutput(createOutput<PJ301MPort>(
        Vec(jackX, jackY + 2 * dy),
        module,
        Comp::DEBUG_OUTPUT));
 #endif
}


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

CompressorWidget::CompressorWidget(CompressorModule *module)
{
    setModule(module);
    SqHelper::setPanel(this, "res/compressor_panel.svg");

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addControls(module, icomp);
    addJacks(module, icomp);
    

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
   // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
   // addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelCompressorModule = createModel<CompressorModule, CompressorWidget>("squinkylabs-comp");


