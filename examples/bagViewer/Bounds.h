#ifndef BOUNDS_H
#define BOUNDS_H

#include <QVector3D>
#include <Edge.h>

// Combination of bounding box and sphere
class Bounds
{
    QVector3D lowerBottomLeft,upperTopRight;
    
public:
    Bounds():upperTopRight(-1.0,-1.0,-1.0) {}
    Bounds(Bounds const &o):lowerBottomLeft(o.lowerBottomLeft),upperTopRight(o.upperTopRight){}
    Bounds(QVector3D const &p):lowerBottomLeft(p),upperTopRight(p){}
    Bounds(QVector3D const &p1,QVector3D const &p2):lowerBottomLeft(p1),upperTopRight(p1){add(p2);}
    
    void add(QVector3D const &p)
    {
        if(!valid())
        {
            lowerBottomLeft = p;
            upperTopRight = p;
        }
        else
        {
            lowerBottomLeft.setX(std::min(lowerBottomLeft.x(),p.x()));
            lowerBottomLeft.setY(std::min(lowerBottomLeft.y(),p.y()));
            lowerBottomLeft.setZ(std::min(lowerBottomLeft.z(),p.z()));
            upperTopRight.setX(std::max(upperTopRight.x(),p.x()));
            upperTopRight.setY(std::max(upperTopRight.y(),p.y()));
            upperTopRight.setZ(std::max(upperTopRight.z(),p.z()));
        }
    }
    
    bool valid() const { return lowerBottomLeft.x()<=upperTopRight.x() && lowerBottomLeft.y()<=upperTopRight.y() && lowerBottomLeft.z()<=upperTopRight.z(); }
    QVector3D center() const {Q_ASSERT(valid()); return lowerBottomLeft+((upperTopRight-lowerBottomLeft)/2.0);}
    float radius() const {Q_ASSERT(valid()); return lowerBottomLeft.distanceToPoint(upperTopRight)/2.0;}
    
    QVector3D const &min() const {Q_ASSERT(valid()); return lowerBottomLeft;}
    QVector3D const &max() const {Q_ASSERT(valid()); return upperTopRight;}
    
    QVector3D size() const {Q_ASSERT(valid()); return upperTopRight-lowerBottomLeft;}
    
    bool contains(QVector3D const &p) const 
    {
        Q_ASSERT(valid());
        return !(p.x() < lowerBottomLeft.x() || p.x() > upperTopRight.x()
              || p.y() < lowerBottomLeft.y() || p.y() > upperTopRight.y()
              || p.z() < lowerBottomLeft.z() || p.z() > upperTopRight.z());
    }
    
    bool sphericallyIntersects(Bounds const &o) const 
    {
        Q_ASSERT(valid());
        Q_ASSERT(o.valid());
        return center().distanceToPoint(o.center()) <= radius()+o.radius();
    }
    
    bool intersects(Bounds const &o) const
    {
        Q_ASSERT(valid());
        Q_ASSERT(o.valid());
        return !(lowerBottomLeft.x() > o.upperTopRight.x() || upperTopRight.x() < o.lowerBottomLeft.x()
              || lowerBottomLeft.y() > o.upperTopRight.y() || upperTopRight.y() < o.lowerBottomLeft.y()
              || lowerBottomLeft.z() > o.upperTopRight.z() || upperTopRight.z() < o.lowerBottomLeft.z());
    }
    
    std::vector<Edge> getEdges() const
    {
        Q_ASSERT(valid());
        QVector3D p000 = QVector3D(min().x(),min().y(),min().z());
        QVector3D p001 = QVector3D(min().x(),min().y(),max().z());
        QVector3D p010 = QVector3D(min().x(),max().y(),min().z());
        QVector3D p011 = QVector3D(min().x(),max().y(),max().z());
        QVector3D p100 = QVector3D(max().x(),min().y(),min().z());
        QVector3D p101 = QVector3D(max().x(),min().y(),max().z());
        QVector3D p110 = QVector3D(max().x(),max().y(),min().z());
        QVector3D p111 = QVector3D(max().x(),max().y(),max().z());
        
        std::vector<Edge> ret;
        ret.push_back(Edge(p000,p001));
        ret.push_back(Edge(p000,p010));
        ret.push_back(Edge(p000,p100));
        ret.push_back(Edge(p111,p110));
        ret.push_back(Edge(p111,p101));
        ret.push_back(Edge(p111,p011));
        ret.push_back(Edge(p001,p101));
        ret.push_back(Edge(p001,p011));
        ret.push_back(Edge(p010,p011));
        ret.push_back(Edge(p010,p110));
        ret.push_back(Edge(p100,p110));
        ret.push_back(Edge(p100,p101));
        
        return ret;
    }
};

#endif