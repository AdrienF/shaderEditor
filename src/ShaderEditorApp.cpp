#include "ShaderEditorApp.h"
#include <QIcon>

#include "ShaderEditor.h"
#include "RenderWidget.h"

ShaderEditorApp::ShaderEditorApp(int argc, char **argv)
    : QApplication(argc, argv)
    , m_editor(new ShaderEditor())
    , m_renderer(new RenderWidget())
{

    QIcon icon(":/Icons/croissant.png");
    setWindowIcon(icon);

    m_editor->resize(512, 1024);

    connect(m_editor.get(), SIGNAL(requestShaderValidation(QString)),
            static_cast<RenderWidget*>(m_renderer.get()), SLOT(rebuildShader(QString)));

    QSurfaceFormat format;
    format.setSamples(16);
    m_renderer->setFormat(format);
    m_renderer->resize(640, 480);
    m_renderer->setAnimating(true);
}

void ShaderEditorApp::openDocument(const QString &filename)
{
    m_editor->openFile(filename);
    m_editor->setDefinition("GLSL");
    m_editor->show();

    m_renderer->show();
}
