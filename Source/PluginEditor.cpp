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