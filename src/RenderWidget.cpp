#include "RenderWidget.h"

#include <QDebug>
#include <QScreen>
#include <QFile>


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
{

    m_iChannelTime[0] = 0;
    m_iChannelTime[1] = 0;
    m_iChannelTime[2] = 0;
    m_iChannelTime[3] = 0;
    m_iChannelResolution[0] = vec3({0,0,0});
    m_iChannelResolution[1] = vec3({0,0,0});
    m_iChannelResolution[2] = vec3({0,0,0});
    m_iChannelResolution[3] = vec3({0,0,0});

    float   left  = 0.,
            right  = 1,
            top    = 1,
            bottom = 0,
            far    = 1,
            near   = -1;
    m_projMatrix.ortho(left, right, bottom, top, near, far);

    m_vertexShader   = readFile(":/Shaders/basicShader.vsh");
    m_fragmentShader = attributes() + readFile(":/Shaders/basicShader.fsh");
}

void RenderWidget::rebuildShader(QString content)
{
    m_fragmentShader = attributes() + content;
    qDebug() << m_fragmentShader;
    initialize();
}


QString RenderWidget::attributes()
{
    QString s = "";
    // From ShaderToy.com
    s += "uniform vec3	iResolution;";      // image/buffer	The viewport resolution (z is pixel aspect ratio, usually 1.0)
    //s += "uniform float	iGlobalTime;";      // image/sound/buffer	Current time in seconds
    //s += "uniform float	iTimeDelta;";       // image/buffer	Time it takes to render a frame, in seconds
    s += "uniform int	iFrame;";           // image/buffer	Current frame
    //s += "uniform float	iFrameRate;";       // image/buffer	Number of frames rendered per second
    //s += "uniform float	iChannelTime[4];";  // image/buffer	Time for channel (if video or sound), in seconds
    //s += "uniform vec3	iChannelResolution[4];";   // image/buffer/sound	Input texture resolution for each channel
    //s += "uniform vec4	iMouse;";           // image/buffer	xy = current pixel coords (if LMB is down). zw = click pixel
    //s += "uniform sampler2D iChannel{i};";  // image/buffer/sound	Sampler for input textures i
    //s += "uniform vec4	iDate;";            // image/buffer/sound	Year, month, day, time in seconds in .xyzw
    //s += "uniform float	iSampleRate;";      // image/buffer/sound	The sound sample rate (typically 44100)

    return s;
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
        return;
    }
    if(!m_program->link())
    {
        qWarning() << "Can't link GL program";
        qWarning().noquote() << "Log:" << endl << m_program->log();
        return;
    }
    getUniforms();
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
    //m_iGlobalTimeUniform = m_program->uniformLocation("iGlobalTime");;
    //m_iTimeDeltaUniform = m_program->uniformLocation("iTimeDelta");;
    m_iFrameUniform = m_program->uniformLocation("iFrame");;
    //m_iFrameRateUniform = m_program->uniformLocation("iFrameRate");;
    //m_iChannelTimeUniform = m_program->uniformLocation("iChannelTime");;
    //m_iChannelResolutionUniform = m_program->uniformLocation("iChannelResolution");;
    //m_iMouseUniform = m_program->uniformLocation("iMouse");;
    //m_iDateUniform = m_program->uniformLocation("iDate");;
    //m_iSampleRateUniform = m_program->uniformLocation("iSampleRate");;
}

void RenderWidget::updateUniforms()
{
    const qreal retinaScale = devicePixelRatio();
    m_iResolution = vec3({(GLfloat)(width() * retinaScale), (GLfloat)(height()* retinaScale), 1});
    ++m_iFrame;
}

void RenderWidget::setUniforms()
{
    //qDebug() << m_iResolutionUniform << m_iResolution.x << m_iResolution.y;
    m_program->setUniformValue(m_matrixUniform, m_projMatrix);
    m_program->setUniformValue(m_iResolutionUniform,  m_iResolution.x, m_iResolution.y, m_iResolution.z);
    m_program->setUniformValue(m_iFrameUniform,  m_iFrame);
}

//! [5]
void RenderWidget::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

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
