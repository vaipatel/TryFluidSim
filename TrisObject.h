#ifndef TRISOBJECT_H
#define TRISOBJECT_H

#include <vector>

struct Vertex
{
    float m_pos[3];
    float m_color[4];
    float m_texCoords[2];
};

struct Tri
{
    Vertex m_v0;
    Vertex m_v1;
    Vertex m_v2;
};

class TrisObject
{
public:
    TrisObject(const std::vector<Tri>& _tris);
    ~TrisObject();
    void Draw();
    void CleanUp();

private:
    unsigned int m_vao;
    unsigned int m_vbo;
    size_t m_numTris;
};

#endif // TRISOBJECT_H
