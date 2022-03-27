#include "SpectrogramWindow.h"
#include "Blitter.h"
#include "Shared.h"
#include "Texture.h"
#include <QOpenGLExtraFunctions> // TODO: Remove after glTexSubImage2D is moved to Texture
#include <QScreen>
#include <cmath>

SpectrogramWindow::SpectrogramWindow(QWindow* _parent) : OpenGLWindow(_parent)
{
    m_rng.seed(1);

    m_halfNumFFTSamples = s_NUM_FFT_SAMPLES / 2;

    // Create the magnitudes data, just as single red values
    float lastCenterHz = std::numeric_limits<float>::max();
    float useCenterHz = 0.0f;
    float noise = 0.0f;
    for ( size_t timeIdx = 0; timeIdx < s_NUM_TIME_SAMPLES; timeIdx++ )
    {
        float ratio = static_cast<float>(timeIdx)/s_NUM_TIME_SAMPLES;
        float centerHz = (1.0f * s_SAMPLING_FREQ_HZ) * ratio; // Go from [0, sampling_freq/2)
        centerHz = std::round(centerHz/100000.0f) * 100000.0f;
        if ( std::fabs(centerHz - lastCenterHz) > 0.001f )
        {
            lastCenterHz = centerHz;
            useCenterHz = centerHz + 0.4f * s_SAMPLING_FREQ_HZ * static_cast<float>(m_rng())/static_cast<float>(m_rng.max());
            useCenterHz = std::min(useCenterHz, s_SAMPLING_FREQ_HZ);
            noise = 0.4f * static_cast<float>(m_rng() - m_rng.min())/static_cast<float>(m_rng.max() - m_rng.min());
        }
        float stdDevHz = 50000.0f * (ratio + 1.0f);           // Go from m*[1, 2)
        std::vector<float> magnitudesAtTimeIdx = CreateFakeMagData(useCenterHz, stdDevHz, s_SAMPLING_FREQ_HZ, m_halfNumFFTSamples, noise);
        m_magnitudesOverTime.push_back(magnitudesAtTimeIdx);
    }

    // Convert the single red magnitudes to RGBA vals
//    MyRGBA black = {0.0f, 0.0f, 0.0f, 0.0f};
    m_textureMagnitudesToUpload = ConvertRedToRGBA(m_magnitudesOverTime[0]);
    std::fill(m_textureMagnitudesToUpload.begin(), m_textureMagnitudesToUpload.end(), 0.0f);
    for ( size_t timeIdx = 0; timeIdx < m_magnitudesOverTime.size(); timeIdx++ )
    {
        std::vector<float> convertedMags = ConvertRedToRGBA(m_magnitudesOverTime[timeIdx]);
        m_inputMagnitudesOverTimeRGBA.push_back(convertedMags);

        for ( size_t magIdx = 0; magIdx < convertedMags.size(); magIdx++ )
        {
            m_outputTextureMagnitudesOverTimeRGBA.push_back(0.1f);
        }
    }

    m_texWidth = static_cast<int>(m_magnitudesOverTime.size());
    m_texHeight = static_cast<int>(m_magnitudesOverTime[0].size());
}

SpectrogramWindow::~SpectrogramWindow()
{
    delete m_blitter;
    delete m_spectrogramTexture;
}

void SpectrogramWindow::initialize()
{
    m_blitter = new Blitter;

    SetupTextures();

    m_cumulativeDeltaS = 0.0f;
}

void SpectrogramWindow::render()
{
    float dtS = static_cast<float>(1/(screen()->refreshRate()));

    m_cumulativeDeltaS += dtS;

    if ( m_cumulativeDeltaS > s_UPDATE_PERIOD_S )
    {
        m_cumulativeDeltaS = 0.0f;

        QOpenGLExtraFunctions* extraFuncs = QOpenGLContext::currentContext()->extraFunctions();
        if ( extraFuncs )
        {
            // Copy the data
            m_textureMagnitudesToUpload = m_inputMagnitudesOverTimeRGBA[static_cast<size_t>(m_currXOffset)];

            // Update the pixels
            m_spectrogramTexture->Bind();
//            extraFuncs->glPixelStorei( GL_UNPACK_ROW_LENGTH, 1 );
            checkGLError();
            extraFuncs->glTexSubImage2D(GL_TEXTURE_2D, 0, m_currXOffset, 0, 1, m_texHeight, GL_RGBA, GL_FLOAT, m_textureMagnitudesToUpload.data());
            checkGLError();
//            extraFuncs->glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
            m_currXOffset = (m_currXOffset + 1) % m_texWidth;
        }
    }

    m_spectrogramTexture->Bind();
    m_blitter->BlitToScreen(m_spectrogramTexture);
}

void SpectrogramWindow::HandleViewPortUpdated()
{
    // Update viewport
    glViewport(0, 0, m_viewWidth, m_viewHeight);

    CleanUpTextures();
    SetupTextures();
}

std::vector<float> SpectrogramWindow::CreateFakeMagData(float _centerHz, float _stdDevHz, float _samplingFreqHz, size_t _halfNumFFTSamples, float _noise)
{
    std::vector<float> magnitudes(_halfNumFFTSamples);
    for (size_t i = 0; i < _halfNumFFTSamples; i++)
    {
        float freqAtIHz = _samplingFreqHz * static_cast<float>(i)/static_cast<float>(_halfNumFFTSamples);
        float normalizedFreq = (freqAtIHz - _centerHz)/_stdDevHz;
        float exponent = -0.5f * std::pow(normalizedFreq, 2.0f);
        _noise += 1e-6f * static_cast<float>(m_rng() - m_rng.min())/static_cast<float>(m_rng.max() - m_rng.min());
        magnitudes[i] = std::exp(exponent) + _noise;
        magnitudes[i] = std::min(magnitudes[i], 1.0f);
    }

    return magnitudes;
}

std::vector<float> SpectrogramWindow::ConvertRedToRGBA(const std::vector<float>& _redVals)
{
    std::vector<float> output(4 * _redVals.size(), 0.0f);
    for ( size_t i = 0; i < _redVals.size(); i++ )
    {
        output[i * 4] = _redVals[i];
        output[i * 4 + 3] = 1.0f;
    }

    return output;
}

void SpectrogramWindow::CleanUpTextures()
{
    SafeDelete(m_spectrogramTexture);
}

void SpectrogramWindow::SetupTextures()
{
    m_spectrogramTexture = new Texture({{m_texWidth, m_texHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::LINEAR, m_outputTextureMagnitudesOverTimeRGBA.data()}});
}
