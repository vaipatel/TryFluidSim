#include "FluidEqualizerWindow.h"
#include "Blitter.h"
#include "DoubleRenderTargetBuffer.h"
#include "PointsObject.h"
#include "RenderTargetBuffer.h"
#include "ShaderProgram.h"
#include "Shared.h"
#include "Texture.h"
#include "TrisObject.h"
#include <QAudioEncoderSettings>
#include <QAudioProbe>
#include <QDebug>
#include <QKeyEvent>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QMouseEvent>
#include <QScreen>
#include <QVector2D>
#include <QVector3D>
#include <algorithm>
#include <math.h>
#include "DspFilters/ChebyshevI.h"

FluidEqualizerWindow::FluidEqualizerWindow(QWindow* _parent) : OpenGLWindow(_parent)
{

}



FluidEqualizerWindow::~FluidEqualizerWindow()
{
    delete m_blitter;
    delete m_velocityOutTextureA;
    delete m_velocityOutTextureB;
    delete m_velocityDoubleTargetBuffer;
    delete m_dyeTextureA;
    delete m_dyeTextureB;
    delete m_dyeDoubleTargetBuffer;
    delete m_divergenceTexture;
    delete m_divergenceTargetBuffer;
    delete m_pressureTextureA;
    delete m_pressureTextureB;
    delete m_pressureDoubleTargetBuffer;
    delete m_advectProgram;
    delete m_splatForceProgram;
    delete m_divergenceProgram;
    delete m_pressureSolveProgram;
    delete m_gradientSubtractProgram;
    delete m_vectorsProgram;
    delete m_vectors;
    m_player->stop();

    ClearQueue(m_freeQueue);
    ClearQueue(m_filledQueue);
}

void FluidEqualizerWindow::initialize()
{
    m_blitter = new Blitter;

    m_uVelocityInputTexture = new Texture(m_perlinNoiseImgFileName, TextureData::FilterParam::LINEAR, 0);
    m_uSourceInputTexture = new Texture(m_moscowImgFileName, TextureData::FilterParam::LINEAR, 1);

    m_advectProgram = new ShaderProgram(m_baseVertShaderFileName, m_advectFragShaderFileName);
    m_splatForceProgram = new ShaderProgram(m_baseVertShaderFileName, m_splatForceFragShaderFileName);
    m_divergenceProgram = new ShaderProgram(m_baseVertShaderFileName, m_divergenceFragShaderFileName);
    m_pressureSolveProgram = new ShaderProgram(m_baseVertShaderFileName, m_pressureSolveFragShaderFileName);
    m_gradientSubtractProgram = new ShaderProgram(m_baseVertShaderFileName, m_gradientSubtractFragShaderFileName);
    m_vectorsProgram = new ShaderProgram(m_vectorsVertShaderFileName, m_vectorsGeomShaderFileName, m_vectorsFragShaderFileName);

    m_player = new QMediaPlayer(this);
    m_playlist = new QMediaPlaylist(m_player);
    m_audioProbe = new QAudioProbe(this);


    SetupAudio();
    SetupTextures();
    SetupParticles();
}

