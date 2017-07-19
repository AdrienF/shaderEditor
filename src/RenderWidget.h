#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QWindow>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QDateTime>
#include <QTimer>
#include "openglwindow.h"

class RenderWidget : public OpenGLWindow
{
    Q_OBJECT
public:
    explicit RenderWidget();
    void initialize() Q_DECL_OVERRIDE;
    void render() Q_DECL_OVERRIDE;


private:

    //The shader code block declaring all defaults uniforms
    static QString attributes() ;
    //Size (in lines) of the header
    int headerOffset() const ;
    //In shaderToy the entry point could optionally be mainImage, mainSound, mainVR
    QString formatFromShaderToy(const QString& content);
    //Get the default uniforms id
    void getUniforms();
    //Update uniform values
    void updateUniforms();
    //Set the default uniforms values
    void setUniforms();

    //timer
    QDate m_date;
    QTime m_timer;
    std::unique_ptr<QTimer> m_eventTimer;

    //mouse position
    const qreal m_retinaScale;
    QPointF m_mousePressedPos;
    QPointF m_mouseCurrentPos;

    //vertex shader uniforms
    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;
    QMatrix4x4 m_projMatrix;

    //fragment shader uniforms
    GLuint m_iResolutionUniform;
    GLuint m_iGlobalTimeUniform;    //Will soon be deprecated in ShaderToy
    GLuint m_iTimeUniform;
    GLuint m_iTimeDeltaUniform;
    GLuint m_iFrameUniform;
    GLuint m_iFrameRateUniform;
    GLuint m_iChannelTimeUniform[4];
    GLuint m_iChannelResolutionUniform[4];
    GLuint m_iMouseUniform;
    GLuint m_iDateUniform;
    GLuint m_iSampleRateUniform;
    QVector<GLuint> m_iChannel;
    std::vector<std::unique_ptr<QOpenGLTexture>> m_iTextures;

    //Shader text code
    QString m_vertexShader;
    QString m_fragmentShader;

    //The OpenGL program being executed
    std::unique_ptr<QOpenGLShaderProgram> m_program;

    typedef struct vec2{
        GLfloat x;
        GLfloat y;
    } vec2;
    typedef struct vec3{
        GLfloat x;
        GLfloat y;
        GLfloat z;
    } vec3;
    typedef struct vec4{
        GLfloat x;
        GLfloat y;
        GLfloat z;
        GLfloat w;
    } vec4;

    //Shaders default inputs
    vec3	m_iResolution;	        //image/buffer	The viewport resolution (z is pixel aspect ratio, usually 1.0)
    float	m_iGlobalTime;		    //image/sound/buffer	Current time in seconds
    float	m_iTimeDelta;	        // image/buffer	Time it takes to render a frame, in seconds
    int	    m_iFrame;		        //image/buffer	Current frame
    float	m_iFrameRate;           //image/buffer	Number of frames rendered per second
    float	m_iChannelTime[4];	    //image/buffer	Time for channel (if video or sound), in seconds
    vec3	m_iChannelResolution[4];//image/buffer/sound	Input texture resolution for each channel
    vec4	m_iMouse;		        //image/buffer	xy = current pixel coords (if LMB is down). zw = click pixel
    //sampler2D m_iChannel{i};	    //image/buffer/sound	Sampler for input textures i
    vec4	m_iDate;		        //image/buffer/sound	Year, month, day, time in seconds in .xyzw
    float	m_iSampleRate;	        //image/buffer/sound	The sound sample rate (typically 44100)

public slots:
    //Shader code content has changed, need to rebuild the program
    void rebuildShader(QString content);
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void updateTexture(int idx, QImage img);

signals:
    void buildChanged(QString);
    void sendFPS(float);
    void sendGlobalTime(float);
};

#endif // RENDERWIDGET_H
