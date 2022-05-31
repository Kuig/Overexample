#pragma once
#include <JuceHeader.h>

#define TARGET_SAMPLING_RATE 192000.0

class OversampledSaturation
{
public:
    OversampledSaturation() {}

    ~OversampledSaturation() {}

    void prepare(const dsp::ProcessSpec& spec) noexcept
    {
        specs = spec;

        oversamplingFactor = jmax(roundToInt(TARGET_SAMPLING_RATE / specs.sampleRate), 1);
        overSampledRate = specs.sampleRate * oversamplingFactor;

        oversampler.reset(new dsp::Oversampling<float>(
            specs.numChannels, 
            log2(oversamplingFactor), 
            dsp::Oversampling<float>::FilterType::filterHalfBandFIREquiripple, 
            true, 
            true));

        oversampler->initProcessing(specs.maximumBlockSize);
        oversampler->reset();
    }

    //template <typename SampleType>
    //SampleType processSample(SampleType s) noexcept
    //{
    //    return s;
    //}

    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept
    {
        auto&& inBlock = context.getInputBlock();
        auto&& outBlock = context.getOutputBlock();

        jassert(inBlock.getNumChannels() == outBlock.getNumChannels());
        jassert(inBlock.getNumSamples() == outBlock.getNumSamples());

        if (context.isBypassed)
        {
            if (context.usesSeparateInputAndOutputBlocks())
                outBlock.copyFrom(inBlock);

            return;
        }

        auto oversampledBlock = oversampler->processSamplesUp(inBlock);

        auto len = oversampledBlock.getNumSamples();
        auto numChannels = oversampledBlock.getNumChannels();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = oversampledBlock.getChannelPointer(ch);

            for (size_t i = 0; i < len; ++i)
                data[i] = atan(gain * data[i]) * invAtanGain;
        }

        oversampler->processSamplesDown(outBlock);
    }

    int getLatency()
    {
        return roundToInt(oversampler->getLatencyInSamples());
    }

    double getOversampledFrequency()
    {
        return overSampledRate;
    }

    void setGain(float newValue)
    {
        gain = newValue;

        jassert(gain != 0.0f);
        invAtanGain = 1.0f / atan(gain);
    }

private:
    double overSampledRate = 0;
    dsp::ProcessSpec specs{0,0,0};
    std::unique_ptr<dsp::Oversampling<float>> oversampler;
    int oversamplingFactor = 1;
    int latency = 0;

    float gain = 1.0f;
    float invAtanGain = 1.0f / atan(1.0f);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OversampledSaturation)
};