void FluidEqualizerWindow::render()
{
    float dtS = static_cast<float>(1/(screen()->refreshRate()));

    if ( m_numSamplesPerChannel != 0 && !m_freeQueueCreated )
    {
        CreateFreeQueuesOfArraysWithArraySize(m_numSamplesPerChannel);
        m_freeQueueCreated = true;
        m_maxPassBandHz = m_numSamplesPerChannel * 2.0;
    }

    int numFilledProcessed = 0;
    const int MAX_NUM_FILLED_PROCESSED = 1;
    while ( !m_filledQueue.empty() && numFilledProcessed < MAX_NUM_FILLED_PROCESSED )
    {
        CArray& audioData = *m_filledQueue.front();
        m_filledQueue.pop_front();

        std::vector<float> copiedVec;
        std::transform(std::begin(audioData), std::end(audioData), std::back_inserter(copiedVec), [](std::complex<double> _c) -> float { return _c.real(); });
        float* a[1] = { copiedVec.data() };
        Dsp::SimpleFilter <Dsp::ChebyshevI::BandPass <3>, 1> f;
        f.setup (3,           // order
                 m_sampleRate,// sample rate
                 m_passBandHz,// center frequency
                 8000,         // band width
                 10);          // ripple dB
        f.process(static_cast<int>(audioData.size()), a);

        size_t numSamplesToPlot = 4;
        size_t samplesStep = audioData.size() / numSamplesToPlot;
        double small_dtS = static_cast<double>(dtS) / static_cast<double>(numSamplesToPlot);
        const double bpm = 108.0;
        const double omega = 2.0 * PI * ConvertBPMToHz(bpm * 1);
        const double radialLengthFactor = 1.0e-1;

        for (size_t idx = 0; idx < numSamplesToPlot; idx++ )
        {
            m_accumTimeS += small_dtS;
            size_t sampleIdx = idx * samplesStep;
            double radialLength = std::abs(copiedVec[sampleIdx]) * radialLengthFactor;
//            radialLength = tanh(radialLength) * 2.0e-1;
            assert(radialLength < 1.0);
            double audioVal = copiedVec[sampleIdx] * radialLengthFactor;
            double radius = 0.25 + audioVal;

            double angleRad = omega * static_cast<double>(m_accumTimeS);
            double currEqX = 0.5 + radius * cos( angleRad );
            double currEqY = 0.5 + radius * sin( angleRad );
            QVector2D currXY(static_cast<float>(currEqX), static_cast<float>(currEqY));
            QVector2D delta;
            QVector2D velocity;

            if ( !m_startedTrail )
            {
                m_startedTrail = true;
            }
            else
            {
                delta = (currXY - m_prevXY); //- 5e-2 * QVector2D(radius * cos( angleRad ), radius * sin( angleRad )); // * (1.0f - 1e-2f) + QVector2D(-radius * sin( angleRad ), radius * cos( angleRad )) * omega * 1e-2f;
                velocity = delta;
                velocity = -velocity * 5e-1f / numSamplesToPlot;

                size_t numSteps = 10;
                float oneStep = static_cast<float>(1.0f)/numSteps;
                if ( m_doFwd )
                {
                    for ( size_t step = 0; step < numSteps; step++ )
                    {
                        float deltaStep = static_cast<float>(step)/numSteps;
                        Splat(m_prevXY.x() + deltaStep * delta.x(), m_prevXY.y() + deltaStep * delta.y(), -SPLAT_FORCE * velocity.x() * oneStep, -SPLAT_FORCE * velocity.y() * oneStep,
                              {static_cast<float>(std::abs(audioVal) / radialLengthFactor),
                               static_cast<float>(audioVal / radialLengthFactor),
                               static_cast<float>(m_passBandHz) / static_cast<float>(m_maxPassBandHz)});
                    }
                }
                else
                {
                    for ( size_t step = 0; step < numSteps; step++ )
                    {
                        float deltaStep = static_cast<float>(numSteps - 1 - step)/numSteps;
                        Splat(currXY.x() - deltaStep * delta.x(), currXY.y() - deltaStep * delta.y(), SPLAT_FORCE * velocity.x() * oneStep, SPLAT_FORCE * velocity.y() * oneStep,
                              {static_cast<float>(std::abs(audioVal) / radialLengthFactor),
                               static_cast<float>(audioVal / radialLengthFactor),
                               static_cast<float>(m_passBandHz) / static_cast<float>(m_maxPassBandHz)});
                    }
                }
            }

            m_prevXY = currXY;
        }

        m_freeQueue.push_back(&audioData);

        numFilledProcessed++;

//        if ( m_filledQueue.empty() && numFilledProcessed < MAX_NUM_FILLED_PROCESSED )
//        {
//            qDebug() << "VAIVAI WOW";
//        }
    }

    // Add forces
    {
        static int maxPosToProcess = 10;
        int processedPos = 0;
        while ( !m_mousePosList.empty() && processedPos < maxPosToProcess )
        {
            processedPos++;

            QVector2D mousePos = m_mousePosList.front();
            m_mousePosList.pop_front();

            QVector2D delta;
            if ( !m_mousePosList.empty() )
            {
                delta = m_mousePosList.front() - mousePos;
            }

            Splat(mousePos.x(), mousePos.y(), SPLAT_FORCE * delta.x(), SPLAT_FORCE * delta.y(), {1.0f, 0.0, 1.0});
        }
    }

    ComputeDivergence();

    SolvePressure();

    SubtractGradient();

    // Advect velocity
    Advect(m_velocityDoubleTargetBuffer, dtS);

    // Advect dye
    Advect(m_dyeDoubleTargetBuffer, dtS);

    m_blitter->BindTarget(nullptr);
    m_blitter->DrawTextureOnScreenQuad(m_dyeDoubleTargetBuffer->GetFirst()->GetTargetTexture());

    // Draw vector field if needed
    DrawVectors();
}

