# Mono/Stereo Audio Plugin

Professional audio plugin for instant mono/stereo conversion with pan control.

## Features
- Toggle switch between Mono/Stereo modes
- Volume control knob
- Stereo pan control (Stereo mode only)
- Clean, professional UI

## Code Structure
### PluginProcessor.h

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


###PluginProcessor.h
#pragma once

#include <JuceHeader.h>

class MonoStereoAudioProcessor : public juce::AudioProcessor
{
public:
    MonoStereoAudioProcessor();
    ~MonoStereoAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState parameters;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::atomic<float>* modeParam = nullptr;
    std::atomic<float>* volumeParam = nullptr;
    std::atomic<float>* panParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonoStereoAudioProcessor)
};

###PluginEditor.cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

MonoStereoAudioProcessorEditor::MonoStereoAudioProcessorEditor (MonoStereoAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // CARGAR BACKGROUND - ELIGE UNA OPCIÓN:
    
    // Opción 1: Si usas recursos binarios (RECOMENDADO)
    // backgroundImage = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
    
    // Opción 2: Si quieres cargar desde archivo (actualiza la ruta)
    backgroundImage = juce::ImageFileFormat::loadFrom(juce::File("/Users/david/Downloads/JUCE/MonoStereo/orejas.png"));
    
    // Configurar slider de volumen
    volumeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 70, 18);
    volumeSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff50e3c2)); // Turquesa
    volumeSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff0f0f0));
    volumeSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    volumeSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(volumeSlider);
    
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "volume", volumeSlider);
    
    volumeLabel.setText("VOLUME", juce::dontSendNotification);
    volumeLabel.setJustificationType(juce::Justification::centred);
    volumeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(volumeLabel);

    // Configurar slider de pan
    panSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    panSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 70, 18);
    panSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff4a90e2)); // Azul
    panSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff0f0f0));
    panSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    panSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(panSlider);
    
    panAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "pan", panSlider);
    
    panLabel.setText("PAN", juce::dontSendNotification);
    panLabel.setJustificationType(juce::Justification::centred);
    panLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(panLabel);

    // Obtener estado inicial del modo
    isStereoMode = audioProcessor.parameters.getRawParameterValue("mode")->load() > 0.5f;
    updatePanSliderState();

    setSize (640, 400);
}

MonoStereoAudioProcessorEditor::~MonoStereoAudioProcessorEditor()
{
}

void MonoStereoAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fondo
    g.fillAll (juce::Colour(0xff323e44));
    
    if (!backgroundImage.isNull())
    {
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
    }
    else
    {
        // Fondo de respuesto con gradiente
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff2d3740), 0, 0, 
                                             juce::Colour(0xff1a1f24), 0, (float)getHeight(), false));
        g.fillRect(getLocalBounds());
        
        // Título
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(20.0f, juce::Font::bold));
        g.drawText("MONO/STEREO", getLocalBounds().removeFromTop(50), juce::Justification::centred, true);
    }

    // Pie de página - By David Signals
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(juce::Font(12.0f, juce::Font::plain));
    g.drawText("By - David Signals", getLocalBounds().removeFromBottom(30), juce::Justification::centred, true);

    // ========== DIBUJAR EL SWITCH COMPACTO ==========
    switchArea = juce::Rectangle<int>(getWidth()/2 - 40, 80, 80, 40);
    
    // Fondo del switch
    g.setColour(juce::Colour(0xff4a5568));
    g.fillRoundedRectangle(switchArea.toFloat(), 8.0f);
    
    // Borde del switch
    g.setColour(juce::Colours::white.withAlpha(0.3f));
    g.drawRoundedRectangle(switchArea.toFloat(), 8.0f, 2.0f);
    
    // Botón del switch con colores invertidos
    if (isStereoMode)
    {
        // STEREO - AZUL (derecha)
        g.setColour(juce::Colour(0xff4a90e2));
        g.fillRoundedRectangle(switchArea.getX() + 40, switchArea.getY() + 5, 35, 30, 6.0f);
    }
    else
    {
        // MONO - TURQUESA (izquierda)
        g.setColour(juce::Colour(0xff50e3c2));
        g.fillRoundedRectangle(switchArea.getX() + 5, switchArea.getY() + 5, 35, 30, 6.0f);
    }
    
    // Indicador de estado actual (arriba del switch)
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    if (isStereoMode)
        g.drawText("STEREO MODE", getWidth()/2 - 60, 50, 120, 25, juce::Justification::centred);
    else
        g.drawText("MONO MODE", getWidth()/2 - 60, 50, 120, 25, juce::Justification::centred);
}

void MonoStereoAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    // Slider de volumen
    volumeSlider.setBounds(area.getCentreX() - 50, 150, 100, 110);
    
    // Slider de pan
    panSlider.setBounds(area.getCentreX() - 50, 280, 100, 110);
    
    // Labels
    volumeLabel.setBounds(area.getCentreX() - 100, 130, 200, 15);
    panLabel.setBounds(area.getCentreX() - 100, 260, 200, 15);
}

void MonoStereoAudioProcessorEditor::mouseDown (const juce::MouseEvent& event)
{
    // Verificar si el clic fue en el área del switch
    if (switchArea.contains(event.getPosition()))
    {
        toggleMode();
    }
}

void MonoStereoAudioProcessorEditor::toggleMode()
{
    isStereoMode = !isStereoMode;
    
    // Actualizar parámetro
    *audioProcessor.parameters.getRawParameterValue("mode") = isStereoMode ? 1.0f : 0.0f;
    
    updatePanSliderState();
    repaint();
}

void MonoStereoAudioProcessorEditor::updatePanSliderState()
{
    panSlider.setEnabled(isStereoMode);
    panSlider.setAlpha(isStereoMode ? 1.0f : 0.4f);
    panLabel.setAlpha(isStereoMode ? 1.0f : 0.4f);
}

###PluginEditor.h
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MonoStereoAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    MonoStereoAudioProcessorEditor (MonoStereoAudioProcessor&);
    ~MonoStereoAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent& event) override;

private:
    MonoStereoAudioProcessor& audioProcessor;

    juce::Image backgroundImage;
    
    juce::Slider volumeSlider;
    juce::Slider panSlider;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;
    
    juce::Label volumeLabel;
    juce::Label panLabel;

    bool isStereoMode = false;
    juce::Rectangle<int> switchArea;

    void toggleMode();
    void updatePanSliderState();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MonoStereoAudioProcessorEditor)
};
