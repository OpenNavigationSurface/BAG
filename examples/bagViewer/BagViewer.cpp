#include "BagViewer.h"
#include "ui_BagViewer.h"
#include "BagGL.h"

#include <QFileDialog>
#include <QMessageBox>

BagViewer::BagViewer(QWidget* parent): QMainWindow(parent),
    bagGL(new BagGL), ui(new Ui::BagViewer)
{
    ui->setupUi(this);
    
    QSurfaceFormat format;
    format.setSamples(16);
    
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
    }
}