void FluidEqualizerWindow::cleanup()
{

}

void FluidEqualizerWindow::HandleViewPortUpdated()
{
    // Update viewport
    glViewport(0, 0, m_viewWidth, m_viewHeight);

    m_texelSizeX = 1.0f / static_cast<float>(m_viewWidth);
    m_texelSizeY = 1.0f / static_cast<float>(m_viewHeight);

    CleanUpTextures();
    SetupTextures();
}

void FluidEqualizerWindow::mouseMoveEvent(QMouseEvent* _ev)
{
    if ( _ev->buttons() & Qt::LeftButton )
    {
        float x = static_cast<float>(_ev->x()) / static_cast<float>(width());
        float y = 1.0f - static_cast<float>(_ev->y()) / static_cast<float>(height());
        m_mousePosList.push_back({x, y});
    }
}

void FluidEqualizerWindow::keyPressEvent(QKeyEvent* _ev)
{
    if ( _ev->key() == Qt::Key_V )
    {
        m_showVectors = !m_showVectors;
    }
    else if ( _ev->key() == Qt::Key_Up )
    {
        m_passBandHz += m_passBandIncrementHz;
        m_passBandHz = std::max(m_minPassBandHz, std::min(m_passBandHz, m_maxPassBandHz));
        qDebug() << "m_passBandHz" << m_passBandHz;
    }
    else if ( _ev->key() == Qt::Key_Down )
    {
        m_passBandHz -= m_passBandIncrementHz;
        m_passBandHz = std::max(m_minPassBandHz, std::min(m_passBandHz, m_maxPassBandHz));
        qDebug() << "m_passBandHz" << m_passBandHz;
    }
    else if ( _ev->key() == Qt::Key_F )
    {
        m_doFwd = !m_doFwd;
    }
}

void FluidEqualizerWindow::SlotProcessAudioBuffer(const QAudioBuffer& _buffer)
{
    if ( _buffer.isValid() )
    {
        m_sampleRate = _buffer.format().sampleRate();
        m_bytesPerSample = _buffer.format().bytesPerFrame();
        size_t numSamples = static_cast<size_t>(_buffer.sampleCount());
        size_t numChannels = static_cast<size_t>(_buffer.format().channelCount());
        m_numSamplesPerChannel = numSamples/numChannels;

        if ( m_freeQueueCreated )
        {
            if ( !m_freeQueue.empty() )
            {
                CArray* in = m_freeQueue.front();
                m_freeQueue.pop_front();
                if ( _buffer.format().sampleType() == QAudioFormat::SignedInt )
                {
                    assert(m_bytesPerSample == 2 * numChannels);
                    if (in->size() == m_numSamplesPerChannel)
                    {
                        const qint16* data = _buffer.constData<qint16>();
                        for (size_t i = 0; i < m_numSamplesPerChannel; i++)
                        {
                            double val = static_cast<double>(data[i*numChannels]) / SHRT_MAX;
                            (*in)[i] = val;
                        }
            //            FFT(*in);
                    }
                }
                m_filledQueue.push_back(in);
            }
            else
            {
                qDebug() << "VAIVAI OMG!";
            }
        }
    }
}

void FluidEqualizerWindow::CleanUpTextures()
{
    SafeDelete(m_velocityDoubleTargetBuffer);
    SafeDelete(m_velocityOutTextureA);
    SafeDelete(m_velocityOutTextureB);
    SafeDelete(m_dyeDoubleTargetBuffer);
    SafeDelete(m_dyeTextureA);
    SafeDelete(m_dyeTextureB);
    SafeDelete(m_divergenceTargetBuffer);
    SafeDelete(m_divergenceTexture);
    SafeDelete(m_pressureDoubleTargetBuffer);
    SafeDelete(m_pressureTextureA);
    SafeDelete(m_pressureTextureB);
}

