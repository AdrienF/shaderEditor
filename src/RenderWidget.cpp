#include "RenderWidget.h"

#include <QDebug>
#include <QScreen>
#include <QFile>
#include <QMouseEvent>


QString readFile(const QString &filename)
{
    QString res = "";
    QFile f(filename);
    if(!f.open(QFile::ReadOnly | QFile::Text))
    {
        qWarning() << "Can't open file" << filename;
    }
    else
    {
        QTextStream stream(&f);
        while (!stream.atEnd()){
            res += stream.readLine() + "\n";
        }
    }
    return res + "\n";
}

RenderWidget::RenderWidget()
    : m_iResolution ( vec3({640., 480., 1.}) )
    , m_iGlobalTime   (0)
    , m_iTimeDelta    (0)
    , m_iFrame        (-1)
    , m_iFrameRate    (0)
    , m_iMouse        ( vec4({0,0, 0, 0}) )
    , m_iDate         ( vec4{1987, 2, 7, 0} )
    , m_iSampleRate   ( 44100)
    , m_mousePressedPos(0.,0.)
    , m_mouseCurrentPos(0.,0.)
    , m_retinaScale (devicePixelRatio())
{

    m_iChannel.resize(4);   //up to 4 input textures
    m_iTextures.resize(4);
    m_iChannelTime[0] = 0;
    m_iChannelTime[1] = 0;
    m_iChannelTime[2] = 0;
    m_iChannelTime[3] = 0;
    m_iChannelResolution[0] = vec3({0,0,0});
    m_iChannelResolution[1] = vec3({0,0,0});
    m_iChannelResolution[2] = vec3({0,0,0});
    m_iChannelResolution[3] = vec3({0,0,0});

    const float left        =  0.,
                right       =  1.,
                top         =  1.,
                bottom      =  0.,
                farPlane    =  1.,
                nearPlane   = -1.;
    m_projMatrix.ortho(left, right, bottom, top, nearPlane, farPlane);

    m_vertexShader   = readFile(":/Shaders/basicShader.vsh");
    m_fragmentShader = attributes() + readFile(":/Shaders/basicShader.fsh");

    m_date = QDate::currentDate();

    // A timer to send regular info to UI
    m_eventTimer.reset(new QTimer(this));
    connect(m_eventTimer.get(), &QTimer::timeout, [this](){
        emit sendFPS(m_iFrameRate);
        emit sendGlobalTime(m_iGlobalTime);
    });
    m_eventTimer->start(100);
}

void RenderWidget::rebuildShader(QString content)
{
    m_fragmentShader = attributes() + formatFromShaderToy( content );
    initialize();
}

//return the number of lines of code that have been prepended to the shader displayed in editor
int RenderWidget::headerOffset() const
{
    return attributes().count('\n') + 1 ;
}

QString RenderWidget::attributes()
{
    QString s = "";
    // From ShaderToy.com
    s += "uniform vec3	iResolution;";      // image/buffer	The viewport resolution (z is pixel aspect ratio, usually 1.0)
    s += "uniform float	iGlobalTime;";      // image/sound/buffer	Current time in seconds - DEPRECATED
    s += "uniform float	iTime;";            // image/sound/buffer	Current time in seconds
    s += "uniform float	iTimeDelta;";       // image/buffer	Time it takes to render a frame, in seconds
    s += "uniform int	iFrame;";           // image/buffer	Current frame
    s += "uniform float	iFrameRate;";       // image/buffer	Number of frames rendered per second
    s += "uniform vec4	iMouse;";           // image/buffer	xy = current pixel coords (if LMB is down). zw = click pixel
    s += "uniform sampler2D iChannel0;";  // image/buffer/sound	Sampler for input textures i
    s += "uniform sampler2D iChannel1;";  // image/buffer/sound	Sampler for input textures i
    s += "uniform sampler2D iChannel2;";  // image/buffer/sound	Sampler for input textures i
    s += "uniform sampler2D iChannel3;";  // image/buffer/sound	Sampler for input textures i
    //s += "uniform float	iChannelTime[4];";  // image/buffer	Time for channel (if video or sound), in seconds
    //s += "uniform vec3	iChannelResolution[4];";   // image/buffer/sound	Input texture resolution for each channel
    //s += "uniform vec4	iDate;";            // image/buffer/sound	Year, month, day, time in seconds in .xyzw
    //s += "uniform float	iSampleRate;";      // image/buffer/sound	The sound sample rate (typically 44100)

    return s;
}


QString RenderWidget::formatFromShaderToy(const QString& content)
{
    QString res(content);

    //adapt mainImage()
    if(content.contains("mainImage", Qt::CaseInsensitive))
    {
        res += "\n"
                "void main(){ mainImage(gl_FragColor, gl_FragCoord.xy); }\n";
    } else if(content.contains("void mainSound(", Qt::CaseInsensitive))
    {
        qWarning() << "*** mainSound() entry point not suported";
        res = "";
    } else if(content.contains("void mainVR(", Qt::CaseInsensitive))
    {
        qWarning() << "*** mainVR() entry point not suported";
        res = "";
    }
    return res;
}

