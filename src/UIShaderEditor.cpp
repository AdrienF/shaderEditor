#include "UIShaderEditor.h"

#include <QFont>
#include <QDir>
#include <QFileDialog>
#include <QPixmap>

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

    //only up to 4 textures for now
    m_textureRep.resize(4);
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


void UIShaderEditor::updateTex(int i)
{
    //open file dialog
    QDir d(documentName());
    QString defaultDir = d.absolutePath();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    defaultDir,
                                                    tr("Image (*.png *.jpg *.jpeg)"));
    if(fileName != m_textureRep.at(i).path)
    {
        ImageRep &rep = m_textureRep[i];
        rep.path = fileName;
        rep.img  = QImage(fileName);
        rep.icon = QIcon(QPixmap::fromImage( rep.img.scaled(QSize(80, 60), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation) ));
        emit updateTexture(i, rep.img);
        switch(i)
        {
        case 0:
            ui->pushButtonTex_0->setIcon(rep.icon);
            ui->pushButtonTex_0->setText("");
            break;
        case 1:
            ui->pushButtonTex_1->setIcon(rep.icon);
            ui->pushButtonTex_1->setText("");
            break;
        case 2:
            ui->pushButtonTex_2->setIcon(rep.icon);
            ui->pushButtonTex_2->setText("");
            break;
        case 3:
            ui->pushButtonTex_3->setIcon(rep.icon);
            ui->pushButtonTex_3->setText("");
            break;
        default :
            break;
        }
    }
}

void UIShaderEditor::on_pushButtonTex_0_pressed(){ updateTex(0); }
void UIShaderEditor::on_pushButtonTex_1_pressed(){ updateTex(1); }
void UIShaderEditor::on_pushButtonTex_2_pressed(){ updateTex(2); }
void UIShaderEditor::on_pushButtonTex_3_pressed(){ updateTex(3); }