void FluidEqualizerWindow::SetupAudio()
{
    QAudioEncoderSettings audioSettings;
    audioSettings.setCodec("audio/wav");
    audioSettings.setChannelCount(2);

    const QString audioFileName = "audio1.wav";
    QFile audioFile(audioFileName);
    assert(audioFile.exists());
    m_player->setMedia(QUrl::fromLocalFile(audioFileName));
    assert(m_player->error() == QMediaPlayer::NoError);

    bool canSetProbeSource = m_audioProbe->setSource(m_player);
    assert(canSetProbeSource);

    bool canConnect = connect(m_audioProbe, SIGNAL(audioBufferProbed(const QAudioBuffer&)), this, SLOT(SlotProcessAudioBuffer(const QAudioBuffer&)));
    assert(canConnect);

    m_player->play();
    assert(m_player->error() == QMediaPlayer::NoError);
}

void FluidEqualizerWindow::SetupTextures()
{
    m_velocityOutTextureA = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::LINEAR, nullptr, 0);
    m_velocityOutTextureB = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::LINEAR, nullptr, 0);
    m_velocityDoubleTargetBuffer = new DoubleRenderTargetBuffer(m_velocityOutTextureA, m_velocityOutTextureB);
    std::vector<RenderTargetBuffer*> velocityBuffers = m_velocityDoubleTargetBuffer->GetBoth();
    foreach (RenderTargetBuffer* velocityBuffer, velocityBuffers)
    {
        ConfigureRenderTarget(velocityBuffer);
    }

    m_dyeTextureA = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::LINEAR, nullptr, 0);
    m_dyeTextureB = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::LINEAR, nullptr, 0);
    m_dyeDoubleTargetBuffer = new DoubleRenderTargetBuffer(m_dyeTextureA, m_dyeTextureB);
    std::vector<RenderTargetBuffer*> dyeBuffers = m_dyeDoubleTargetBuffer->GetBoth();
    foreach (RenderTargetBuffer* dyeBuffer, dyeBuffers)
    {
        ConfigureRenderTarget(dyeBuffer);
    }

    m_divergenceTexture = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::NEAREST, nullptr, 0);
    m_divergenceTargetBuffer = new RenderTargetBuffer(m_divergenceTexture);
    ConfigureRenderTarget(m_divergenceTargetBuffer);

    m_pressureTextureA = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::NEAREST, nullptr, 0);
    m_pressureTextureB = new Texture(m_viewWidth, m_viewHeight, GL_RGBA, GL_FLOAT, TextureData::FilterParam::NEAREST, nullptr, 0);
    m_pressureDoubleTargetBuffer =  new DoubleRenderTargetBuffer(m_pressureTextureA, m_pressureTextureB);
    std::vector<RenderTargetBuffer*> pressureBuffers = m_pressureDoubleTargetBuffer->GetBoth();
    foreach(RenderTargetBuffer* pressureBuffer, pressureBuffers)
    {
        ConfigureRenderTarget(pressureBuffer);
    }
}

void FluidEqualizerWindow::SetupParticles()
{
    size_t numParticlesX = 96;
    size_t numParticlesY = 96;
    float cellSizeX = 2.0f / static_cast<float>(numParticlesX);
    float cellSizeY = 2.0f / static_cast<float>(numParticlesY);
    float startX = -1.0f;
    float startY = -1.0f;
    std::vector<Point> m_particlePts;
    for (size_t i = 0; i < numParticlesX; i++)
    {
        for (size_t j = 0; j < numParticlesY; j++)
        {
            float x = startX + (static_cast<float>(i) + 0.5f) * cellSizeX;
            float y = startY + (static_cast<float>(j) + 0.5f) * cellSizeY;
            m_particlePts.push_back({x, y, 0.0f});
        }
    }

    m_vectors = new PointsObject(m_particlePts);
}

