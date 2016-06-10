#ifndef GLCAMERA_H
#define GLCAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QVector2D>
#include "Bounds.h"

class GLCamera
{
    float nearDistance, farDistance;
    QVector3D eyePosition;
    
    float zoom;
    float yaw;
    float pitch;
    
    int viewportWidth;
    int viewportHeight;
    
    QVector3D centerPosition;
    
    float heightExaggeration;

    QMatrix4x4 matrix, modelviewMatrix;
    
    struct Plane
    {
        QVector3D ll,lr,ul,ur;
        Plane(float depth):
        ll(-1.0,-1.0,depth),
        lr(1.0,-1.0,depth),
        ul(-1.0,1.0,depth),
        ur(1.0,1.0,depth)
        {}
    };
    
    Plane nearPlane;
    Plane farPlane;
    Bounds bounds;
    QVector3D eyePositionInScene;
    float pixelSizeOnNearPlane;
    float nearPlaneDistance;
    QVector3D nearPlaneNormal;

    Bounds ndcBounds; // ndc - normalized device coordinates (cube from -1 to 1)
    
    void update()
    {
        modelviewMatrix.setToIdentity();
        modelviewMatrix.translate(eyePosition);
        modelviewMatrix.rotate(-90, 1, 0, 0);
        modelviewMatrix.scale(zoom,zoom,zoom);
        modelviewMatrix.rotate(pitch, 1, 0, 0);
        modelviewMatrix.rotate(yaw, 0, 0, 1);
        modelviewMatrix.scale(1.0,1.0,heightExaggeration);
        modelviewMatrix.translate(-centerPosition.x(),-centerPosition.y(),-centerPosition.z());
        
        matrix.setToIdentity();
        matrix.perspective(60.0f, viewportWidth/float(viewportHeight), nearDistance, farDistance);
        matrix *= modelviewMatrix;
        
        QMatrix4x4 imatrix = matrix.inverted();
        nearPlane = Plane(-1.0);
        nearPlane.ll = imatrix*nearPlane.ll;
        nearPlane.lr = imatrix*nearPlane.lr;
        nearPlane.ul = imatrix*nearPlane.ul;
        nearPlane.ur = imatrix*nearPlane.ur;

        farPlane = Plane(1.0);
        farPlane.ll = imatrix*farPlane.ll;
        farPlane.lr = imatrix*farPlane.lr;
        farPlane.ul = imatrix*farPlane.ul;
        farPlane.ur = imatrix*farPlane.ur;
        
        bounds = Bounds();
        bounds.add(nearPlane.ll);
        bounds.add(nearPlane.lr);
        bounds.add(nearPlane.ul);
        bounds.add(nearPlane.ur);
        bounds.add(farPlane.ll);
        bounds.add(farPlane.lr);
        bounds.add(farPlane.ul);
        bounds.add(farPlane.ur);
        
        QMatrix4x4 imv = genModelviewMatrix().inverted();
        
        eyePositionInScene = imv.column(3).toVector3D();
        //qDebug() << "eye position" << eyePositionInScene;

        QVector3D nearPlaneCenter = imatrix*QVector3D(0.0,0.0,-1.0);
        pixelSizeOnNearPlane = nearPlaneCenter.distanceToPoint(imatrix*QVector3D(1.0/float(viewportWidth),0.0,-1.0));
        
        nearPlaneDistance = nearPlaneCenter.distanceToPoint(eyePositionInScene);
        
        //qDebug() << "near plane center:" << nearPlaneCenter << "\tpixel size:" << pixelSizeOnNearPlane << "near plane distance:" << nearPlaneDistance;
        
        nearPlaneNormal = QVector3D::normal(nearPlane.ll,nearPlane.ul,nearPlane.lr);
    }
        
    
public:
    GLCamera():
        nearDistance(1.0),
        farDistance(100.0),
        eyePosition(0.0, 0.0, -5.0),
        zoom(1.0),
        yaw(0.0),
        pitch(30.0),
        viewportWidth(1),
        viewportHeight(1),
        centerPosition(0.0,0.0,0.0),
        heightExaggeration(1.0),
        nearPlane(-1.0),
        farPlane(1.0),
        ndcBounds(QVector3D(1.0,1.0,1.0),QVector3D(-1.0,-1.0,-1.0))
    {
        update();
    }
    
    QMatrix4x4 const &getMatrix() const {return matrix;}
    
    QMatrix4x4 const &genModelviewMatrix() const {return modelviewMatrix;}
    
    QMatrix4x4 genNormalMatrix()
    {
        QMatrix4x4 ret;
        ret.scale(1.0,1.0,heightExaggeration);
        return ret;
    }
    
    QVector3D getCenterPosition() const {return centerPosition;}
    void setCenterPosition(QVector3D const &p) {centerPosition = p; update();}
    
    float getPitch() const {return pitch;}
    void setPitch(float p) { pitch = std::max(-90.0f,std::min(90.0f,p)); update();}

    float getYaw() const {return yaw;}
    void setYaw(float y) {yaw = y; update();}
    
    float getZoom() const {return zoom;}
    void setZoom(float z) {zoom = z; update();}
    
    float getHeightExaggeration() const {return heightExaggeration;}
    void setHeightExaggeration(float e) {heightExaggeration = std::max(1.0f,std::min(e,500.0f)); update();}
    
    void setViewport(int width, int height) {viewportWidth = width; viewportHeight = height; update();}
    
    // returns -1 if outside view frustum
    float getSizeInPixels(Bounds const &b) const
    {
        if(!b.intersects(bounds))
            return -1.0;

        std::vector<Edge> edges = b.getEdges();
        
        if(b.center().distanceToPlane(nearPlane.ll,nearPlaneNormal)<b.radius())
            edges = trim(edges, nearPlane.ll,nearPlaneNormal);
        
        if(edges.empty())
            return -1.0;
        
        Bounds ndcb;
        
        for(Edge const &e: edges)
        {
            ndcb.add(matrix*e.p1);
            ndcb.add(matrix*e.p2);
        }
        
        if(!ndcBounds.intersects(ndcb))
            return -1.0;
        
        QVector3D s = ndcb.size();
        return std::max(s.x()*viewportWidth,s.y()*viewportHeight);
    }
};

#endif