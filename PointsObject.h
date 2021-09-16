#ifndef POINTSOBJECT_H
#define POINTSOBJECT_H

#include <vector>

struct Point
{
    float m_pos[3];
};

class PointsObject
{
public:
    PointsObject(const std::vector<Point>& _points);
    ~PointsObject();
    void Draw();
    void CleanUp();

private:
    unsigned int m_vao;
    unsigned int m_vbo;
    size_t m_numPoints;
};

#endif // POINTSOBJECT_H
