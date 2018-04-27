#include "BagViewer.h"
#include "ui_BagViewer.h"
#include "BagGL.h"

#include <QFileDialog>
#include <QMessageBox>

BagViewer::BagViewer(QWidget* parent): QMainWindow(parent), ui(new Ui::BagViewer)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon(QString(":/icons/bagViewer.png")));

    colormapActionGroup = new QActionGroup(this);
    colormapActionGroup->addAction(ui->actionTopographic);
    colormapActionGroup->addAction(ui->actionOmnimap);
    
    drawStyleActionGroup = new QActionGroup(this);
    drawStyleActionGroup->addAction(ui->actionSolid);
    drawStyleActionGroup->addAction(ui->actionWireframe);
    drawStyleActionGroup->addAction(ui->actionPoints);
    
    ui->bagGL->setStatusBar(ui->statusbar);
}

BagViewer::~BagViewer()
{
    delete ui;
}

void BagViewer::openBag(const std::string& fname)
{
    ui->bagGL->openBag(QString(fname.c_str()));
}


void BagViewer::on_actionExit_triggered()
{
    qApp->quit();
}

void BagViewer::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), QString(),
                                                    tr("Bag Files (*.bag);;All files (*)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("Could not open file"));
            return;
        }
        if(!ui->bagGL->openBag(fileName))
            QMessageBox::critical(this, tr("Error"), tr("Could not open bag file"));
    }
}

void BagViewer::on_actionTopographic_triggered()
{
    ui->bagGL->setColormap("topographic");
}

void BagViewer::on_actionOmnimap_triggered()
{
    ui->bagGL->setColormap("omnimap");
}

void BagViewer::on_actionSolid_triggered()
{
    ui->bagGL->setDrawStyle("solid");
}

void BagViewer::on_actionWireframe_triggered()
{
    ui->bagGL->setDrawStyle("wireframe");
}

void BagViewer::on_actionPoints_triggered()
{
    ui->bagGL->setDrawStyle("points");
}

