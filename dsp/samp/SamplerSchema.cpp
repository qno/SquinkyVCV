#include "SamplerSchema.h"

#include <assert.h>

#include <set>

#include "SParse.h"

using Opcode = SamplerSchema::Opcode;
using OpcodeType = SamplerSchema::OpcodeType;
using DiscreteValue = SamplerSchema::DiscreteValue;

// TODO: compare this to the spec
static std::map<Opcode, OpcodeType> keyType = {
    {Opcode::HI_KEY, OpcodeType::Int},
    {Opcode::KEY, OpcodeType::Int},
    {Opcode::LO_KEY, OpcodeType::Int},
    {Opcode::HI_VEL, OpcodeType::Int},
    {Opcode::LO_VEL, OpcodeType::Int},
    {Opcode::SAMPLE, OpcodeType::String},
    {Opcode::AMPEG_RELEASE, OpcodeType::Float},
    {Opcode::LOOP_MODE, OpcodeType::Discrete},
    {Opcode::PITCH_KEYCENTER, OpcodeType::Int},
    {Opcode::LOOP_START, OpcodeType::Int},
    {Opcode::LOOP_END, OpcodeType::Int},
    {Opcode::PAN, OpcodeType::Int},
    {Opcode::GROUP, OpcodeType::Int},
    {Opcode::TRIGGER, OpcodeType::Discrete},
    {Opcode::VOLUME, OpcodeType::Float},
    {Opcode::TUNE, OpcodeType::Int},
    {Opcode::OFFSET, OpcodeType::Int},
    {Opcode::POLYPHONY, OpcodeType::Int},
    {Opcode::PITCH_KEYTRACK, OpcodeType::Int},
    {Opcode::AMP_VELTRACK, OpcodeType::Float},
    {Opcode::LO_RAND, OpcodeType::Float},
    {Opcode::HI_RAND, OpcodeType::Float},
    {Opcode::SEQ_LENGTH, OpcodeType::Int},
    {Opcode::SEQ_POSITION, OpcodeType::Int}};

static std::map<std::string, Opcode> opcodes = {
    {"hivel", Opcode::HI_VEL},
    {"lovel", Opcode::LO_VEL},
    {"hikey", Opcode::HI_KEY},
    {"lokey", Opcode::LO_KEY},
    {"hirand", Opcode::HI_RAND},
    {"lorand", Opcode::LO_RAND},
    {"pitch_keycenter", Opcode::PITCH_KEYCENTER},
    {"ampeg_release", Opcode::AMPEG_RELEASE},
    {"loop_mode", Opcode::LOOP_MODE},
    {"loop_continuous", Opcode::LOOP_CONTINUOUS},
    {"loop_start", Opcode::LOOP_START},
    {"loop_end", Opcode::LOOP_END},
    {"sample", Opcode::SAMPLE},
    {"pan", Opcode::PAN},
    {"group", Opcode::GROUP},
    {"trigger", Opcode::TRIGGER},
    {"volume", Opcode::VOLUME},
    {"tune", Opcode::TUNE},
    {"offset", Opcode::OFFSET},
    {"polyphony", Opcode::POLYPHONY},
    {"pitch_keytrack", Opcode::PITCH_KEYTRACK},
    {"amp_veltrack", Opcode::AMP_VELTRACK},
    {"key", Opcode::KEY},
    {"seq_length", Opcode::SEQ_LENGTH},
    {"seq_position", Opcode::SEQ_POSITION}};

static std::set<std::string> unrecognized;

static std::map<std::string, DiscreteValue> discreteValues = {
    {"loop_continuous", DiscreteValue::LOOP_CONTINUOUS},
    {"loop_sustain", DiscreteValue::LOOP_SUSTAIN},
    {"no_loop", DiscreteValue::NO_LOOP},
    {"one_shot", DiscreteValue::ONE_SHOT},
    {"attack", DiscreteValue::ATTACK},
    {"release", DiscreteValue::RELEASE},
};

DiscreteValue SamplerSchema::translated(const std::string& s) {
    auto it = discreteValues.find(s);
    if (it == discreteValues.end()) {
        printf("isn't discrete: %s\n", s.c_str());
        return DiscreteValue::NONE;
    }
    return it->second;
}

void SamplerSchema::compile(SamplerSchema::KeysAndValuesPtr results, SKeyValuePairPtr input) {
    //printf("translating opcode: %s\n", input->key.c_str());
    Opcode opcode = translate(input->key);
    if (opcode == Opcode::NONE) {
        printf("could not translate opcode %s\n", input->key.c_str());
        fflush(stdout);
        return;
    }
    auto typeIter = keyType.find(opcode);
    if (typeIter == keyType.end()) {
        printf("could not find type for %s\n", input->key.c_str());
        fflush(stdout);
        assert(false);
        return;
    }

    const OpcodeType type = typeIter->second;
    ValuePtr vp = std::make_shared<Value>();
    vp->type = type;
    bool isValid = true;
    switch (type) {
        case OpcodeType::Int:
            try {
                int x = std::stoi(input->value);
                vp->numericInt = x;
            } catch (std::exception&) {
                isValid = false;
                printf("could not convert %s to number. key=%s\n", input->value.c_str(), input->key.c_str());
                return;
            }
            break;
        case OpcodeType::Float:
            try {
                float x = std::stof(input->value);
                vp->numericFloat = x;
            } catch (std::exception&) {
                isValid = false;
                printf("could not convert %s to number. key=%s\n", input->value.c_str(), input->key.c_str());
                return;
            }
            break;
        case OpcodeType::String:
            vp->string = input->value;
            break;
        case OpcodeType::Discrete: {
            DiscreteValue dv = translated(input->value);
            assert(dv != DiscreteValue::NONE);
            vp->discrete = dv;
        } break;
        default:
            assert(false);
    }
    if (isValid) {
        results->add(opcode, vp);
    }
}

SamplerSchema::KeysAndValuesPtr SamplerSchema::compile(const SKeyValueList& inputs) {
    SamplerSchema::KeysAndValuesPtr results = std::make_shared<SamplerSchema::KeysAndValues>();
    for (auto input : inputs) {
        compile(results, input);
    }
    return results;
}

SamplerSchema::Opcode SamplerSchema::translate(const std::string& s) {
    auto entry = opcodes.find(s);
    if (entry == opcodes.end()) {
        auto find2 = unrecognized.find(s);
        if (find2 == unrecognized.end()) {
            unrecognized.insert({s});
            printf("!! unrecognized opcode %s\n", s.c_str());
        }

        return Opcode::NONE;
    } else {
        return entry->second;
    }
}
