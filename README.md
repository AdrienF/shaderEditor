# shaderEditor

The shaderEditor app is a desktop application to create and edit GLSL fragment shaders.
It is composed of a code editor with syntax highlighting and error reporting + an opengl viewer window.

## Dependencies

Curently this projects depends on :
* KSyntaxHighlingthing : https://api.kde.org/frameworks/syntax-highlighting/html/index.html
* which depends on the Extra CMake Modules (ECM) : https://github.com/KDE/extra-cmake-modules

## Running the program

If everything goes well you should have the editor loaded with a basic fragment shader writing colors depending on the normalized pixel position in the window.
