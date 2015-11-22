#ifndef BAGVIEWER_H
#define BAGVIEWER_H

#include <QMainWindow>

class BagGL;
namespace Ui {
    class BagViewer;
}

class BagViewer: public QMainWindow
{
    Q_OBJECT
public:
    explicit BagViewer(QWidget *parent=0);
    ~BagViewer();
private:
    BagGL *bagGL;
    Ui::BagViewer *ui;
    
private slots:
    void on_actionOpen_triggered();
    void on_actionExit_triggered();
};    

#endif