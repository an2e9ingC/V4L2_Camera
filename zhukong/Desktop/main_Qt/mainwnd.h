#ifndef MAINWND_H
#define MAINWND_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QLCDNumber>
#include <QPushButton>
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>

class mainwnd : public QWidget
{
    Q_OBJECT

public:
    explicit mainwnd(QWidget *parent = 0);
    ~mainwnd();

protected slots:
    void SlotRecv();
    void SlotConnected();
    void SlotSendLedon();
    void SlotSendFson();
    void SlotSendFmon();
    void SlotSendPlay();
    void SlotSendGetM0();
    void changeFace();

private:
    QGridLayout *glayout;
    QGridLayout *gridLayout;
    QLabel *video;
    QLabel *wd;
    QLabel *gxqd;
    QLabel *shidu;
    QLabel *leddis;
    QLabel *fandis;
    QLabel *buzzerdis;
    QLCDNumber *wddis;
    QLCDNumber *gxqddis;
    QLCDNumber *sddis;
    QPushButton *dswitch;
    QPushButton *fsswitch;
    QPushButton *fmswitch;
    QPushButton *envbutton;
    QPushButton *playswitch;

    QTcpSocket *Socket;
    QByteArray imageData;
};

#endif // MAINWND_H
