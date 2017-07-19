/*
    Copyright (C) 2016 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ShaderEditor.h"

#include <KF5/KSyntaxHighlighting/definition.h>
#include <KF5/KSyntaxHighlighting/foldingregion.h>
#include <KF5/KSyntaxHighlighting/syntaxhighlighter.h>
#include <KF5/KSyntaxHighlighting/theme.h>

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QMenu>
#include <QPainter>
#include <QPalette>

#include <QKeyEvent>

class CodeEditorSidebar : public QWidget
{
    Q_OBJECT
public:
    explicit CodeEditorSidebar(ShaderEditor *editor);
    QSize sizeHint() const Q_DECL_OVERRIDE;

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    ShaderEditor *m_codeEditor;
};

CodeEditorSidebar::CodeEditorSidebar(ShaderEditor *editor) :
    QWidget(editor),
    m_codeEditor(editor)
{
}

QSize CodeEditorSidebar::sizeHint() const
{
    return QSize(m_codeEditor->sidebarWidth(), 0);
}

void CodeEditorSidebar::paintEvent(QPaintEvent *event)
{
    m_codeEditor->sidebarPaintEvent(event);
}

void CodeEditorSidebar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->x() >= width() - m_codeEditor->fontMetrics().lineSpacing()) {
        auto block = m_codeEditor->blockAtPosition(event->y());
        if (!block.isValid() || !m_codeEditor->isFoldable(block))
            return;
        m_codeEditor->toggleFold(block);
    }
    QWidget::mouseReleaseEvent(event);
}


ShaderEditor::ShaderEditor(QWidget *parent) :
    QPlainTextEdit(parent),
    m_highlighter(new KSyntaxHighlighting::SyntaxHighlighter(document())),
    m_sideBar(new CodeEditorSidebar(this))
{
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    setTheme((palette().color(QPalette::Base).lightness() < 128)
        ? m_repository.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme)
        : m_repository.defaultTheme(KSyntaxHighlighting::Repository::LightTheme));

    connect(this, &QPlainTextEdit::blockCountChanged, this, &ShaderEditor::updateSidebarGeometry);
    connect(this, &QPlainTextEdit::updateRequest, this, &ShaderEditor::updateSidebarArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &ShaderEditor::highlightLines);

    updateSidebarGeometry();
    highlightLines();
}

ShaderEditor::~ShaderEditor()
{
}

void ShaderEditor::openFile(const QString& fileName)
{
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly)) {
        qWarning() << "Failed to open" << fileName << ":" << f.errorString();
        return;
    }

    clear();

    setPlainText(QString::fromUtf8(f.readAll()));
    f.close();

    const auto def = m_repository.definitionForName("GLSL");
    m_highlighter->setDefinition(def);
    m_errorLines.clear();

    //rebuild program
    emit requestShaderValidation(document()->toPlainText());
}

void ShaderEditor::setDefinition(const QString &defName)
{
    const auto def = m_repository.definitionForName(defName);
    m_highlighter->setDefinition(def);
}

void ShaderEditor::setErrorLines(QVector<GlslErrorManager::ErrorLog> errLines)
{
    m_errorLines = errLines;
    highlightLines();
}

void ShaderEditor::contextMenuEvent(QContextMenuEvent *event)
{
    auto menu = createStandardContextMenu(event->pos());
    menu->addSeparator();
    auto openAction = menu->addAction(QStringLiteral("Open File..."));
    connect(openAction, &QAction::triggered, this, [this]() {
        const auto fileName = QFileDialog::getOpenFileName(this, QStringLiteral("Open File"));
        if (!fileName.isEmpty())
            openFile(fileName);
    });

    // syntax selection
    if(0){
        auto hlActionGroup = new QActionGroup(menu);
        hlActionGroup->setExclusive(true);
        auto hlGroupMenu = menu->addMenu(QStringLiteral("Syntax"));
        auto noHlAction = hlGroupMenu->addAction(QStringLiteral("None"));
        noHlAction->setCheckable(true);
        hlActionGroup->addAction(noHlAction);
        noHlAction->setChecked(!m_highlighter->definition().isValid());
        QMenu *hlSubMenu = nullptr;
        QString currentGroup;
        foreach (const auto &def, m_repository.definitions()) {
            if (def.isHidden())
                continue;
            if (currentGroup != def.section()) {
                currentGroup = def.section();
                hlSubMenu = hlGroupMenu->addMenu(def.translatedSection());
            }

            Q_ASSERT(hlSubMenu);
            auto action = hlSubMenu->addAction(def.translatedName());
            action->setCheckable(true);
            action->setData(def.name());
            hlActionGroup->addAction(action);
            if (def.name() == m_highlighter->definition().name())
                action->setChecked(true);
        }
        connect(hlActionGroup, &QActionGroup::triggered, this, [this](QAction *action) {
            const auto defName = action->data().toString();
            const auto def = m_repository.definitionForName(defName);
            m_highlighter->setDefinition(def);
        });
    }

    // theme selection
    auto themeGroup = new QActionGroup(menu);
    themeGroup->setExclusive(true);
    auto themeMenu = menu->addMenu(QStringLiteral("Theme"));
    foreach (const auto &theme, m_repository.themes()) {
        auto action = themeMenu->addAction(theme.translatedName());
        action->setCheckable(true);
        action->setData(theme.name());
        themeGroup->addAction(action);
        if (theme.name() == m_highlighter->theme().name())
            action->setChecked(true);
    }
    connect(themeGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        const auto themeName = action->data().toString();
        const auto theme = m_repository.theme(themeName);
        setTheme(theme);
    });

    menu->exec(event->globalPos());
    delete menu;
}

void ShaderEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    updateSidebarGeometry();
}

void ShaderEditor::keyPressEvent(QKeyEvent *event)
{
    if(Qt::ControlModifier & event->modifiers())
    {
        switch(event->key())
        {
        case Qt::Key_B:
            m_errorLines.clear();
            highlightLines();
            emit requestShaderValidation(document()->toPlainText());
            break;
        case Qt::Key_S:
            if(Qt::ShiftModifier & event->modifiers())
            {
                emit saveDocumentAs();
            }else{
                emit saveDocument();
            }
            break;
        case Qt::Key_O:
            emit openNewDocument();
            break;

        case Qt::Key_Slash:
            commentLines();
            break;
        default:
            QPlainTextEdit::keyPressEvent(event);
            break;
        }
    }
    else
        QPlainTextEdit::keyPressEvent(event);
}

void ShaderEditor::setTheme(const KSyntaxHighlighting::Theme &theme)
{
    auto pal = qApp->palette();
    if (theme.isValid()) {
        pal.setColor(QPalette::Base, theme.editorColor(KSyntaxHighlighting::Theme::BackgroundColor));
        pal.setColor(QPalette::Text, theme.textColor(KSyntaxHighlighting::Theme::Normal));
        pal.setColor(QPalette::Highlight, theme.editorColor(KSyntaxHighlighting::Theme::TextSelection));
    }
    setPalette(pal);

    m_highlighter->setTheme(theme);
    m_highlighter->rehighlight();
    highlightLines();
}

int ShaderEditor::sidebarWidth() const
{
    int digits = 1;
    auto count = blockCount();
    while (count >= 10) {
        ++digits;
        count /= 10;
    }
    return 4 + fontMetrics().width(QLatin1Char('9')) * digits + fontMetrics().lineSpacing();
}

void ShaderEditor::sidebarPaintEvent(QPaintEvent *event)
{
    QPainter painter(m_sideBar);
    painter.fillRect(event->rect(), m_highlighter->theme().editorColor(KSyntaxHighlighting::Theme::IconBorder));

    auto block = firstVisibleBlock();
    auto blockNumber = block.blockNumber();
    int top = blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + blockBoundingRect(block).height();
    const int currentBlockNumber = textCursor().blockNumber();

    const auto foldingMarkerSize = fontMetrics().lineSpacing();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const auto number = QString::number(blockNumber + 1);
            painter.setPen(m_highlighter->theme().editorColor(
                (blockNumber == currentBlockNumber) ? KSyntaxHighlighting::Theme::CurrentLineNumber
                                                    : KSyntaxHighlighting::Theme::LineNumbers));
            painter.drawText(0, top, m_sideBar->width() - 2 - foldingMarkerSize, fontMetrics().height(), Qt::AlignRight, number);
        }

        // folding marker
        if (block.isVisible() && isFoldable(block)) {
            QPolygonF polygon;
            if (isFolded(block)) {
                polygon << QPointF(foldingMarkerSize * 0.4, foldingMarkerSize * 0.25);
                polygon << QPointF(foldingMarkerSize * 0.4, foldingMarkerSize * 0.75);
                polygon << QPointF(foldingMarkerSize * 0.8, foldingMarkerSize * 0.5);
            } else {
                polygon << QPointF(foldingMarkerSize * 0.25, foldingMarkerSize * 0.4);
                polygon << QPointF(foldingMarkerSize * 0.75, foldingMarkerSize * 0.4);
                polygon << QPointF(foldingMarkerSize * 0.5, foldingMarkerSize * 0.8);
            }
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(m_highlighter->theme().editorColor(KSyntaxHighlighting::Theme::CodeFolding)));
            painter.translate(m_sideBar->width() - foldingMarkerSize, top);
            painter.drawPolygon(polygon);
            painter.restore();
        }

        block = block.next();
        top = bottom;
        bottom = top + blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void ShaderEditor::updateSidebarGeometry()
{
    setViewportMargins(sidebarWidth(), 0, 0, 0);
    const auto r = contentsRect();
    m_sideBar->setGeometry(QRect(r.left(), r.top(), sidebarWidth(), r.height()));
}

void ShaderEditor::updateSidebarArea(const QRect& rect, int dy)
{
    if (dy)
        m_sideBar->scroll(0, dy);
    else
        m_sideBar->update(0, rect.y(), m_sideBar->width(), rect.height());
}

void ShaderEditor::highlightLines()
{
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QColor(m_highlighter->theme().editorColor(KSyntaxHighlighting::Theme::CurrentLine)));
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();

    QList<QTextEdit::ExtraSelection> extraSelections ;
    extraSelections.append(selection);
    if(!m_errorLines.isEmpty())
        extraSelections.append(highlightErrorLines());

    setExtraSelections(extraSelections);
}

QList<QTextEdit::ExtraSelection> ShaderEditor::highlightErrorLines()
{
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QColor(m_highlighter->theme().editorColor(KSyntaxHighlighting::Theme::MarkError)));
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);

    QList<QTextEdit::ExtraSelection> extraSelections;
    QTextCursor cursor = textCursor();

    //for each error line, set the cursor and append to the selection;
    for(const auto& errLine : m_errorLines)
    {
        //reset position
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, errLine.row - 1);

        selection.cursor = cursor;
        selection.cursor.clearSelection();

        extraSelections.append(selection);
    }

    return extraSelections;
}

void ShaderEditor::commentLines()
{
    QTextCursor currentCursor = textCursor();
    currentCursor.movePosition(QTextCursor::StartOfLine);
    //select the first word of the line
    currentCursor.select(QTextCursor::WordUnderCursor);
    QString textAtCursor = currentCursor.selectedText();
    currentCursor.clearSelection();

    //toggle the comment
    if(textAtCursor.left(2) == "//")
    {
        //select the first two characters of the line
        currentCursor.movePosition(QTextCursor::StartOfLine);
        currentCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
        currentCursor.removeSelectedText();
    }else{
        currentCursor.movePosition(QTextCursor::StartOfLine);
        currentCursor.insertText("//");
    }
}

QTextBlock ShaderEditor::blockAtPosition(int y) const
{
    auto block = firstVisibleBlock();
    if (!block.isValid())
        return QTextBlock();

    int top = blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + blockBoundingRect(block).height();
    do {
        if (top <= y && y <= bottom)
            return block;
        block = block.next();
        top = bottom;
        bottom = top + blockBoundingRect(block).height();
    } while (block.isValid());
    return QTextBlock();
}

bool ShaderEditor::isFoldable(const QTextBlock &block) const
{
    return m_highlighter->startsFoldingRegion(block);
}

bool ShaderEditor::isFolded(const QTextBlock &block) const
{
    if (!block.isValid())
        return false;
    const auto nextBlock = block.next();
    if (!nextBlock.isValid())
        return false;
    return !nextBlock.isVisible();
}

void ShaderEditor::toggleFold(const QTextBlock &startBlock)
{
    // we also want to fold the last line of the region, therefore the ".next()"
    const auto endBlock = m_highlighter->findFoldingRegionEnd(startBlock).next();

    if (isFolded(startBlock)) {
        // unfold
        auto block = startBlock.next();
        while (block.isValid() && !block.isVisible()) {
            block.setVisible(true);
            block.setLineCount(block.layout()->lineCount());
            block = block.next();
        }

    } else {
        // fold
        auto block = startBlock.next();
        while (block.isValid() && block != endBlock) {
            block.setVisible(false);
            block.setLineCount(0);
            block = block.next();
        }
    }

    // redraw document
    document()->markContentsDirty(startBlock.position(), endBlock.position() - startBlock.position() + 1);

    // update scrollbars
    emit document()->documentLayout()->documentSizeChanged(document()->documentLayout()->documentSize());
}

#include "ShaderEditor.moc"
