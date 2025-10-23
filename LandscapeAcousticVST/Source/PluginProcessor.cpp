#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <atomic>
#include <memory>

//==============================================================================
LandscapeAcousticVSTProcessor::LandscapeAcousticVSTProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
       parameters(*this, nullptr, "Parameters", createParameterLayout()),
       needsIRUpdate(false),
       isProcessingIR(false),
       pointsAreSet(false),
       sourcePoint(0.0, 0.0),
       receiverPoint(0.0, 0.0)
{
    // Initialize core components
    terrainLoader = std::make_unique<TerrainLoader>();
    acousticEngine = std::make_unique<AcousticEngine>();
    impulseGenerator = std::make_unique<ImpulseGenerator>();
    
    // Add parameter listeners
    parameters.addParameterListener("dry_wet", this);
    parameters.addParameterListener("output_gain", this);
    parameters.addParameterListener("temperature", this);
    parameters.addParameterListener("humidity", this);
    parameters.addParameterListener("ground_type", this);
    parameters.addParameterListener("source_height", this);
    parameters.addParameterListener("receiver_height", this);
}

LandscapeAcousticVSTProcessor::~LandscapeAcousticVSTProcessor()
{
    // Remove parameter listeners to avoid callbacks during destruction
    parameters.removeParameterListener("dry_wet", this);
    parameters.removeParameterListener("output_gain", this);
    parameters.removeParameterListener("temperature", this);
    parameters.removeParameterListener("humidity", this);
    parameters.removeParameterListener("ground_type", this);
    parameters.removeParameterListener("source_height", this);
    parameters.removeParameterListener("receiver_height", this);
}

//==============================================================================
const juce::String LandscapeAcousticVSTProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LandscapeAcousticVSTProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool LandscapeAcousticVSTProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool LandscapeAcousticVSTProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double LandscapeAcousticVSTProcessor::getTailLengthSeconds() const
{
    return 2.0; // 2 second tail for impulse response
}

int LandscapeAcousticVSTProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int LandscapeAcousticVSTProcessor::getCurrentProgram()
{
    return 0;
}

void LandscapeAcousticVSTProcessor::setCurrentProgram (int index)
{
}

const juce::String LandscapeAcousticVSTProcessor::getProgramName (int index)
{
    return {};
}

void LandscapeAcousticVSTProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void LandscapeAcousticVSTProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Configure DSP spec for convolution
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
    
    // Prepare convolution engine with non-uniform partitioning for efficiency
    convolution.prepare(spec);
    convolution.reset();
    
    // Load default IR if available
    if (impulseGenerator->hasImpulseResponse()) {
        updateImpulseResponse();
    }
}

void LandscapeAcousticVSTProcessor::releaseResources()
{
    convolution.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool LandscapeAcousticVSTProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void LandscapeAcousticVSTProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // Clear any unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // Check if we need to update IR (atomic check)
    if (needsIRUpdate.exchange(false)) {
        updateImpulseResponse();
    }
    
    // Skip processing if we're currently generating IR
    if (isProcessingIR.load()) {
        return;
    }
    
    // Get parameter values (with null checks)
    auto dryWetParam = parameters.getRawParameterValue("dry_wet");
    auto outputGainParam = parameters.getRawParameterValue("output_gain");
    
    if (dryWetParam == nullptr || outputGainParam == nullptr) {
        // Parameters not ready, pass through
        return;
    }
    
    float dryWet = *dryWetParam;
    float outputGain = juce::Decibels::decibelsToGain(*outputGainParam);
    
    // Store dry signal for mixing
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples());
    dryBuffer.makeCopyOf(buffer);
    
    // Apply convolution to wet signal
    if (impulseGenerator->hasImpulseResponse()) {
        try {
            juce::dsp::AudioBlock<float> block(buffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            convolution.process(context);
            
            // Mix dry/wet signals
            for (int channel = 0; channel < totalNumOutputChannels; ++channel) {
                auto* wetData = buffer.getWritePointer(channel);
                auto* dryData = dryBuffer.getReadPointer(channel);
                
                for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                    wetData[sample] = dryData[sample] * (1.0f - dryWet) + 
                                      wetData[sample] * dryWet;
                    wetData[sample] *= outputGain;
                }
            }
        } catch (...) {
            // If convolution fails, fall back to dry signal
            buffer.makeCopyOf(dryBuffer);
            buffer.applyGain(outputGain);
        }
    } else {
        // No IR loaded, pass through dry signal with gain
        buffer.applyGain(outputGain);
    }
}

