
#include <sstream>
#include "Squinky.hpp"
#include "WidgetComposite.h"


#include "Lim.h"
#include "ctrl/SqHelper.h"
#include "ctrl/SqMenuItem.h"

using Comp = Lim<WidgetComposite>;

/**
 */
struct LimModule : Module
{
public:
    LimModule();
    /**
     *
     * Overrides of Module functions
     */
    void process(const ProcessArgs& args) override;
    void onSampleRateChange() override;

    std::shared_ptr<Comp> lim;
private:

};

LimModule::LimModule()
{
    config(Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
    lim = std::make_shared<Comp>(this);
    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    SqHelper::setupParams(icomp, this); 

    onSampleRateChange();
    lim->init();
}

void LimModule::process(const ProcessArgs& args)
{
    lim->process(args);
}

void LimModule::onSampleRateChange()
{
    lim->onSampleRateChange();
}

////////////////////
// module widget
////////////////////

struct LimWidget : ModuleWidget
{
    LimWidget(LimModule *);
    DECLARE_MANUAL("Blank Manul", "https://github.com/squinkylabs/SquinkyVCV/blob/main/docs/booty-shifter.md");

    Label* addLabel(const Vec& v, const char* str, const NVGcolor& color = SqHelper::COLOR_BLACK)
    {
        Label* label = new Label();
        label->box.pos = v;
        label->text = str;
        label->color = color;
        addChild(label);
        return label;
    }

    void addJacks(LimModule *module, std::shared_ptr<IComposite> icomp);
};

void LimWidget::addJacks(LimModule *module, std::shared_ptr<IComposite> icomp)
{
    const float jackX = 14;
    const float jackY = 249;
    const float dy = 30;

    addInput(createInput<PJ301MPort>(
        Vec(jackX, jackY + 0 * dy),
        module,
        Comp::AUDIO_INPUT));
    addOutput(createOutput<PJ301MPort>(
        Vec(jackX, jackY + 1 * dy),
        module,
        Comp::AUDIO_OUTPUT));

     addOutput(createOutput<PJ301MPort>(
        Vec(jackX, 40),
        module,
        Comp::DEBUG_OUTPUT));
}


/**
 * Widget constructor will describe my implementation structure and
 * provide meta-data.
 * This is not shared by all modules in the DLL, just one
 */

LimWidget::LimWidget(LimModule *module)
{
    setModule(module);
    SqHelper::setPanel(this, "res/lim_panel.svg");

    std::shared_ptr<IComposite> icomp = Comp::getDescription();
    addJacks(module, icomp);

    // screws
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
   // addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
   // addChild( createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
}

Model *modelLimModule = createModel<LimModule, LimWidget>("squinkylabs-lim");

