#include "mainwnd.h"
#include <QHBoxLayout>
#include <QTimer>
#include <QPainter>

unsigned int ledflag = 1;
unsigned int fsflag = 1;
unsigned int fmflag = 1;

mainwnd::mainwnd(QWidget *parent) : QWidget(parent)
{
    resize(700, 500);
    setMaximumSize(830, 500);
    setMinimumSize(830, 500);
    gridLayout = new QGridLayout;
    gridLayout->setMargin(2);
    gridLayout->setSpacing(20);
    video = new QLabel;
//    video->setMargin(0);
    gridLayout->addWidget(video, 0, 0, 0, 0);

    wd = new QLabel;
    wd->setText("温度");
    QFont font;
    font.setFamily(QStringLiteral("Arial"));
    font.setPointSize(12);
    wd->setFont(font);
    wd->setFrameShape(QFrame::Panel);
    wd->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(wd, 0, 5, 1, 1);

    wddis = new QLCDNumber;
    gridLayout->addWidget(wddis, 1, 5, 1, 1);


    shidu = new QLabel;
    shidu->setText("湿度");
    shidu->setFont(font);
    shidu->setFrameShape(QFrame::Panel);
    shidu->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(shidu, 2, 5, 1, 1);

    sddis = new QLCDNumber;
    gridLayout->addWidget(sddis, 3, 5, 1, 1);


    gxqd = new QLabel;
    gxqd->setText("光线强度");
    gxqd->setFont(font);
    gxqd->setFrameShape(QFrame::Panel);
    gxqd->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(gxqd, 4, 5, 1, 1);

    gxqddis = new QLCDNumber;
    gridLayout->addWidget(gxqddis, 5, 5, 1, 1);


    envbutton = new QPushButton;
    envbutton->setText("获取环境信息");
    gridLayout->addWidget(envbutton, 6, 5, 1, 1);

    leddis = new QLabel;
    leddis->setFont(font);
    leddis->setText("status");
    leddis->setFrameShape(QFrame::Panel);
    leddis->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(leddis, 7, 4, 1, 1);

    fandis = new QLabel;
    fandis->setFont(font);
    fandis->setText("status");
    fandis->setFrameShape(QFrame::Panel);
    fandis->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(fandis, 8, 4, 1, 1);



    buzzerdis = new QLabel;
    buzzerdis->setFont(font);
    buzzerdis->setText("status");
    buzzerdis->setFrameShape(QFrame::Panel);
    buzzerdis->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(buzzerdis, 9, 4, 1, 1);

    dswitch = new QPushButton;
    dswitch->setText("灯开关");
    gridLayout->addWidget(dswitch, 7, 5, 1, 1);

    fsswitch = new QPushButton;
    fsswitch->setText("风扇开关");
    gridLayout->addWidget(fsswitch, 8, 5, 1, 1);

    fmswitch = new QPushButton;
    fmswitch->setText("蜂鸣器");
    gridLayout->addWidget(fmswitch, 9, 5, 1, 1);


    playswitch = new QPushButton;
    playswitch->setText("播放");
    gridLayout->addWidget(playswitch, 10, 5, 1, 1);

    this->setLayout(gridLayout);

    Socket = new QTcpSocket(this);
//    Socket->waitForReadyRead(30000);
    Socket->setReadBufferSize(1024*1024);
    connect(Socket, SIGNAL(connected()), this, SLOT(SlotConnected()));
    Socket->connectToHost(QHostAddress("192.168.2.28"), 8000);
/*
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(SlotRecv()));
    timer->start(100);//定时器，每隔0.1秒刷新
*/
    this->setWindowTitle("等待连接...");
}

mainwnd::~mainwnd()
{
}


void mainwnd :: SlotConnected()
{
    qDebug() << "~~~连接成功~~~" << endl;
    this->setWindowTitle("连接成功");


/************************************************************************/
    connect(Socket, SIGNAL(readyRead()), this, SLOT( SlotRecv()));
/************************************************************************/
    connect(dswitch,  SIGNAL(clicked()),this, SLOT( SlotSendLedon()));
/************************************************************************/
    connect(fsswitch,  SIGNAL(clicked()),this, SLOT( SlotSendFson()));
/************************************************************************/
    connect(fmswitch,  SIGNAL(clicked()),this, SLOT( SlotSendFmon()));
/************************************************************************/
    connect(playswitch,  SIGNAL(clicked()),this, SLOT( SlotSendPlay()));
/************************************************************************/
    connect(envbutton,  SIGNAL(clicked()),this, SLOT( SlotSendGetM0()));
/************************************************************************/
}


void mainwnd :: SlotRecv()
{
    if(Socket && Socket->isValid())
    {
        qDebug() << "~~~recv~~~" << endl;
        QByteArray arr;
        arr = Socket->readAll();
        qDebug() << "get the message from the server:" << arr << endl;

        if(arr == "led-on")
            leddis->setText(arr);
        if(arr == "led-off")
            leddis->setText(arr);
        if(arr == "fan-on")
            fandis->setText(arr);
        if(arr == "fan-off")
            fandis->setText(arr);
        if(arr == "bee-on")
            buzzerdis->setText(arr);
        if(arr == "bee-off")
            buzzerdis->setText(arr);

        if(arr.mid(0, 3) == "cam")
        {
            QString s = arr.mid(3, 20);
            arr.remove(0, 23);
            imageData.append(arr);

            if(imageData.length() == s.toInt())
                qDebug() << "image size can be confirm" << endl;
            qDebug() << s.toInt() << endl;
            qDebug() << imageData.length() << endl;

            changeFace();
        }

        if(arr.mid(0, 3) == "tmp")
        {
            qDebug() << "temp message" << endl;
            QString env = arr.mid(3,2);
            wddis->display(env);
            env = arr.mid(5,2);
            sddis->display(env);
            env = arr.mid(7,4);
            gxqddis->display(env);
        }
    }
}

void mainwnd :: SlotSendPlay()
{
    qDebug() << "video message" << endl;

    Socket->write("camon");
}

void mainwnd :: changeFace()
{
    qDebug() << "~~~changeFace ~~~" << endl;
    QPalette palette;
    QImage image=QImage::fromData(imageData,"jpg");
    palette.setBrush(QPalette::Background, QBrush(image));
    video->resize(QSize(image.width(),image.height()));
    video->setPixmap(QPixmap::fromImage(image));
    video->setPalette(palette);
}

void mainwnd ::SlotSendGetM0()
{
    qDebug() << "~~~send M0 command ~~~" << endl;
    Socket->write("temp");
}

void mainwnd :: SlotSendLedon()
{
    qDebug() << "~~~send led control ~~~" << endl;
    ledflag = ~ledflag;
    if (ledflag == 1)
    {
        Socket->write("ledoff");
        qDebug() << " led off " << endl;
    }
    else
    {
        Socket->write("ledon");
        qDebug() << " led on " << endl;
    }
}

void mainwnd :: SlotSendFson()
{
    qDebug() << "~~~send fs control ~~~" << endl;
    fsflag = ~fsflag;
    if (fsflag == 1)
    {
        Socket->write("fanoff");
        qDebug() << " fs off " << endl;
    }
    else
    {
        Socket->write("fanon");
        qDebug() << " fs on " << endl;
    }
}

void mainwnd :: SlotSendFmon()
{
    qDebug() << "~~~send fm control ~~~" << endl;
    fmflag = ~fmflag;
    if (fmflag == 1)
    {
        Socket->write("beeoff");
        qDebug() << " bee off " << endl;
    }
    else
    {
        Socket->write("beeon");
        qDebug() << " bee on " << endl;
    }
}
