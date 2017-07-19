#include "GlslErrorManager.h"

GlslErrorManager::GlslErrorManager(QObject *parent)
    : QObject(parent)
{
    m_status.clear();
}

QVector<GlslErrorManager::ErrorLog> GlslErrorManager::parseLog(const QString &log) const
{
    //TODO : use real offset
    const int offset = 1;// headerOffset();
    QVector<ErrorLog>res;
    QStringList lines = log.split('\n');
    for(const auto & line : lines)
    {
        if(line.contains("ERROR"))
        {
            ErrorLog errLog;
            QStringList blocks = line.split(':');
            errLog.level = GLSL_ERROR;
            errLog.row = blocks.at(2).toUInt() - offset;
            errLog.col = 0;//blocks.at(1).at(1).toUInt();
            errLog.log = blocks.back();
            res.push_back(errLog);
        }
    }
    return res;
}

QVector<GlslErrorManager::ErrorLog> GlslErrorManager::parseSource(const QString &log) const
{
    QVector<ErrorLog>res;
    //TODO
    return res;
}

void GlslErrorManager::onBuildFinish(QString buildLog)
{
    m_status = parseLog(buildLog);
    emit statusUpdate(m_status);
}

void GlslErrorManager::onSourceChanged(QString sourceCode)
{
    //shader code analysiso
    m_status.clear();
//    m_status = parseSource(sourceCode);

    //signal new status
    emit statusUpdate(m_status);

    //request to rebuild sader program from source
    if(m_status.empty())
        emit rebuildFromSource(sourceCode);
}
