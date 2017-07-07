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
    : m_frame(0)
{
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
    QString s;
    s += "uniform float iWidth;";
    s += "uniform float iHeight;";
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
    //vertex shader attributes
    m_posAttr = m_program->attributeLocation("posAttr");
    m_colAttr = m_program->attributeLocation("colAttr");
    m_matrixUniform = m_program->uniformLocation("matrix");
    //fragment shader attributes
    m_widthUniform = m_program->uniformLocation("iWidth");
    m_heightUniform = m_program->uniformLocation("iHeight");


}
//! [4]

//! [5]
void RenderWidget::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    QMatrix4x4 matrix;
    float   left  = 0.,
            right  = 1,
            top    = 1,
            bottom = 0,
            far    = 1,
            near   = -1;
    matrix.ortho(left, right, bottom, top, near, far);

    m_program->setUniformValue(m_matrixUniform, matrix);

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

     m_program->setUniformValue(m_widthUniform,  (GLfloat)(width() * retinaScale));
     m_program->setUniformValue(m_heightUniform, (GLfloat)(height()* retinaScale));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    m_program->release();

    ++m_frame;
}
