#include "ShaderEditorApp.h"
#include <QIcon>
#include <QFileDialog>

#include "ShaderEditor.h"
#include "RenderWidget.h"

ShaderEditorApp::ShaderEditorApp(int argc, char **argv)
    : QApplication(argc, argv)
    , m_renderer(new RenderWidget())
    , m_editor(new UIShaderEditor())
{

    QIcon icon(":/Icons/croissant.png");
    setWindowIcon(icon);

    ShaderEditor *actualEditor = m_editor->editor();

    connect(m_editor.get(), &UIShaderEditor::requestShaderValidation,  m_renderer.get(), &RenderWidget::rebuildShader);
    connect(actualEditor, &ShaderEditor::saveDocument,      this, &ShaderEditorApp::saveDocument);
    connect(actualEditor, &ShaderEditor::saveDocumentAs,    this, &ShaderEditorApp::saveDocumentAs);
    connect(actualEditor, &ShaderEditor::openNewDocument,   [this](){
        QDir d(this->m_editor->documentName());
        QString defaultDir = d.absolutePath();
        QString fileName = QFileDialog::getOpenFileName(this->m_editor->editor(), tr("Open File"),
                                                        defaultDir,
                                                        tr("Fragment shader (*.fsh *.glsl *.frag)"));
        this->m_editor->openFile(fileName);
    });
    connect(m_renderer.get(), &RenderWidget::buildFailed,   [this](){
        this->m_editor->setErrorLines(this->m_renderer->parseLog());
    });
    connect(m_renderer.get(), &RenderWidget::buildSuccess,   [this](){
        this->m_editor->setErrorLines(this->m_renderer->parseLog());
    });
    connect(m_renderer.get(), &RenderWidget::sendFPS,           m_editor.get(), &UIShaderEditor::updateFPS);
    connect(m_renderer.get(), &RenderWidget::sendGlobalTime,    m_editor.get(), &UIShaderEditor::updateGlobalTime);

    QSurfaceFormat format;
    format.setSamples(16);
    m_renderer->setFormat(format);
    m_renderer->resize(640, 480);
    m_renderer->setAnimating(true);

    m_editor->show();
    m_renderer->show();
}

void ShaderEditorApp::openDocument(const QString &filename)
{
    m_editor->openFile(filename);
}


void ShaderEditorApp::saveDocument()
{
    const QString filename = m_editor->documentName();

    //sepcial case of resource file
    if(filename.at(0)==':')
        return saveDocumentAs();

    QFile f(filename);
    if (!f.open(QFile::WriteOnly)) {
        qWarning() << "Failed to save" << filename << ":" << f.errorString();
        return;
    }
    f.write(m_editor->editor()->document()->toPlainText().toStdString().c_str());
    f.close();
}

void ShaderEditorApp::saveDocumentAs()
{
    const QString filename = m_editor->documentName();
    QString defaultDir;
    if(filename.at(0)!=':'){
        QDir d(m_editor->documentName());
        defaultDir = d.absolutePath();
    }else{
        defaultDir = QDir::homePath();
    }

    QString fileName = QFileDialog::getSaveFileName(0, tr("Save File"),
                                 defaultDir,
                                 tr("Fragment shader (*.fsh *.glsl *.frag)"));

    QFile f(fileName);
    if (!f.open(QFile::WriteOnly)) {
        qWarning() << "Failed to save" << fileName << ":" << f.errorString();
        return;
    }
    f.write(m_editor->editor()->document()->toPlainText().toStdString().c_str());
    f.close();

    if(fileName != m_editor->documentName())
        m_editor->setDocumentName(fileName);
}
