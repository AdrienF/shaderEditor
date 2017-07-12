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


#ifndef SHADEREDITOR_H
#define SHADEREDITOR_H

#include <QPlainTextEdit>
#include <KF5/KSyntaxHighlighting/repository.h>

namespace KSyntaxHighlighting {
class SyntaxHighlighter;
}
class CodeEditorSidebar;

class ShaderEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit ShaderEditor(QWidget *parent=NULL);
    ~ShaderEditor();

    void openFile(const QString &fileName);
    void setDocumentName(const QString& docName);
    const QString& documentName() const {return m_filename;}

    void setDefinition(const QString &defName);

signals:
    void requestShaderValidation(QString);
    void saveDocument();
    void saveDocumentAs();
    void openNewDocument();

protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

private:
    friend class CodeEditorSidebar;
    void setTheme(const KSyntaxHighlighting::Theme &theme);
    int sidebarWidth() const;
    void sidebarPaintEvent(QPaintEvent *event);
    void updateSidebarGeometry();
    void updateSidebarArea(const QRect &rect, int dy);
    void highlightCurrentLine();
    void highlightErrorLines(QVector<int>);


    QTextBlock blockAtPosition(int y) const;
    bool isFoldable(const QTextBlock &block) const;
    bool isFolded(const QTextBlock &block) const;
    void toggleFold(const QTextBlock &block);

     KSyntaxHighlighting::Repository m_repository;
     KSyntaxHighlighting::SyntaxHighlighter *m_highlighter;
     CodeEditorSidebar *m_sideBar;
     QString m_filename;
};

#endif // SHADEREDITOR_H
