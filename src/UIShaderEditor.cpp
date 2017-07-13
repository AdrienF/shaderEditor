#include "UIShaderEditor.h"

#include <QFont>

#include "ShaderEditor.h"
#include "ui_UIShaderEditor.h"

UIShaderEditor::UIShaderEditor(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::UIShaderEditor)
{
    ui->setupUi(this);
    ui->listWidgetErrLog->setSelectionMode( QAbstractItemView::SingleSelection );
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->labelFPS->setFont(fixedFont);
    ui->labelGlobalTime->setFont(fixedFont);

    connect(ui->editor, &ShaderEditor::requestShaderValidation, this, &UIShaderEditor::requestShaderValidation);
    connect(ui->pushButtonBuild, &QPushButton::pressed,         [this](){
        emit requestShaderValidation(this->editor()->document()->toPlainText());
    });
}

UIShaderEditor::~UIShaderEditor()
{
    delete ui;
}


ShaderEditor *UIShaderEditor::editor()
{
    return ui->editor;
}

void UIShaderEditor::setDocumentName(const QString& docName)
{
    m_filename = docName;
    setWindowTitle(m_filename);
}

void UIShaderEditor::openFile(const QString& fileName)
{
   setDocumentName(fileName);
   ui->editor->openFile(fileName);
   m_errorLines.clear();
   updateErrLog();
}

void UIShaderEditor::setErrorLines(QVector<RenderWidget::ErrorLog> errLines)
{
    m_errorLines = errLines;
    ui->editor->setErrorLines(errLines);
    updateErrLog();
}

void UIShaderEditor::updateErrLog()
{
  ui->listWidgetErrLog->clear();
  for(const auto &errLog : m_errorLines)
  {
      QString message = QString("Line %1 : %2")
              .arg(errLog.row)
              .arg(errLog.log);
      ui->listWidgetErrLog->addItem(message);
  }
  ui->listWidgetErrLog->setVisible(!m_errorLines.isEmpty());
}


void UIShaderEditor::updateFPS(float val)
{
    ui->labelFPS->setText(QString("%1 fps").arg(QString::number(val, 'f', 2)));
}

void UIShaderEditor::updateGlobalTime(float val)
{
    ui->labelGlobalTime->setText(QString("%1s elapsed").arg(QString::number(val, 'f', 2)));
}
