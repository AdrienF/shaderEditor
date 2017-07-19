#ifndef UISHADEREDITOR_H
#define UISHADEREDITOR_H

#include <QTabWidget>
#include <GlslErrorManager.h>

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

public slots:
    void updateFPS(float);
    void updateGlobalTime(float);
    void setErrorLines(QVector<GlslErrorManager::ErrorLog>);

private slots:
    void on_pushButtonTex_0_pressed();
    void on_pushButtonTex_1_pressed();
    void on_pushButtonTex_2_pressed();
    void on_pushButtonTex_3_pressed();
    void on_toolButtonHelp_pressed();

signals:
    void requestShaderValidation(QString);
    void updateTexture(int, QImage);

private:
    struct ImageRep{
        QString path;
        QImage  img;
        QIcon   icon;
    };
    Ui::UIShaderEditor *ui;
    QString m_filename;
    QVector<GlslErrorManager::ErrorLog> m_errorLines;
    QVector<ImageRep> m_textureRep;

    void updateErrLog();
    void updateTex(int i);
};

#endif // UISHADEREDITOR_H
