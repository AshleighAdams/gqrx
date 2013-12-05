#include "dockscanner.h"
#include "ui_dockscanner.h"

DockScanner::DockScanner(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::DockScanner)
{
    ui->setupUi(this);
}

DockScanner::~DockScanner()
{
    delete ui;
}

void DockScanner::setNewFttData(double *fftData, int size)
{
    ui->scanner->setNewFttData(fftData, size);
}

void DockScanner::setSampleRate(double rate)
{
    ui->scanner->setSampleRate(rate);
}

void DockScanner::on_scanner_newDemodFreqDelta(qint64 delta)
{
    emit newDemodFreqDelta(delta); //Re-emit the signal
}

void DockScanner::on_enableButton_toggled(bool checked)
{
    ui->scanner->setScannerEnabled(checked);
}

void DockScanner::on_ignoreButton_clicked()
{
    ui->scanner->ignoreCurrent();
}

void DockScanner::on_clearButton_clicked()
{
    ui->scanner->clear();
}
