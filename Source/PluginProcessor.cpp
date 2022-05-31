#include "PluginProcessor.h"

OverexampleAudioProcessor::OverexampleAudioProcessor()
    : parameters(*this, nullptr, "Params", {
            std::make_unique<AudioParameterFloat>("gain", "Gain", NormalisableRange<float>(0.5f, 10.0f, 0.01f, 0.5f), 1.0f)
        })
{
    parameters.addParameterListener("gain", this);
}

//==============================================================================

void OverexampleAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    auto numCh = jmax(getTotalNumOutputChannels(), getTotalNumInputChannels());
    dsp::ProcessSpec spec {sampleRate, samplesPerBlock, numCh};
    saturator.prepare(spec);
    setLatencySamples(saturator.getLatency());
}

void OverexampleAudioProcessor::releaseResources() {}

void OverexampleAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    dsp::AudioBlock<float> block(buffer);
    dsp::ProcessContextReplacing<float> context(block);

    saturator.process(context);
    
    // to add a dry/wet you can use the dsp::dryWetMixer, but remember to set the wet latency to saturator.getLatency() samples
}

void OverexampleAudioProcessor::parameterChanged(const String& parameterID, float newValue)
{
    saturator.setGain(newValue);
}

//==============================================================================

juce::AudioProcessorEditor* OverexampleAudioProcessor::createEditor()
{
    return nullptr;
}

void OverexampleAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OverexampleAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OverexampleAudioProcessor();
}
