#include "BagViewer.h"
#include "ui_BagViewer.h"
#include "BagGL.h"

#include <QFileDialog>
#include <QMessageBox>

BagViewer::BagViewer(QWidget* parent): QMainWindow(parent),
    bagGL(new BagGL), ui(new Ui::BagViewer)
{
    ui->setupUi(this);
    colormapActionGroup = new QActionGroup(this);
    colormapActionGroup->addAction(ui->actionTopographic);
    colormapActionGroup->addAction(ui->actionOmnimap);
    
    QSurfaceFormat format;
    //format.setSamples(16);
    format.setDepthBufferSize(32);
    
    bagGL->setFormat(format);
    
    bagGL->setAnimating(true);
    
    QWidget *container = QWidget::createWindowContainer(bagGL);
    setCentralWidget(container);
}

BagViewer::~BagViewer()
{
    delete bagGL;
    delete ui;
}

void BagViewer::openBag(const std::string& fname)
{
    bagGL->openBag(QString(fname.c_str()));
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
        if(!bagGL->openBag(fileName))
            QMessageBox::critical(this, tr("Error"), tr("Could not open bag file"));
    }
}

void BagViewer::on_actionTopographic_triggered()
{
    bagGL->setColormap("topographic");
}

void BagViewer::on_actionOmnimap_triggered()
{
    bagGL->setColormap("omnimap");
}

