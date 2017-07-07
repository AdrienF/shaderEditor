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

#include "ShaderEditorApp.h"

#include <QCommandLineParser>

int main(int argc, char **argv)
{
    ShaderEditorApp app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("source"), QStringLiteral("The source file to highlight."));
//    parser.process(app);

    QString fileToOpen;
    if (parser.positionalArguments().size() == 1)
        fileToOpen = parser.positionalArguments().at(0);
    else
        fileToOpen = ":/Shaders/basicShader.fsh";

    app.openDocument(fileToOpen);

    return app.exec();
}