void FluidEqualizerWindow::Advect(DoubleRenderTargetBuffer* _doubleBuffer, float _dt)
{
    // Bind advect shader program
    m_advectProgram->Bind();

    // Pass quantity to be advected
    Texture* textureToBeAdvected = _doubleBuffer->GetFirst()->GetTargetTexture();
    size_t textureToBeAdvectedUnitId = 0;
    textureToBeAdvected->Bind(0, &textureToBeAdvectedUnitId);
    m_advectProgram->SetUniform("uSource", static_cast<int>(textureToBeAdvectedUnitId));

    // Pass velocity buffer
    Texture* velTex = m_velocityDoubleTargetBuffer->GetFirst()->GetTargetTexture();
    size_t velTexUnitId = 0;
    if ( _doubleBuffer != m_velocityDoubleTargetBuffer )
    {
        velTexUnitId = 1; // If advected quantity is not velocity itself, bind velocity to different texture unit
    }
    velTex->Bind(0, &velTexUnitId);
    m_advectProgram->SetUniform("uVelocity", static_cast<int>(velTexUnitId));

    // Pass time
    m_advectProgram->SetUniform("dt", _dt);

    // Pass dissipation
    float dissipation = 0.7f;
    if ( _doubleBuffer != m_velocityDoubleTargetBuffer )
    {
        dissipation = 1.f;
    }
    m_advectProgram->SetUniform("dissipation", dissipation);

    // Pass cell size
    m_advectProgram->SetUniform("texelSize", {m_texelSizeX, m_texelSizeY});

    // Blit advected result onto second buffer
    m_blitter->BlitToTarget(_doubleBuffer->GetSecond());

    // Release advect shader program
    m_advectProgram->Release();

    // Swap double buffers. Next time advected result will be read from.
    _doubleBuffer->SwapBuffers();
}

void FluidEqualizerWindow::Splat(float _x, float _y, float _dx, float _dy, const QVector3D& _color)
{
    // Bind the splat force shader
    m_splatForceProgram->Bind();

    // Pass first velocity buffer's texture
    Texture* velTex = m_velocityDoubleTargetBuffer->GetFirst()->GetTargetTexture();
    size_t velTexUnitId = 0;
    velTex->Bind(0, &velTexUnitId);
    m_splatForceProgram->SetUniform("uSource", static_cast<int>(velTexUnitId));

    // Pass other params
    m_splatForceProgram->SetUniform("aspectRatio", m_viewAspect);
    m_splatForceProgram->SetUniform("color", QVector3D(_dx, _dy, 0.0f));
    m_splatForceProgram->SetUniform("point", QVector2D(_x, _y));
    m_splatForceProgram->SetUniform("radius", 0.25f / 1600.0f);

    // Blit result onto second velocity buffer
    m_blitter->BlitToTarget(m_velocityDoubleTargetBuffer->GetSecond());

    // Swap velocity buffers
    m_velocityDoubleTargetBuffer->SwapBuffers();

    // Pass first dye buffer's texture
    Texture* dyeTex = m_dyeDoubleTargetBuffer->GetFirst()->GetTargetTexture();
    size_t dyeTexUnitId = 1;
    dyeTex->Bind(0, &dyeTexUnitId);
    m_splatForceProgram->SetUniform("uSource", static_cast<int>(dyeTexUnitId));

    // Pass the color
    m_splatForceProgram->SetUniform("color", _color);

    // Blit result to second dye buffer
    m_blitter->BlitToTarget(m_dyeDoubleTargetBuffer->GetSecond());

    // Release the splat force shader
    m_splatForceProgram->Release();

    // Swap dye buffers
    m_dyeDoubleTargetBuffer->SwapBuffers();
}

void FluidEqualizerWindow::ComputeDivergence()
{
    m_divergenceProgram->Bind();

    // Pass first velocity buffer's texture
    Texture* velTex = m_velocityDoubleTargetBuffer->GetFirst()->GetTargetTexture();
    size_t velTexUnitId = 0;
    velTex->Bind(0, &velTexUnitId);
    m_divergenceProgram->SetUniform("uVelocity", static_cast<int>(velTexUnitId));

    // Pass cell size
    m_divergenceProgram->SetUniform("texelSize", {m_texelSizeX, m_texelSizeY});

    m_blitter->BlitToTarget(m_divergenceTargetBuffer);

    m_divergenceProgram->Release();
}

