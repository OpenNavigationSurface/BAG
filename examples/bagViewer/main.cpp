#include<QApplication>
#include "BagViewer.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    BagViewer bagViewer;
    bagViewer.resize(640,480);
    bagViewer.show();
    if(argc>1)
        bagViewer.openBag(argv[1]);
    return app.exec();
}
