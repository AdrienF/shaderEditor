#ifndef SHADEREDITORAPP_H
#define SHADEREDITORAPP_H

#include <memory>
#include <QApplication>

#include "ShaderEditor.h"
#include "RenderWidget.h"
#include "UIShaderEditor.h"

class ShaderEditorApp : public QApplication
{
    Q_OBJECT
public:
    ShaderEditorApp(int argc, char **argv);

    void openDocument(const QString &filename);

private slots:
    void saveDocument();
    void saveDocumentAs();

private:
    std::unique_ptr<RenderWidget> m_renderer;
    std::unique_ptr<UIShaderEditor> m_editor;
};

#endif // SHADEREDITORAPP_H