void FluidEqualizerWindow::SolvePressure()
{
    m_pressureSolveProgram->Bind();

    // Pass divergence texture
    Texture* divTex = m_divergenceTargetBuffer->GetTargetTexture();
    size_t divTexUnitId = 0;
    divTex->Bind(0, &divTexUnitId);
    m_pressureSolveProgram->SetUniform("uDivergence", static_cast<int>(divTexUnitId));

    // Pass cell size
    m_pressureSolveProgram->SetUniform("texelSize", {m_texelSizeX, m_texelSizeY});

    for (size_t i = 0; i < 40; i++)
    {
        // Pass curr pressure texture
        Texture* pressureTex = m_pressureDoubleTargetBuffer->GetFirst()->GetTargetTexture();
        size_t pressureTexUnitId = 1;
        pressureTex->Bind(0, &pressureTexUnitId);
        m_pressureSolveProgram->SetUniform("uPressure", static_cast<int>(pressureTexUnitId));

        m_blitter->BlitToTarget(m_pressureDoubleTargetBuffer->GetSecond());

        m_pressureDoubleTargetBuffer->SwapBuffers();
    }

    m_pressureSolveProgram->Release();
}

void FluidEqualizerWindow::SubtractGradient()
{
    m_gradientSubtractProgram->Bind();

    // Pass cell size
    m_gradientSubtractProgram->SetUniform("texelSize", {m_texelSizeX, m_texelSizeY});

    // Pass first velocity buffer's texture
    Texture* velTex = m_velocityDoubleTargetBuffer->GetFirst()->GetTargetTexture();
    size_t velTexUnitId = 0;
    velTex->Bind(0, &velTexUnitId);
    m_gradientSubtractProgram->SetUniform("uVelocity", static_cast<int>(velTexUnitId));

    // Pass first pressure buffer's texture
    Texture* pressureTex = m_pressureDoubleTargetBuffer->GetFirst()->GetTargetTexture();
    size_t pressureTexUnitId = 1;
    pressureTex->Bind(0, &pressureTexUnitId);
    m_gradientSubtractProgram->SetUniform("uPressure", static_cast<int>(pressureTexUnitId));

    m_blitter->BlitToTarget(m_velocityDoubleTargetBuffer->GetSecond());

    m_velocityDoubleTargetBuffer->SwapBuffers();

    m_gradientSubtractProgram->Release();
}

void FluidEqualizerWindow::DrawVectors()
{
    if ( m_showVectors )
    {
        m_vectorsProgram->Bind();

        // Pass first velocity buffer's texture
        Texture* velTex = m_velocityDoubleTargetBuffer->GetFirst()->GetTargetTexture();
        size_t velTexUnitId = 0;
        velTex->Bind(0, &velTexUnitId);
        m_vectorsProgram->SetUniform("uSource", static_cast<int>(velTexUnitId));

        m_vectors->Draw();

        m_vectorsProgram->Release();
    }
}

void FluidEqualizerWindow::ConfigureRenderTarget(RenderTargetBuffer* _renderTarget)
{
    _renderTarget->SetDepthTestEnabled(false);
    _renderTarget->SetClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    _renderTarget->SetClearColor({38, 38, 38, 255});
}

void FluidEqualizerWindow::FFT(CArray& _x)
{
    const size_t N = _x.size();
    if (N <= 1) return;

    // divide
    CArray even = _x[std::slice(0, N/2, 2)];
    CArray  odd = _x[std::slice(1, N/2, 2)];

    // conquer
    FFT(even);
    FFT(odd);

    // combine
    for (size_t k = 0; k < N/2; ++k)
    {
        Complex t = std::polar(1.0, -2 * PI * k / N) * odd[k];
        _x[k    ] = even[k] + t;
        _x[k+N/2] = even[k] - t;
    }
}

double FluidEqualizerWindow::ConvertBPMToHz(double _bpm)
{
    return _bpm * (1.0/60.0);
}

void FluidEqualizerWindow::ClearQueue(std::list<CArray*>& _queue)
{
    while ( !_queue.empty() )
    {
        CArray* buffer = _queue.front();
        delete buffer;
        _queue.pop_front();
    }
}

void FluidEqualizerWindow::CreateFreeQueuesOfArraysWithArraySize(size_t _freeQueueFFTArraySize)
{
    ClearQueue(m_freeQueue);
    for ( size_t i = 0; i < MAX_AUDIO_QUEUE_SIZE; i++ )
    {
        m_freeQueue.push_back(new CArray(_freeQueueFFTArraySize));
    }
}

