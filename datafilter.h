#ifndef DATAFILTER_H
#define DATAFILTER_H

#include <QString>

// 最大帧长度
#define MaxFrameLength	10			// 对最大帧长度加以限定，防止接收到过长的帧数据
// 完整的帧头，2个字节
#define Frame_Header1   0x3A                    // 串口接收消息包的帧头的第1个字节
#define Frame_Header2   0x3B                    // 串口接收消息包的帧头的第2个字节
// 完整的帧尾，2个字节
#define Frame_Tail1     0x7E                    // 串口接收消息包的帧尾的第1个字节
#define Frame_Tail2     0x7F                    // 串口接收消息包的帧尾的第2个字节


class DataFilter
{
public:
    DataFilter();
    ~DataFilter();

    QByteArray xFrameDataFilter(QByteArray *str);

private:
    int snum = 0;                               // 系统串口接收缓存区的可用字节数
    int tnum = 0;                               // 用户串口接收区的数目
    unsigned char chrtmp[100];                  // 用户串口接收区，将缓存的数据放入这里处理
    int f_h1_flag = 0;                          // 接收到帧头的第一个字节标志位
    int f_h_flag = 0;                           // 接收到帧头标志位
    int f_t1_flag = 0;                          // 接收到帧尾的第一个字节标志位
};

#endif // DATAFILTER_H
