/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "./PeakMeter.hpp"
#include "./RMSMeter.hpp"
#include "./FFTMeter.hpp"
#include <string>
#include <memory>

typedef double dB_t;
typedef int millisec;

struct IPeakMeterController
{
public:
    virtual ~IPeakMeterController() {}

    virtual void PeakMeter_SetPeakHoldTime(millisec t) = 0;
    virtual void PeakMeter_SetAverageNum(int n) = 0;
};

struct IRMSMeterController
{
public:
    virtual ~IRMSMeterController() {}
    
    virtual void RMSMeter_SetIntegrationTime(millisec t) = 0;
};

struct IFFTMeterController
{
public:
    virtual ~IFFTMeterController() {}
    
    virtual void FFTMeter_SetFFTOrder(int n) = 0;
    virtual void FFTMeter_SetPeakHoldTime(millisec t) = 0;
    virtual void FFTMeter_SetAverageNum(int n) = 0;
};

class SliderWithTitle
:   public juce::Component
,   juce::Slider::Listener
{
public:
    SliderWithTitle(juce::String name, int value_min, int value_max, int default_value)
    {
        addAndMakeVisible(slider_);
        addAndMakeVisible(title_);
        
        slider_.setRange(value_min, value_max, 1);
        slider_.setValue(default_value, juce::NotificationType::dontSendNotification);
        slider_.addListener(this);
        
        title_.setText(name, juce::NotificationType::dontSendNotification);
        title_.setJustificationType(juce::Justification::left);
        
        title_.setColour(juce::Label::textColourId, juce::Colours::orange);
    }

public:
    void SetCallback(std::function<void(int value)> callback)
    {
        callback_ = callback;
    }
    
private:
    void resized() override
    {
        title_.setBounds(getLocalBounds().removeFromTop(getHeight() / 2).reduced(2));
        slider_.setBounds(getLocalBounds().removeFromBottom(getHeight() / 2).reduced(2));
    }
    
    void sliderValueChanged(Slider *) override
    {
        if(callback_) {
            callback_(slider_.getValue());
        }
    }
    
private:
    juce::Label title_;
    juce::Slider slider_;
    int title_width_;
    std::function<void(int)> callback_;
};

//! ピークメーターの表示用クラス
class PeakMeterComponent
:   public juce::Component
{
    int const GetPeakMeterWidth()
    {
        return getWidth() / 6;
    }

    int const kPeakMeterPadding = 2;
    
public:
    PeakMeterComponent(IPeakMeterController *controller)
    :   current_level_(std::numeric_limits<dB_t>::lowest())
    ,   peak_hold_level_(std::numeric_limits<dB_t>::lowest())
    ,   controller_(controller)
    {
        peak_hold_time_ = std::make_unique<SliderWithTitle>("Peak Hold Time(ms)", 0, 2000, 1000);
        average_num_ = std::make_unique<SliderWithTitle>("Average Num", 1, 500, 1);
        
        peak_hold_time_->SetCallback([this](int value) { controller_->PeakMeter_SetPeakHoldTime(value); });
        average_num_->SetCallback([this](int value) { controller_->PeakMeter_SetAverageNum(value); });
        
        addAndMakeVisible(*peak_hold_time_);
        addAndMakeVisible(*average_num_);
    }
    
private:
    void paint(juce::Graphics &g) override
    {
        int const w = getWidth();
        int const h = getHeight();
        
        g.setColour(juce::Colours::orange);
        g.drawRect(0, 0, w, h);
        
        float meter_height = h - kPeakMeterPadding * 2;
        float meter_width = GetPeakMeterWidth();
        float meter_y = kPeakMeterPadding;
        float meter_x = w - (meter_width + kPeakMeterPadding * 2);
        
        g.drawText("Peak Meter", getLocalBounds().reduced(2), juce::Justification::topLeft);
        
        //! -96.0 .. 0 のdBを 0 .. 1.0にscaleする
        //! -96dB以下の信号は無音とみなす。
        static auto const scale = [](float x) { return std::max<double>(-96, x) / 96.0 + 1.0; };
        
        float current_level = scale(current_level_);
        float peak_hold_level = scale(peak_hold_level_);
        
        float current_level_height = meter_height * current_level;
        float peak_hold_height = meter_height * peak_hold_level;
        
        juce::ColourGradient grad(juce::Colours::orange,
                                  meter_x + meter_width / 2,
                                  meter_y,
                                  juce::Colours::lightblue,
                                  meter_x + meter_width / 2,
                                  meter_height,
                                  false
                                  );
        g.setGradientFill(grad);
        
        g.fillRect(meter_x, meter_y + meter_height - current_level_height, meter_width, current_level_height);
        
        g.setColour(juce::Colours::red);
        g.drawRect(meter_x, meter_y + meter_height - peak_hold_height, meter_width, 2.0f);
    }
    
    void resized() override
    {
        int w = getWidth();
        auto rect = getLocalBounds().removeFromLeft(w - (GetPeakMeterWidth() + kPeakMeterPadding * 2));
        rect.reduced(2);

        rect.removeFromTop(50);
        peak_hold_time_->setBounds(rect.removeFromTop(50));
        average_num_->setBounds(rect.removeFromTop(50));
    }
    
public:
    void SetCurrentPeakLevel(dB_t level)
    {
        current_level_ = level;
    }

    void SetPeakHoldLevel(dB_t level)
    {
        peak_hold_level_ = level;
    }
    
private:
    dB_t current_level_;
    dB_t peak_hold_level_;
    std::unique_ptr<SliderWithTitle> peak_hold_time_;
    std::unique_ptr<SliderWithTitle> average_num_;
    IPeakMeterController *controller_;
};

