#include "PluginProcessor.h"
#include "PluginEditor.h"

MonoStereoAudioProcessor::MonoStereoAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    modeParam = parameters.getRawParameterValue("mode");
    volumeParam = parameters.getRawParameterValue("volume");
    panParam = parameters.getRawParameterValue("pan");
}

MonoStereoAudioProcessor::~MonoStereoAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout MonoStereoAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // Cambiamos a AudioParameterFloat para el slider de 2 posiciones
    params.push_back(std::make_unique<juce::AudioParameterFloat>("mode", "Mode", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("volume", "Volume", 0.0f, 1.0f, 0.7f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("pan", "Pan", -1.0f, 1.0f, 0.0f));
    
    return { params.begin(), params.end() };
}

void MonoStereoAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void MonoStereoAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MonoStereoAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    #if JucePlugin_IsMidiEffect
        ignoreUnused (layouts);
        return true;
    #else
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
         && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
            return false;

        #if ! JucePlugin_IsSynth
        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
            return false;
        #endif

        return true;
    #endif
}
#endif

void MonoStereoAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    float volume = *volumeParam;
    float pan = *panParam;
    bool isStereo = *modeParam > 0.5f;  // Ahora comparamos con 0.5f otra vez

    // Apply volume
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
            channelData[sample] *= volume;
    }

    // Handle mono/stereo processing
    if (totalNumInputChannels == 2 && totalNumOutputChannels == 2)
    {
        if (!isStereo) // Mono mode - mix to mono
        {
            auto* leftChannel = buffer.getWritePointer(0);
            auto* rightChannel = buffer.getWritePointer(1);
            
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float mono = (leftChannel[sample] + rightChannel[sample]) * 0.5f;
                leftChannel[sample] = mono;
                rightChannel[sample] = mono;
            }
        }
        else // Stereo mode - apply panning
        {
            auto* leftChannel = buffer.getWritePointer(0);
            auto* rightChannel = buffer.getWritePointer(1);
            
            float leftGain = 1.0f - std::max(0.0f, pan);
            float rightGain = 1.0f - std::max(0.0f, -pan);
            
            for (int sample = 0; sample < numSamples; ++sample)
            {
                leftChannel[sample] *= leftGain;
                rightChannel[sample] *= rightGain;
            }
        }
    }
}

juce::AudioProcessorEditor* MonoStereoAudioProcessor::createEditor()
{
    return new MonoStereoAudioProcessorEditor (*this);
}

bool MonoStereoAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String MonoStereoAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MonoStereoAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MonoStereoAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MonoStereoAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MonoStereoAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MonoStereoAudioProcessor::getNumPrograms()
{
    return 1;
}

int MonoStereoAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MonoStereoAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MonoStereoAudioProcessor::getProgramName (int index)
{
    return {};
}

void MonoStereoAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void MonoStereoAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MonoStereoAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MonoStereoAudioProcessor();
}