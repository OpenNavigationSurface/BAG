#include<QApplication>
#include<QSurfaceFormat>
#include "BagViewer.h"
#include <QDebug>

int main(int argc, char **argv)
{
    QSurfaceFormat format;
    format.setSamples(16);
    format.setDepthBufferSize(32);
    format.setVersion(4,0);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    //QSurfaceFormat::setDefaultFormat(format);
    
    QApplication app(argc, argv);
    BagViewer bagViewer;
    bagViewer.resize(640,480);
    bagViewer.show();
    if(argc>1)
        bagViewer.openBag(argv[1]);
    return app.exec();
}
