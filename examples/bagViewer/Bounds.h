#ifndef BOUNDS_H
#define BOUNDS_H

#include <QVector3D>

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
    
    QVector3D min() const {Q_ASSERT(valid()); return lowerBottomLeft;}
    QVector3D max() const {Q_ASSERT(valid()); return upperTopRight;}
    
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
};

#endif