//==============================================================================
bool LandscapeAcousticVSTProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* LandscapeAcousticVSTProcessor::createEditor()
{
    return new LandscapeAcousticVSTEditor (*this);
}

//==============================================================================
void LandscapeAcousticVSTProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    
    // Add custom state data
    auto customState = state.getOrCreateChildWithName("CustomData", nullptr);
    
    if (terrainLoader->isLoaded()) {
        customState.setProperty("terrainFile", 
                               terrainLoader->getDEMData().filename, nullptr);
    }
    
    customState.setProperty("sourceX", sourcePoint.x, nullptr);
    customState.setProperty("sourceY", sourcePoint.y, nullptr);
    customState.setProperty("receiverX", receiverPoint.x, nullptr);
    customState.setProperty("receiverY", receiverPoint.y, nullptr);
    customState.setProperty("pointsSet", pointsAreSet, nullptr);
    
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void LandscapeAcousticVSTProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName(parameters.state.getType())) {
            auto newState = juce::ValueTree::fromXml(*xmlState);
            parameters.replaceState(newState);
            
            // Restore custom state
            auto customState = newState.getChildWithName("CustomData");
            if (customState.isValid()) {
                sourcePoint.x = customState.getProperty("sourceX", 0.0);
                sourcePoint.y = customState.getProperty("sourceY", 0.0);
                receiverPoint.x = customState.getProperty("receiverX", 0.0);
                receiverPoint.y = customState.getProperty("receiverY", 0.0);
                pointsAreSet = customState.getProperty("pointsSet", false);
                
                juce::String terrainFile = customState.getProperty("terrainFile", "");
                if (terrainFile.isNotEmpty()) {
                    juce::File file(terrainFile);
                    if (file.existsAsFile()) {
                        juce::String error;
                        loadTerrainFile(file, error);
                    }
                }
            }
        }
    }
}

//==============================================================================
// Plugin-specific methods

juce::AudioProcessorValueTreeState::ParameterLayout 
LandscapeAcousticVSTProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    // Dry/Wet Mix
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "dry_wet", "Dry/Wet Mix", 
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f,
        "Blend between original (dry) and processed (wet) signal"));
    
    // Output Gain
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "output_gain", "Output Gain (dB)", 
        juce::NormalisableRange<float>(-20.0f, 20.0f, 0.1f), 0.0f,
        "Output level adjustment"));
    
    // Atmospheric Parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "temperature", "Temperature (°C)", 
        juce::NormalisableRange<float>(-10.0f, 40.0f, 0.1f), 15.0f,
        "Air temperature for atmospheric absorption"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "humidity", "Humidity (%)", 
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 70.0f,
        "Relative humidity for atmospheric absorption"));
    
    // Ground Type
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "ground_type", "Ground Type",
        juce::StringArray{"Hard", "Porous", "Mixed"}, 1,
        "Surface type for ground reflection modeling"));
    
    // Source/Receiver Heights
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "source_height", "Source Height (m)",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.1f), 2.0f,
        "Height of sound source above terrain"));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "receiver_height", "Receiver Height (m)",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.1f), 2.0f,
        "Height of listener above terrain"));
    
    return { params.begin(), params.end() };
}

void LandscapeAcousticVSTProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Mark that IR needs updating when relevant parameters change
    if (parameterID == "temperature" || parameterID == "humidity" || 
        parameterID == "ground_type" || parameterID == "source_height" ||
        parameterID == "receiver_height") {
        
        if (terrainLoader->isLoaded() && pointsAreSet) {
            needsIRUpdate.store(true);
        }
    }
}

bool LandscapeAcousticVSTProcessor::loadTerrainFile(const juce::File& demFile, juce::String& errorMsg)
{
    bool success = terrainLoader->loadDEM(demFile, errorMsg);
    
    if (success && pointsAreSet) {
        needsIRUpdate.store(true);
    }
    
    return success;
}

void LandscapeAcousticVSTProcessor::setAnalysisPoints(juce::Point<double> source, juce::Point<double> receiver)
{
    sourcePoint = source;
    receiverPoint = receiver;
    pointsAreSet = true;
    
    if (terrainLoader->isLoaded()) {
        needsIRUpdate.store(true);
    }
}

