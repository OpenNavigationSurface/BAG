#ifndef BAGVIEWER_H
#define BAGVIEWER_H

#include <QMainWindow>

class QActionGroup;
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
    void openBag(std::string const &fname);
private:
    Ui::BagViewer *ui;
    QActionGroup *colormapActionGroup;
    QActionGroup *drawStyleActionGroup;
    
private slots:
    void on_actionOpen_triggered();
    void on_actionExit_triggered();
    void on_actionTopographic_triggered();
    void on_actionOmnimap_triggered();
    void on_actionSolid_triggered();
    void on_actionWireframe_triggered();
    void on_actionPoints_triggered();
};    

#endif
