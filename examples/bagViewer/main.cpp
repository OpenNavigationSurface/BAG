#include<QApplication>
#include "BagViewer.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    BagViewer bagViewer;
    bagViewer.resize(640,480);
    bagViewer.show();
    return app.exec();
}
