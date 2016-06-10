#ifndef EDGE_H
#define EDGE_H

#include <QVector3D>
#include <memory>

struct Edge
{
    QVector3D p1,p2;
    Edge(QVector3D const &p1, QVector3D const &p2):p1(p1),p2(p2){}
};

inline std::vector<Edge> trim(std::vector<Edge> const &edges, QVector3D const &p0, QVector3D const &n)
{
    
    std::vector<Edge> ret;
    for(Edge const &e: edges)
    {
        float p1d = e.p1.distanceToPlane(p0,n);
        float p2d = e.p2.distanceToPlane(p0,n);
        if(p1d >= 0.0 && p2d >= 0.0)
            ret.push_back(e);
        else if(p1d >= 0.0 || p2d >= 0.0)
        {
            // https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
            // plane: (p-p0)dot n = 0
            //        n is normal vector to plane
            //        p0 is point on plane
            // line: p = dl+l0
            //       l is vector in direction of line
            //       l0 is point on line
            //       d is scalar in real number domain
            //
            // d = ((p0-l0)dot n)/(l dot n)
            QVector3D l = e.p2-e.p1;
            QVector3D l0 = e.p1;
            float d = QVector3D::dotProduct(p0-l0,n)/QVector3D::dotProduct(l,n);
            QVector3D p = d*l+l0;
            if(p1d >= 0.0)
                ret.push_back(Edge(e.p1,p));
            else
                ret.push_back(Edge(e.p2,p));
        }
    }
    return ret;
}


#endif