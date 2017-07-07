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

    static QString attributes() ;

    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

    GLuint m_widthUniform;
    GLuint m_heightUniform;

    QString m_vertexShader;
    QString m_fragmentShader;

    std::unique_ptr<QOpenGLShaderProgram> m_program;
    int m_frame;

public slots:
    void rebuildShader(QString content);
};

#endif // RENDERWIDGET_H
