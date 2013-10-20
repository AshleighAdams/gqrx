#ifndef DOCKSCANNER_H
#define DOCKSCANNER_H

#include <QDockWidget>

namespace Ui {
class DockScanner;
}

class DockScanner : public QDockWidget
{
    Q_OBJECT
    
public:
    explicit DockScanner(QWidget *parent = 0);
    ~DockScanner();
    void setNewFttData(double *fftData, int size);
    void setSampleRate(double rate);

signals:
    void newDemodFreqDelta(qint64 delta);

private slots:
    void on_scanner_newDemodFreqDelta(qint64 delta);
    void on_enableButton_toggled(bool checked);
    void on_ignoreButton_clicked();
    void on_clearButton_clicked();

private:
    Ui::DockScanner *ui;
};

#endif // DOCKSCANNER_H
