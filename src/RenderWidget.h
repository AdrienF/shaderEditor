#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QWindow>
#include <QOpenGLShaderProgram>
#include "openglwindow.h"

class RenderWidget : public OpenGLWindow
{
    Q_OBJECT
public:
    explicit RenderWidget();
    void initialize() Q_DECL_OVERRIDE;
    void render() Q_DECL_OVERRIDE;

private:
    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

    QString m_vertexShader;
    QString m_fragmentShader;

    std::unique_ptr<QOpenGLShaderProgram> m_program;
    int m_frame;

public slots:
    void rebuildShader(QString content);
};

#endif // RENDERWIDGET_H
