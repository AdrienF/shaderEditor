#ifndef UISHADEREDITOR_H
#define UISHADEREDITOR_H

#include <QTabWidget>
#include <RenderWidget.h>

namespace Ui {
class UIShaderEditor;
}
class ShaderEditor;

class UIShaderEditor : public QTabWidget
{
    Q_OBJECT

public:
    explicit UIShaderEditor(QWidget *parent = 0);
    ~UIShaderEditor();

    ShaderEditor *editor();
    const QString& documentName() const {return m_filename;}
    void setDocumentName(const QString& docName);
    void openFile(const QString &fileName);
    void setErrorLines(QVector<RenderWidget::ErrorLog>);

public slots:
    void updateFPS(float);
    void updateGlobalTime(float);

signals:
    void requestShaderValidation(QString);

private:
    Ui::UIShaderEditor *ui;
    QString m_filename;
    QVector<RenderWidget::ErrorLog> m_errorLines;

    void updateErrLog();
};

#endif // UISHADEREDITOR_H
