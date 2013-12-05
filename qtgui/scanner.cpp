#include "scanner.h"
#include <QtGlobal>

#include<math.h>

Scanner::Scanner(QWidget *parent) : QFrame(parent)
{
    m_Enabled=false;

    m_Size = QSize(0,0);
    m_2DPixmap = QPixmap(0,0);

    m_SampleRate=96000;
    m_fftData = NULL;
    m_fftDataSize = 0;

    m_BackgroundBrush=QBrush();
    m_Brush=QBrush();

    m_Threshold = -30;
    m_MindB = -80;
    m_MaxdB = 0;

    m_NumBins=100;
    m_BinTimes = new int[m_NumBins](); //Default initializer
    m_BinBestDB = new double[m_NumBins];
    m_BinBestOffset = new qint64[m_NumBins];
    m_CurrentBin=-1;
    m_LastUpdate=QTime();
    m_LastTune=QTime();

    m_PenColor = Qt::yellow;
}

Scanner::~Scanner()
{
    delete[] m_BinBestDB;
    delete[] m_BinBestOffset;
    delete[] m_BinTimes;
}

void Scanner::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_2DPixmap);
    painter.end();
}

void Scanner::resizeEvent(QResizeEvent *)
{
    if (!size().isValid())
        return;

    if (m_Size != size())
    {
        m_Size = size();

        QLinearGradient gradient(0, 0, 0, m_Size.height());
        gradient.setColorAt(1, Qt::blue);
        gradient.setColorAt(0, Qt::cyan);
        m_Brush = QBrush(gradient);

        QLinearGradient gradient2(0, 0, 0, m_Size.height());
        gradient2.setColorAt(1, Qt::black);
        gradient2.setColorAt(0, QColor::fromRgb(60, 60, 60));
        m_BackgroundBrush = QBrush(gradient2);

        m_2DPixmap = QPixmap(m_Size.width(), m_Size.height());
    }

    draw();
}

void Scanner::setNewFttData(double *fftData, int size)
{
    m_fftData = fftData;
    m_fftDataSize = size;

    if(m_Enabled)
    {
        updateBinData();
        draw();
    }
}

void Scanner::setScannerEnabled(bool enabled)
{
    m_Enabled=enabled;

    if(!m_Enabled)
        m_CurrentBin = -1;

    draw();
}

void Scanner::ignoreCurrent()
{
    if(m_CurrentBin != -1)
    {
        m_BinTimes[m_CurrentBin] = -1;
        m_CurrentBin = -1;
        draw();
    }
}

void Scanner::clear()
{
    for(int i=0; i<m_NumBins; i++)
        m_BinTimes[i]=0;
    draw();
}

void Scanner::mousePressEvent(QMouseEvent * event)
{
    if(!m_Enabled)
        return;

    QPoint pt = event->pos();

    if (event->buttons() == Qt::LeftButton)
    {
        m_Threshold = (m_Size.height()-pt.y())/(double)m_Size.height() * (m_MaxdB - m_MindB) + m_MindB;
        draw();
    }
    else if(event->buttons() == Qt::RightButton)
    {
        int bin = binFromX(pt.x());
        if(bin>=0 && bin<m_NumBins)
        {
            m_BinTimes[bin]=(m_BinTimes[bin]==-1)?0:-1;
            if(bin==m_CurrentBin)
                m_CurrentBin=-1;
        }
    }
 }

template<typename T>
T map(T Value, T FromMin, T FromMax, T ToMax, T ToMin)
{
        T from_diff = FromMax - FromMin;
        T to_diff = ToMax - ToMin;

        T y = (Value - FromMin) / from_diff;
        return ToMin + to_diff * y;
}

