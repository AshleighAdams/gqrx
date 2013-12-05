#ifndef SCANNER_H
#define SCANNER_H

#include <QFrame>
#include <QtGui>
#include <QImage>

#define MIN_ACTIVE_TIME 10 //0.01 secs
#define MIN_STAY_TIME 100 //0.1 secs

class Scanner : public QFrame
{
    Q_OBJECT
public:
    explicit Scanner(QWidget *parent = 0);
    ~Scanner();

    void setNewFttData(double *fftData, int size);
    void setSampleRate(double rate) { m_SampleRate=rate; }
    void setScannerEnabled(bool enabled);
    void ignoreCurrent();
    void clear();

signals:
    void newDemodFreqDelta(qint64 delta);

public slots:
    
protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent* event);
    void mousePressEvent(QMouseEvent * event);

private:
    void draw();
    void updateBinData();
    int binFromX(int x);

    bool m_Enabled;
    double m_SampleRate;
    double *m_fftData;
    int m_fftDataSize;

    QPixmap m_2DPixmap;
    QSize m_Size;
    QBrush m_Brush;
    QColor m_PenColor;
    QBrush m_BackgroundBrush;

    double m_Threshold;
    double m_MindB;
    double m_MaxdB;

    int m_NumBins;
    double* m_BinBestDB;
    qint64* m_BinBestOffset;
    int* m_BinTimes;

    QTime m_LastUpdate;
    QTime m_LastTune;

    int m_CurrentBin;

};

#endif // SCANNER_H
