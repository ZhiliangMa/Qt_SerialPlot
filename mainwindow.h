#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <QPainter>
#include "plot.h"

// 接收缓冲区大小，单位字节
#define BufferSize      50
// 最大帧长度，单位字节
#define MaxFrameLength	40+5			// 对最大帧长度加以限定，防止接收到过长的帧数据
// 完整的帧头，2个字节
#define Frame_Header1   0x3A                    // 串口接收消息包的帧头的第1个字节
#define Frame_Header2   0x3B                    // 串口接收消息包的帧头的第2个字节
// 完整的帧尾，2个字节
#define Frame_Tail1     0x7E                    // 串口接收消息包的帧尾的第1个字节
#define Frame_Tail2     0x7F                    // 串口接收消息包的帧尾的第2个字节

// 功能字1，0x01，自定义波形显示
#define FunWord_WF      0x01
// 功能字2，0x02，信息绘图页面显示，暂时未用
#define FunWord_SM      0x02
// 帧数据中包含有效字节的最大长度
#define ValidByteLength	40			// 对最大帧长度加以限定，防止接收到过长的帧数据

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 绘图事件
    void paintEvent(QPaintEvent *);

private slots:
    void on_btnSwitch_clicked();

    void serialPortRead_Slot();

    void on_btnSend_clicked();

    void on_btnClearRec_clicked();

    void on_btnClearSend_clicked();

    void on_chkRec_stateChanged(int arg1);

    void on_chkSend_stateChanged(int arg1);

    void on_chkTimSend_stateChanged(int arg1);

    void on_pushButton_clicked();

    void on_btnFramaDebug_clicked();

    void dataRateCalculate(void);

private:
    Ui::MainWindow *ui;
    // 波形绘图窗口
    Plot *plot = NULL;// 必须初始化为空，否则后面NEW判断的时候会异常结束

    QSerialPort *mySerialPort;

    // 发送、接收字节计数
    long sendNum=0, recvNum=0, tSend=0, tRecv=0;// 发送/接收数量，历史发送/接收数量，Byte
    long sendRate=0, recvRate=0;// 发送/接收速率，Byte/s
    long recvFrameNum=0, recvFrameRate=0, recvErrorNum=0, tFrame=0;// 接收的有效帧数，帧速率，误码帧数量，历史帧数量
    QLabel *lblSendNum, *lblRecvNum, *lblSendRate, *lblRecvRate, *lblRecvFrameNum, *lblFrameRate;

    void setNumOnLabel(QLabel *lbl, QString strS, long num);

    /* 与帧过滤有关的标志位 */
    //int snum = 0;                               // 系统串口接收缓存区的可用字节数
    int tnum = 0;                               // 用户串口接收缓存的指针位置
    unsigned char chrtmp[BufferSize];           // 用户串口接收缓存，将缓存的数据放入这里处理
    int f_h1_flag = 0;                          // 接收到帧头的第一个字节标志位
    int f_h_flag = 0;                           // 接收到帧头标志位
    int f_t1_flag = 0;                          // 接收到帧尾的第一个字节标志位
    // 即是标志位，也包含信息
    int f_fun_word = 0;                         // 功能字，限定为0x01、0x02
    int f_length = 0;                           // 帧数据中包含有效字节的长度
    void xFrameDataFilter(QByteArray *str, short value[]);
    //QByteArray xFrameDataFilter(QByteArray *str);

    // 定时发送-定时器
    QTimer *timSend;
    // 发送速率、接收速率统计-定时器
    QTimer *timRate;
};
#endif // MAINWINDOW_H
