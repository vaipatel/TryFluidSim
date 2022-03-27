#ifndef SPECTROGRAMWINDOW_H
#define SPECTROGRAMWINDOW_H

#include "OpenGLWindow.h"
#include <random>
#include <vector>

class Blitter;
class Texture;

//struct MyRGBA
//{
//    float m_r;
//    float m_g;
//    float m_b;
//    float m_a;
//};

class SpectrogramWindow : public OpenGLWindow
{
public:
    SpectrogramWindow(QWindow* _parent = nullptr);
    ~SpectrogramWindow() override;

protected:
    void initialize() override;
    void render() override;
//    void cleanup() override;
    void HandleViewPortUpdated() override;

private:
    std::vector<float> CreateFakeMagData(float _centerHz, float _stdDevHz, float _samplingFreqHz, size_t _numMags, float _noise);
    static std::vector<float> ConvertRedToRGBA(const std::vector<float>& _redVals);
    void CleanUpTextures();
    void SetupTextures();

    static constexpr float s_SAMPLING_FREQ_HZ = 2.5e6f;
    static constexpr size_t s_NUM_FFT_SAMPLES = 2500;
    static constexpr size_t s_NUM_TIME_SAMPLES = 1250;
    static constexpr float s_UPDATE_PERIOD_S = 0.01f; //!< Period of time after which to update the spectrogram data to be rendered

    size_t m_halfNumFFTSamples = 0;
    std::vector<std::vector<float>> m_magnitudesOverTime; //!< Collection of num time samples Magnitude vectors, each of length half num fft samples
    std::vector<std::vector<float>> m_inputMagnitudesOverTimeRGBA;
    std::mt19937 m_rng;

    // Convenience for blitting to framebuffer of choice. In our case will just be the screen.
    Blitter* m_blitter = nullptr;

    // Input textures
    Texture* m_spectrogramTexture = nullptr;

    std::vector<float> m_outputTextureMagnitudesOverTimeRGBA;
    std::vector<float> m_textureMagnitudesToUpload;
    int m_texWidth = 0;
    int m_texHeight = 0;

    float m_cumulativeDeltaS = 0.0f;
    int m_currXOffset = 0;
};

#endif // SPECTROGRAMWINDOW_H