void RenderWidget::initialize()
{
    m_program.reset( new QOpenGLShaderProgram(this) );
    if(!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, m_vertexShader.toStdString().c_str()))
    {
        qWarning() << "Can't compile vertex shader";
        qWarning() << "Source:" << endl << m_vertexShader;
        qWarning().noquote() << "Log:" << endl << m_program->log();
        return;
    }
    if(!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, m_fragmentShader.toStdString().c_str()))
    {
        qWarning().noquote() << "Can't compile fragment shader";
        qWarning().noquote() << "Source:" << endl << m_fragmentShader;
        qWarning().noquote() << "Log:" << endl << m_program->log();
        emit buildChanged(m_program->log());
        return;
    }
    if(!m_program->link())
    {
        qWarning() << "Can't link GL program";
        qWarning().noquote() << "Log:" << endl << m_program->log();
        emit buildChanged(m_program->log());
        return;
    }
    emit buildChanged(m_program->log());
    getUniforms();
    m_timer.restart();
    m_iFrame = 0;
}

//! [4]

void RenderWidget::getUniforms()
{
    //vertex shader attributes
    m_posAttr = m_program->attributeLocation("posAttr");
    m_colAttr = m_program->attributeLocation("colAttr");
    m_matrixUniform = m_program->uniformLocation("matrix");
    //fragment shader attributes
    m_iResolutionUniform = m_program->uniformLocation("iResolution");
    m_iGlobalTimeUniform = m_program->uniformLocation("iGlobalTime");;
    m_iTimeUniform = m_program->uniformLocation("iTime");;
    m_iTimeDeltaUniform = m_program->uniformLocation("iTimeDelta");;
    m_iFrameUniform = m_program->uniformLocation("iFrame");;
    m_iFrameRateUniform = m_program->uniformLocation("iFrameRate");
    for(int i=0; i<4; ++i)
    {
        m_iChannel[i] = m_program->uniformLocation(QString("iChannel%1").arg(i));
    }
    //m_iChannelTimeUniform = m_program->uniformLocation("iChannelTime");;
    //m_iChannelResolutionUniform = m_program->uniformLocation("iChannelResolution");;
    m_iMouseUniform = m_program->uniformLocation("iMouse");;
    //m_iDateUniform = m_program->uniformLocation("iDate");;
    //m_iSampleRateUniform = m_program->uniformLocation("iSampleRate");;
}

void RenderWidget::updateUniforms()
{
    m_iResolution = vec3({(GLfloat)(width() * m_retinaScale), (GLfloat)(height()* m_retinaScale), 1.});
    ++m_iFrame;

    float elapsedSec = ( m_timer.elapsed() ) / (1000.);
    float deltaSec   = ( elapsedSec -m_iGlobalTime );
    m_iGlobalTime    = elapsedSec;
    m_iTimeDelta     = deltaSec;
    m_iFrameRate     = m_iFrame/elapsedSec;
}

void RenderWidget::setUniforms()
{
    m_program->setUniformValue(m_matrixUniform, m_projMatrix);
    m_program->setUniformValue(m_iResolutionUniform,  m_iResolution.x, m_iResolution.y, m_iResolution.z);
    m_program->setUniformValue(m_iFrameUniform,  m_iFrame);
    m_program->setUniformValue(m_iTimeDeltaUniform,  m_iTimeDelta);
    m_program->setUniformValue(m_iGlobalTimeUniform,  m_iGlobalTime);
    m_program->setUniformValue(m_iTimeUniform,  m_iGlobalTime);
    m_program->setUniformValue(m_iFrameRateUniform,  m_iFrameRate);

    for(int i=0; i<m_iTextures.size(); ++i){
        auto &tex = m_iTextures[i];
        if(tex)
        {
            m_program->setUniformValue(m_iChannel[i],  i);
        }
    }

    vec4 mouseValue;
    mouseValue.x = m_mouseCurrentPos.x();
    mouseValue.y = m_iResolution.y - m_mouseCurrentPos.y();
    mouseValue.z = m_mousePressedPos.x();
    mouseValue.w = m_iResolution.y - m_mousePressedPos.y();

    m_program->setUniformValue(m_iMouseUniform,  mouseValue.x, mouseValue.y, mouseValue.z, mouseValue.w);

}

//! [5]
void RenderWidget::render()
{
    glViewport(0, 0, width() * m_retinaScale, height() * m_retinaScale);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    for(int i=0; i<4; ++i)
    {
        auto &tex = m_iTextures.at(i);
        if(tex)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            tex->bind();
        }
    }
    updateUniforms();
    setUniforms();

    GLfloat vertices[] = {
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };

    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f
    };

    glVertexAttribPointer(m_posAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, colors);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);


    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    m_program->release();

}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(Qt::LeftButton & event->buttons())
    {
        m_mouseCurrentPos =  event->localPos() * m_retinaScale;
    }
}

void RenderWidget::mousePressEvent(QMouseEvent *event)
{
   m_mousePressedPos = event->localPos() * m_retinaScale;
   m_mouseCurrentPos =  m_mousePressedPos ;
}

void RenderWidget::mouseReleaseEvent(QMouseEvent *event)
{
}

void RenderWidget::updateTexture(int idx, QImage img)
{
    m_iTextures[idx].reset( new QOpenGLTexture(QOpenGLTexture::Target2D) );

    // set bilinear filtering mode for texture magnification and minification
    m_iTextures[idx]->setMinificationFilter(QOpenGLTexture::Linear);
    m_iTextures[idx]->setMagnificationFilter(QOpenGLTexture::Linear);

    // set the wrap mode
    m_iTextures[idx]->setWrapMode(QOpenGLTexture::Repeat);

    //transfer image data to server side
    m_iTextures[idx]->setData(img.mirrored(), QOpenGLTexture::MipMapGeneration::DontGenerateMipMaps);
}