//! VUメーターの表示用クラス
class RMSMeterComponent
:   public juce::Component
{
    int const GetRMSMeterWidth() {
        return getWidth() * 0.67;
    }

    int const kRMSMeterPadding = 2;
    
public:
    RMSMeterComponent(IRMSMeterController *controller)
    :   rms_level_(0)
    ,   controller_(controller)
    {
        integration_time_ = std::make_unique<SliderWithTitle>("Integration Time(ms)", 1, 2000, 1000);
        integration_time_->SetCallback([this](int value) { controller_->RMSMeter_SetIntegrationTime(value); });
        
        adjust_input_level_ = std::make_unique<SliderWithTitle>("Adjust Input Level(dB * 10^-3)", -12000, 12000, 0);
        adjust_input_level_->SetCallback([this](int value) { input_level_adjustment_ = value; });
        
        addAndMakeVisible(*integration_time_);
        addAndMakeVisible(*adjust_input_level_);
    }
    
private:
    //! 入力された信号のリニアなレベルを、
    //! -3dB時に90°となるような角度に変換する。
    //! @param level dBスケールではないサンプル値
    //! @return 角度をradianに変換したもの
    double get_linear_to_radian(double level)
    {
        auto scale = [](double level)
        {
            //! 角度は、入力された信号のリニアなレベル(dBではない)に線形に反応する。
            static double const a = (90 - 138) / (dB_to_linear(-3) - dB_to_linear(-20));
            static double const b = 138 - a * dB_to_linear(-20);
            return a * level + b;
        };
        
        static double const angle_max = scale(dB_to_linear(-22));
        static double const angle_min = scale(dB_to_linear(3.1));
        
        double angle = scale(level);
        if(angle < angle_min) {
            angle = angle_min;
        } else if(angle > angle_max) {
            angle = angle_max;
        }

        return angle / 180 * M_PI;
    }
    
    void paint(juce::Graphics &g) override
    {
        struct VUIndicator
        {
            double position_;
            bool print_number_;
        };
        
        static VUIndicator const vu_indicator_list[] = {
            { -22.0, false },
            { -20.0, true },
            { -10.0, true },
            { -7.0, true },
            { -5.0, true },
            { -4.0, false },
            { -3.0, true, },
            { -2.0, false },
            { -1.0, true },
            { -0.5, false },
            { 0.0, true },
            { 0.5, false },
            { 1.0, true },
            { 2.0, false },
            { 3.0, true },
            { 3.1, false }
        };
        
        auto const w = getWidth();
        auto const h = getHeight();
        
        g.setColour(juce::Colours::orange);
        g.drawRect(0, 0, w, h);
        g.drawText("VU Meter", getLocalBounds().reduced(2), juce::Justification::topLeft);
        
        auto needle_base_x = w - (GetRMSMeterWidth() / 2 + kRMSMeterPadding);
        auto needle_base_y = h * 6 / 7;
        
        double R = GetRMSMeterWidth() / 2;
        
        for(auto vu: vu_indicator_list) {
            
            if(vu.position_ < 0) {
                g.setColour(juce::Colours::green);
            } else {
                g.setColour(juce::Colour(0xFFFF0000));
            }
            
            double theta = get_linear_to_radian(dB_to_linear(vu.position_));
            
            double grid_length_ratio;
            if(vu.print_number_) {
                grid_length_ratio = 0.17;
            } else {
                grid_length_ratio = 0.1;
            }
            
            double x1 = R * cos(theta);
            double y1 = -R * sin(theta);
            double x2 = (R + R * grid_length_ratio) * cos(theta);
            double y2 = -(R + R * grid_length_ratio) * sin(theta);
            
            g.drawLine(x1 + needle_base_x, y1 + needle_base_y, x2 + needle_base_x, y2 + needle_base_y, 2.0);
            
            if(vu.print_number_) {
                std::string name = std::to_string((int)vu.position_);
                g.drawText(name, x2 + needle_base_x, y2 + needle_base_y - 14, w, h, juce::Justification::topLeft);
            }
        }
        
        g.setColour(juce::Colour(0x9500ffff));
        
        dB_t adjusted_dB = linear_to_dB(rms_level_) + (input_level_adjustment_ * 0.001);
        double theta = get_linear_to_radian(dB_to_linear(adjusted_dB));
        double x1 = 0;
        double y1 = 0;
        double x2 = (R * 1.4) * cos(theta);
        double y2 = -(R * 1.4) * sin(theta);
        
        g.drawLine(x1 + needle_base_x, y1 + needle_base_y, x2 + needle_base_x, y2 + needle_base_y, 2.0);
    }
    
    void resized() override
    {
        int w = getWidth();
        auto rect = getLocalBounds().removeFromLeft(w - (GetRMSMeterWidth() + kRMSMeterPadding * 2));
        rect.reduced(2);
        
        rect.removeFromTop(50);
        integration_time_->setBounds(rect.removeFromTop(50));
        adjust_input_level_->setBounds(rect.removeFromTop(50));
    }
    
public:
    void SetCurrentRMSLevel(double level)
    {
        rms_level_ = level;
    }
    
private:
    double rms_level_;
    std::unique_ptr<SliderWithTitle> integration_time_;
    std::unique_ptr<SliderWithTitle> adjust_input_level_;
    IRMSMeterController *controller_;
    int input_level_adjustment_ = 0;
};