void LandscapeAcousticVSTProcessor::updateImpulseResponse()
{
    if (!terrainLoader->isLoaded() || !pointsAreSet) {
        return;
    }
    
    isProcessingIR.store(true);
    
    // Get current parameters with null checks
    auto tempParam = parameters.getRawParameterValue("temperature");
    auto humidityParam = parameters.getRawParameterValue("humidity");
    auto groundTypeParam = parameters.getRawParameterValue("ground_type");
    auto sourceHeightParam = parameters.getRawParameterValue("source_height");
    auto receiverHeightParam = parameters.getRawParameterValue("receiver_height");
    
    if (!tempParam || !humidityParam || !groundTypeParam || !sourceHeightParam || !receiverHeightParam) {
        isProcessingIR.store(false);
        return;
    }
    
    // Get current parameters
    AtmosphericConditions atmo;
    atmo.temperature_C = *tempParam;
    atmo.relativeHumidity = *humidityParam;
    
    auto groundType = static_cast<GroundType>(
        static_cast<int>(*groundTypeParam));
    
    double sourceHeight = *sourceHeightParam;
    double receiverHeight = *receiverHeightParam;
    
    // Sample terrain profile
    TerrainProfile profile = terrainLoader->sampleProfile(
        sourcePoint.x, sourcePoint.y,
        receiverPoint.x, receiverPoint.y,
        500);
    
    if (profile.isValid()) {
        // Get current sample rate
        double currentSampleRate = getSampleRate();
        if (currentSampleRate <= 0.0) {
            isProcessingIR.store(false);
            return;
        }
        
        // Generate impulse response
        auto ir = impulseGenerator->generateImpulseResponse(
            profile, sourceHeight, receiverHeight, atmo, groundType, currentSampleRate);
        
        // Load into convolution engine (this is thread-safe)
        if (ir.getNumChannels() > 0 && ir.getNumSamples() > 0) {
            convolution.loadImpulseResponse(
                std::move(ir),
                currentSampleRate,
                juce::dsp::Convolution::Stereo::yes,
                juce::dsp::Convolution::Trim::yes,
                0,
                juce::dsp::Convolution::Normalise::yes);
        }
    }
    
    isProcessingIR.store(false);
}

TerrainProfile LandscapeAcousticVSTProcessor::getCurrentProfile() const
{
    if (!terrainLoader->isLoaded() || !pointsAreSet) {
        return TerrainProfile{};
    }
    
    return terrainLoader->sampleProfile(
        sourcePoint.x, sourcePoint.y,
        receiverPoint.x, receiverPoint.y,
        500);
}

bool LandscapeAcousticVSTProcessor::importQGISConfig(const juce::File& configFile, juce::String& errorMsg)
{
    // Load and parse JSON config file from QGIS export
    juce::FileInputStream stream(configFile);
    if (!stream.openedOk()) {
        errorMsg = "Cannot open config file";
        return false;
    }
    
    juce::String jsonText = stream.readEntireStreamAsString();
    juce::var json = juce::JSON::parse(jsonText);
    
    if (!json.isObject()) {
        errorMsg = "Invalid JSON format";
        return false;
    }
    
    auto* obj = json.getDynamicObject();
    
    // Extract DEM file path
    juce::String demPath = obj->getProperty("dem_file").toString();
    if (demPath.isEmpty()) {
        errorMsg = "No DEM file specified in config";
        return false;
    }
    
    // Load DEM
    juce::File demFile(demPath);
    if (!loadTerrainFile(demFile, errorMsg)) {
        return false;
    }
    
    // Extract points
    auto sourceObj = obj->getProperty("source");
    auto receiverObj = obj->getProperty("receiver");
    
    if (!sourceObj.isObject() || !receiverObj.isObject()) {
        errorMsg = "Invalid source/receiver points in config";
        return false;
    }
    
    auto* srcObj = sourceObj.getDynamicObject();
    auto* rcvObj = receiverObj.getDynamicObject();
    
    juce::Point<double> source(srcObj->getProperty("lon"), srcObj->getProperty("lat"));
    juce::Point<double> receiver(rcvObj->getProperty("lon"), rcvObj->getProperty("lat"));
    
    setAnalysisPoints(source, receiver);
    
    return true;
}

bool LandscapeAcousticVSTProcessor::exportImpulseResponse(const juce::File& outputFile)
{
    return impulseGenerator->exportIR(outputFile);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LandscapeAcousticVSTProcessor();
}