void Scanner::updateBinData()
{
    int elapsed = 0;
    if(m_LastUpdate.isNull())
        m_LastUpdate.start();
    else
        elapsed=m_LastUpdate.restart();

    for(int i=0; i<m_NumBins; i++)
        m_BinBestDB[i]=-1e9;

    for(int i=0; i<m_fftDataSize; i++)
    {
        int bin = round(i * (double)m_NumBins/m_fftDataSize);

        if(m_fftData[i]>m_BinBestDB[bin])
        {
            m_BinBestDB[bin]=m_fftData[i];
            m_BinBestOffset[bin] =i/(double)m_fftDataSize*m_SampleRate - m_SampleRate/2.0;
        }
    }

    // attempt to re-locate the center of the current bin:

    if(m_CurrentBin != -1)
    {
        int center = m_CurrentBin;
        int lower_bound = center;
        int higher_bound = center;

        for(int i = center - 1; i >= 0; i--)
            if(m_BinTimes[i] >= MIN_ACTIVE_TIME) //(m_BinBestDB[i] >= m_Threshold)
                lower_bound = i;
            else
                break;

        for(int i = center + 1; i < m_NumBins; i++)
            if(m_BinTimes[i] >= MIN_ACTIVE_TIME)//(m_BinBestDB[i] >= m_Threshold)
                higher_bound = i;
            else
                break;

        center = round(map(0.5, 0.0, 1.0, (double)lower_bound, (double)higher_bound));

        if(center != m_CurrentBin)
        {
            if(m_BinTimes[center]==-1) //Bin disabled
            {
                m_CurrentBin = -1;
            }
            else
            {
                m_CurrentBin = center;
                emit newDemodFreqDelta(m_BinBestOffset[center]);
            }
        }
    }

    //Check if we need to switch bin
    int best=-1;

    /*
    for(int i=0; i<m_NumBins; i++)
    {
        if(m_BinTimes[i]==-1) //Bin disabled
            continue;

        if(m_BinBestDB[i]>=m_Threshold)
        {
            int peak = 0;
            int start = i;
            int end;

            for(i++;i<m_NumBins;i++)
                if(m_BinBestDB[i] < m_Threshold)
                    break;
                else
                    peak = m_BinBestDB[i] > peak ? m_BinBestDB[i] : peak;
            end = i - 1;

            best = start + round(((double)start - (double)end) / 2.0);
        }
    }
    */

    for(int i=0; i<m_NumBins; i++)
    {
        if(m_BinTimes[i]==-1) //Bin disabled
            continue;

        if(m_BinBestDB[i]>=m_Threshold)
            m_BinTimes[i]=qMin(m_BinTimes[i]+elapsed, (int)1e9); //Prevent overflows
        else
            m_BinTimes[i]=0;

        if(m_BinTimes[i]>=MIN_ACTIVE_TIME && (best==-1 || m_BinBestDB[i]>m_BinBestDB[best]))
            best=i;
    }

    if(m_CurrentBin!=-1 && m_BinBestDB[m_CurrentBin]>m_Threshold)
        m_LastTune.start();

    bool canSwitch =  m_LastTune.isNull() || m_LastTune.elapsed() > MIN_STAY_TIME;
    if(canSwitch && best!=-1)
    {
        m_CurrentBin=best;
        m_LastTune.start();

        emit newDemodFreqDelta(m_BinBestOffset[best]);
    }
}

int Scanner::binFromX(int x)
{
    if(m_Size.width()<=0)
        return -1;

    return x*m_NumBins/m_Size.width();
}

void Scanner::draw()
{
    if(!m_Size.isValid())
        return;

    int w = m_Size.width();
    int h = m_Size.height();
    double  dBGainFactor = h/(m_MaxdB-m_MindB);

    QPainter painter(&m_2DPixmap);

    painter.fillRect(0, 0, m_Size.width(), m_Size.height(), m_BackgroundBrush);

    if(m_fftData!=NULL && m_Enabled)
    {
        painter.setPen(m_PenColor);
        painter.setBrush(QBrush(Qt::green));

        QRect rects[m_NumBins];
        int r=0;
        int s=m_NumBins;
        for(int i=0; i<m_NumBins; i++)
        {
            int binH = (int)round((m_BinBestDB[i]-m_MindB)*dBGainFactor);

            QRect rect=QRect((int)(i*w/m_NumBins), h-binH, (int)(w/m_NumBins), h);

            if(i==m_CurrentBin)
                painter.drawRect(rect);
            else if(m_BinTimes[i]!=-1) //bin enabled
                rects[r++]=rect;
            else //bin disabled
                rects[--s]=rect;
        }

        painter.setBrush(m_Brush);
        painter.drawRects(rects, r);
        painter.setBrush(QBrush(Qt::red));
        painter.drawRects(rects+s, m_NumBins-s);
    }

    painter.setPen(QPen(QColor(255, 255, 255, 128), 1, Qt::DashLine));
    int lineH = (int)(h - (m_Threshold-m_MindB)*dBGainFactor);
    painter.drawLine(0, lineH, w, lineH);

    painter.end();
    update();
}