//! FFTメーターの表示用クラス
class FFTMeterComponent
:   public juce::Component
{
    int const GetFFTMeterWidth() const
    {
        return getWidth() * 0.75;
    }
    int const kFFTMeterPadding = 2;
    
public:
    FFTMeterComponent(IFFTMeterController *controller)
    :   controller_(controller)
    {
        fft_order_ = std::make_unique<SliderWithTitle>("FFT Order(2^n)", 6, 15, 8);
        peak_hold_time_ = std::make_unique<SliderWithTitle>("Peak Hold Time(ms)", 0, 2000, 1000);
        average_num_ = std::make_unique<SliderWithTitle>("Average Num", 1, 50, 12);
        
        fft_order_->SetCallback([this](int value) { controller_->FFTMeter_SetFFTOrder(value); });
        peak_hold_time_->SetCallback([this](int value) { controller_->FFTMeter_SetPeakHoldTime(value); });
        average_num_->SetCallback([this](int value) { controller_->FFTMeter_SetAverageNum(value); });
        
        addAndMakeVisible(*fft_order_);
        addAndMakeVisible(*peak_hold_time_);
        addAndMakeVisible(*average_num_);
    }
    
private:
    void paint(juce::Graphics &g) override
    {
        int const w = getWidth();
        int const h = getHeight();
        
        g.setColour(juce::Colours::orange);
        g.drawRect(0, 0, w, h);
        g.drawText("FFT Meter", getLocalBounds().reduced(2), juce::Justification::topLeft);
        
        float meter_height = h - kFFTMeterPadding * 2;
        float meter_width = GetFFTMeterWidth();
        float meter_y = kFFTMeterPadding;
        float meter_x = w - (meter_width + kFFTMeterPadding * 2);
        
        bool use_log_scale_in_freq = true;
        
        int const num_elements = peak_values_.size();
        for(int i = 0; i < num_elements; ++i) {
            static auto const scale = [](float x) { return std::max<float>(-144.0, x) / 144.0 + 1.0; };
            
            float current_level = scale(current_power_values_[i]);
            float peak_level = scale(peak_values_[i]);
            
            float current_level_height = meter_height * current_level;
            float peak_level_height = meter_height * peak_level;
            
            float x = 0;
            float w = 0;
            
            if(use_log_scale_in_freq) {
                double const x_left = log10((i / (double)num_elements * (10-1) + 1)) * meter_width;
                double const x_right = log10(((i+1) / (double)num_elements * (10-1) + 1)) * meter_width;
                
                x = meter_x + x_left;
                w = x_right - x_left;
            } else {
                x = meter_x + (i / (double)(num_elements) * meter_width);;
                w = 2.0f;
            }
            
            g.setColour(juce::Colours::orange);
            g.fillRect(x, meter_y + meter_height - current_level_height, w, current_level_height);
            
            g.setColour(juce::Colours::red);
            g.drawRect(x, meter_y + meter_height - peak_level_height, w, 2.f);
        }
        
    }
    
    void resized() override
    {
        int w = getWidth();
        auto rect = getLocalBounds().removeFromLeft(w - (GetFFTMeterWidth() + kFFTMeterPadding * 2));
        
        rect.removeFromTop(50);
        fft_order_->setBounds(rect.removeFromTop(50));
        peak_hold_time_->setBounds(rect.removeFromTop(50));
        average_num_->setBounds(rect.removeFromTop(50));
    }

public:
    void FFTOrderChanged(int fft_size)
    {
        peak_values_.resize(fft_size / 2);
        current_power_values_.resize(fft_size / 2);
    }
    
    void SetPeakLevel(dB_t level, int freq_bin_index)
    {
        peak_values_[freq_bin_index] = level;
    }
    
    void SetCurrentPowerLevel(dB_t level, int freq_bin_index)
    {
        current_power_values_[freq_bin_index] = level;
    }
    
private:
    std::unique_ptr<SliderWithTitle> fft_order_;
    std::unique_ptr<SliderWithTitle> peak_hold_time_;
    std::unique_ptr<SliderWithTitle> average_num_;
    std::vector<dB_t> peak_values_;
    std::vector<dB_t> current_power_values_;
    IFFTMeterController *controller_;
};

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent
:   public AudioAppComponent
,   juce::Timer
,   public IPeakMeterController
,   public IRMSMeterController
,   public IFFTMeterController
,   public juce::ButtonListener
{
public:
    //==============================================================================
    MainContentComponent()
    :   file_loaded_(false)
    {
        // specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);

        peak_meter_ = std::make_unique<PeakMeter>(44100, 1, 1000);
        peak_meter_->SetPeakHoldTime(1000);
        peak_meter_->SetReleaseSpeed(-96 / 2);
        
        rms_meter_ = std::make_unique<RMSMeter>(44100, 300);
        
        fft_meter_ = std::make_unique<FFTMeter>(44100, 9, 12, 300, -12);
        
        peak_meter_component_ = std::make_unique<PeakMeterComponent>(this);
        addAndMakeVisible(*peak_meter_component_);

        rms_meter_component_ = std::make_unique<RMSMeterComponent>(this);
        addAndMakeVisible(*rms_meter_component_);
        
        fft_meter_component_ = std::make_unique<FFTMeterComponent>(this);
        addAndMakeVisible(*fft_meter_component_);
        fft_meter_component_->FFTOrderChanged(fft_meter_->GetSize());

        addAndMakeVisible(file_loading_button_);
        addAndMakeVisible(file_path_display_);
        
        file_loading_button_.addListener(this);
        
        file_path_display_.setColour(juce::Label::outlineColourId, juce::Colours::white);
        file_path_display_.setColour(juce::Label::textColourId, juce::Colours::white);
        auto &laf = file_path_display_.getLookAndFeel();
        laf.setDefaultSansSerifTypefaceName("Hiragino Kaku Gothic Pro");
        file_path_display_.sendLookAndFeelChange();
        
        file_loading_button_.setButtonText("Load File");
        
        setSize (800, 600);
        startTimer(16);
    }
    
    void timerCallback() override
    {
        repaint();
    }
    
    void buttonClicked(juce::Button *btn) override
    {
        if(btn == &file_loading_button_) {
            LoadFile();
        }
    }
    
    void LoadFile()
    {
        juce::WildcardFileFilter wildcardFilter ("*.wav;*.aiff;*.mp3;*.mp4;*.m4a", String::empty, "Audio Files");
        
        auto const flags =
        juce::FileBrowserComponent::FileChooserFlags::canSelectFiles |
        juce::FileBrowserComponent::FileChooserFlags::openMode;
        
        
        juce::FileBrowserComponent browser (flags,
                                            juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDesktopDirectory),
                                            &wildcardFilter,
                                            nullptr);
        juce::FileChooserDialogBox dialogBox ("Open some kind of file",
                                              "Please choose some kind of file that you want to open...",
                                              browser,
                                              false,
                                              Colours::lightgrey);
        
        auto &laf_browser = browser.getLookAndFeel();
        laf_browser.setDefaultSansSerifTypefaceName("Hiragino Kaku Gothic Pro");
        browser.sendLookAndFeelChange();

        if (!dialogBox.show()) {
            return;
        }
        
        File selected_file = browser.getSelectedFile (0);
        
        juce::AudioFormatManager mgr;
        mgr.registerBasicFormats();
        auto reader = mgr.createReaderFor(selected_file);
        if(!reader) { return; }
        
        {
            std::unique_lock<std::mutex> lock(process_mutex_);
            file_loaded_ = false;
        }
        
        {
            std::unique_lock<std::mutex> lock(process_mutex_);
            reader_source_ = new juce::AudioFormatReaderSource(reader, true);
            reader_source_->setLooping(true);
            file_loaded_ = true;
        }
        file_path_display_.setText(selected_file.getFullPathName(), juce::NotificationType::dontSendNotification);
    }
    
    void PeakMeter_SetPeakHoldTime(millisec t) override
    {
        std::unique_lock<std::mutex> lock(process_mutex_);
        peak_meter_->SetPeakHoldTime(t);
    }
    
    void PeakMeter_SetAverageNum(int n) override
    {
        std::unique_lock<std::mutex> lock(process_mutex_);
        peak_meter_->SetMovingAverage(n);
    }
    
    void RMSMeter_SetIntegrationTime(millisec t) override
    {
        std::unique_lock<std::mutex> lock(process_mutex_);
        rms_meter_->SetIntegrationTime(t);
    }
    
    void FFTMeter_SetFFTOrder(int value) override
    {
        std::unique_lock<std::mutex> lock(process_mutex_);
        fft_meter_->SetFFTOrder(value);
        fft_meter_component_->FFTOrderChanged(fft_meter_->GetSize());
    }
    
    void FFTMeter_SetPeakHoldTime(millisec t) override
    {
        std::unique_lock<std::mutex> lock(process_mutex_);
        fft_meter_->SetPeakHoldTime(t);
    }
    
    void FFTMeter_SetAverageNum(int value) override
    {
        std::unique_lock<std::mutex> lock(process_mutex_);
        fft_meter_->SetMovingAverage(value);
    }
    
    juce::ScopedPointer<juce::AudioFormatReaderSource> reader_source_;
    std::atomic<bool> file_loaded_;
    std::unique_ptr<PeakMeter> peak_meter_;
    std::unique_ptr<RMSMeter> rms_meter_;
    std::unique_ptr<FFTMeter> fft_meter_;

    std::mutex process_mutex_;

    std::unique_ptr<PeakMeterComponent> peak_meter_component_;
    std::unique_ptr<RMSMeterComponent> rms_meter_component_;
    std::unique_ptr<FFTMeterComponent> fft_meter_component_;
    
    juce::TextButton file_loading_button_;
    juce::Label file_path_display_;

    ~MainContentComponent()
    {
        shutdownAudio();
    }

    //=======================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        // This function will be called when the audio device is started, or when
        // its settings (i.e. sample rate, block size, etc) are changed.

        // You can use this function to initialise any resources you might need,
        // but be careful - it will be called on the audio thread, not the GUI thread.

        // For more details, see the help for AudioProcessor::prepareToPlay()
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        // Your audio-processing code goes here!

        // For more details, see the help for AudioProcessor::getNextAudioBlock()

        // Right now we are not producing any data, in which case we need to clear the buffer
        // (to prevent the output of random noise)
        //bufferToFill.clearActiveBufferRegion();
        
        std::unique_lock<std::mutex> lock(process_mutex_);
        
        if(file_loaded_.load()) {

            reader_source_->getNextAudioBlock(bufferToFill);
            
            peak_meter_->SetSamples(bufferToFill.buffer->getReadPointer(0),
                                    bufferToFill.numSamples);
            rms_meter_->SetSamples(bufferToFill.buffer->getReadPointer(0),
                                     bufferToFill.numSamples);
            fft_meter_->SetSamples(bufferToFill.buffer->getReadPointer(0),
                                   bufferToFill.numSamples);
        } else {
            bufferToFill.clearActiveBufferRegion();
        }
    }

    void releaseResources() override
    {
        // This will be called when the audio device stops, or when it is being
        // restarted due to a setting change.

        // For more details, see the help for AudioProcessor::releaseResources()
    }

    //=======================================================================
    void paint (Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (Colours::black);
        
        std::unique_lock<std::mutex> lock(process_mutex_);
 
        peak_meter_component_->SetCurrentPeakLevel(peak_meter_->GetCurrentPeakLevel());
        peak_meter_component_->SetPeakHoldLevel(peak_meter_->GetPeakHoldLevel());
        rms_meter_component_->SetCurrentRMSLevel(rms_meter_->GetRMS());
        for(int i = 0; i < fft_meter_->GetSize() / 2; ++i) {
            fft_meter_component_->SetPeakLevel(fft_meter_->GetPeakSpectrum(i), i);
            fft_meter_component_->SetCurrentPowerLevel(fft_meter_->GetSpectrum(i), i);
        }
    }

    void resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
    
        auto rect = getLocalBounds();
        
        auto const file_operation_area = rect.removeFromTop(30);
        auto const meters_area = rect;
        
        rect = file_operation_area;
        auto file_loading_button_area = rect.removeFromRight(100);
        file_loading_button_.setBounds(file_loading_button_area);
        auto file_path_area = rect;
        file_path_display_.setBounds(file_path_area);
        
        rect = meters_area;

        auto fft_meter_area = rect.removeFromBottom(rect.getHeight() / 2);
        
        auto peak_meter_area = rect.removeFromLeft(rect.getWidth() / 3);
        peak_meter_component_->setBounds(peak_meter_area);
        
        auto rms_meter_area = rect;
        rms_meter_component_->setBounds(rms_meter_area);
        
        fft_meter_component_->setBounds(fft_meter_area);
    }

private:
    //==============================================================================

    // Your private member variables go here...


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }
