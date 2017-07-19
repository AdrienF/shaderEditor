#ifndef GLSLERRORMANAGER_H
#define GLSLERRORMANAGER_H

#include <QObject>
#include <QVector>

class GlslErrorManager : public QObject
{
    Q_OBJECT
public:
    GlslErrorManager(QObject *parent = NULL);

    enum ErrorLevel{
        GLSL_WARNING = 0,
        GLSL_ERROR = 1,
        GLSL_NONE = 2
    };
    struct ErrorLog{
        ErrorLog(): level(GLSL_NONE), row(-1), col(-1), log(""){}
        ErrorLevel level;
        int row;
        int col;
        QString log;
    };

private:
    //Return the line index (first) and column (second) of lines with errors from a QOpenGLShaderProgram
    QVector<GlslErrorManager::ErrorLog> parseLog(const QString &log) const;
    //Analyse shader source code
    QVector<GlslErrorManager::ErrorLog> parseSource(const QString &log) const;

    QVector<GlslErrorManager::ErrorLog> m_status;

signals:
    void statusUpdate(QVector<ErrorLog>);
    void rebuildFromSource(QString);

public slots:
    void onBuildFinish(QString);
    void onSourceChanged(QString);
};

#endif // GLSLERRORMANAGER_H
