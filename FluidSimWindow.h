#ifndef FLUIDSIMWINDOW_H
#define FLUIDSIMWINDOW_H

#include "OpenGLWindow.h"

class QOpenGLShaderProgram;
class QWindow;

class FluidSimWindow : public OpenGLWindow
{
public:
    FluidSimWindow(QWindow* _parent = nullptr);

    void initialize() override;
    void render() override;

private:
    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

    const QString targetFBOVertexShaderSource =
            "#version 330 core\n"
        "in highp vec4 posAttr;\n"
        "in lowp vec4 colAttr;\n"
        "out lowp vec4 col;\n"
        "uniform highp mat4 matrix;\n"
        "void main() {\n"
        "   col = colAttr;\n"
        "   gl_Position = matrix * posAttr;\n"
        "}\n";

    const QString targetFBOFragmentShaderSource =
            "#version 330 core\n"
        "in vec4 col;\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = col;\n"
        "}\n";

    GLuint m_screenTextureLoc;

    const QString screenVertexShaderSource =
            "#version 330 core\n"
            "layout (location = 0) in vec2 aPos;\n"
            "layout (location = 1) in vec2 aTexCoords;\n"
            "\n"
            "out vec2 TexCoords;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
            "    TexCoords = aTexCoords;\n"
            "}";

    const QString screenFragmentShaderSource =
            "#version 330 core\n"
            "out vec4 FragColor;\n"
            "in vec2 TexCoords;\n"
            "uniform sampler2D screenTexture;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    FragColor = texture(screenTexture, TexCoords);//vec4(TexCoords.x, TexCoords.y, 0, 1);\n"
            "}";

    void DrawTargetFBO();

    QOpenGLShaderProgram* m_targetFBOProgram; //!< Need one of these for each step of fluid sim
    QOpenGLShaderProgram* m_screenProgram;
    uint m_frame;

};

#endif // FLUIDSIMWINDOW_H
