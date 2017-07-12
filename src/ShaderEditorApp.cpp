#include "ShaderEditorApp.h"
#include <QIcon>
#include <QFileDialog>

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

    connect(m_editor.get(), SIGNAL(saveDocument()),
            this, SLOT(saveDocument()));
    connect(m_editor.get(), SIGNAL(saveDocumentAs()),
            this, SLOT(saveDocumentAs()));
    connect(m_editor.get(), &ShaderEditor::openNewDocument,
            [this](){
        QDir d(this->m_editor->documentName());
        QString defaultDir = d.absolutePath();
        QString fileName = QFileDialog::getOpenFileName(m_editor.get(), tr("Open File"),
                                     defaultDir,
                                     tr("Fragment shader (*.fsh *.glsl *.frag)"));
        m_editor->openFile(fileName);
    });

    QSurfaceFormat format;
    format.setSamples(16);
    m_renderer->setFormat(format);
    m_renderer->resize(640, 480);
    m_renderer->setAnimating(true);
}

void ShaderEditorApp::openDocument(const QString &filename)
{
    m_editor->openFile(filename);
    m_editor->show();

    m_renderer->show();
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
    f.write(m_editor->document()->toPlainText().toStdString().c_str());
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
    f.write(m_editor->document()->toPlainText().toStdString().c_str());
    f.close();

    if(fileName != m_editor->documentName())
        m_editor->setDocumentName(fileName);
}
