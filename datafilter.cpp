#include "datafilter.h"

DataFilter::DataFilter()
{
    // 要不要在析构中，初始化清0全局变量？
}

DataFilter::~DataFilter()
{

}

QByteArray DataFilter::xFrameDataFilter(QByteArray *str)
{
    int num = str->size();
    if(num)
    {
        for(int i=0; i<num; i++)
        {
            chrtmp[tnum] = str->at(i);  		// 从接收缓存区读取一个字节

            if (f_h_flag == 1)  // 有帧头，判断帧尾，接收消息
            {
                if (f_t1_flag == 1) //有帧头，有帧尾1
                {
                    if (chrtmp[tnum] == Frame_Tail2)
                    {
                        // 用户处理代码 //
                        // 将接收到符合帧定义的帧，原路发送回去 //
                        /*int i = 0;
                        for (i = 0; i < (tnum + 1); i++)
                        {
                            //serialPutchar(hs1, chrtmp[i]);	// 通过串口发送字节
                        }*/
                        QByteArray str1;
                        for(int i=0; i<(tnum + 1); i++)
                        {
                            str1.append(chrtmp[i]);
                        }
                        return str1;

                        /*// 16进制显示，并转换为大写
                        QString str2 = str1.toHex().toUpper();
                        // 添加空格
                        QString str3;
                        for(int i = 0; i<str2.length (); i+=2)
                        {
                            str3 += str2.mid (i,2);
                            str3 += " ";
                        }
                        ui->txtSend->insertPlainText(str3);
                        // 移动光标到文本结尾
                        ui->txtSend->moveCursor(QTextCursor::End);*/


                        //  处理完用户代码，重新接收计数 //
                        tnum = 0;
                    }
                    else
                    {
                        f_t1_flag = 0;
                        tnum ++;
                    }
                }
                else						// 有帧头，无帧尾1
                {
                    if (chrtmp[tnum] == Frame_Tail1)
                    {
                        f_t1_flag = 1;
                        tnum ++;
                    }
                    else					// 接收消息包中间内容
                    {
                        tnum ++;
                    }
                }
            }
            else						// 没有接收到帧头
            {
                if (f_h1_flag == 1)			        //没有帧头，有帧头1。下一步判断是否为第2个字节
                {
                    if (chrtmp[tnum] == Frame_Header2)          // 如果为帧头的第2个字节，接收到帧头标志位标志位置1，tnum自增
                    {
                        f_h_flag = 1;
                        tnum ++;
                    }
                    else
                    {
                        f_h1_flag = 0;
                        tnum = 0;
                    }
                }
                else						//没有帧头，没有有帧头1。下一步判断，是否为帧头的第1个字节
                {
                    if (chrtmp[tnum] == Frame_Header1)  // 如果为帧头的第1个字节，标志位置1，tnum自增
                    {
                        f_h1_flag = 1;
                        tnum ++;
                    }
                    else                                // 否则，标志位清0，tnum清0
                    {
                        tnum = 0;
                    }
                }
            }

            // 大于MaxFrameLength个字节的帧不接收
            if (tnum > (MaxFrameLength - 1) )
            {
                tnum = 0;
                f_h1_flag = 0;
                f_h_flag = 0;
                f_t1_flag = 0;
                continue;
            }
        }
    }
}

/*
int main(void)
{


    while (1)
    {
        snum = serialDataAvail(hs1);                	// 获取串口接收缓存区的可用字节数
        if(snum > 0)
        {
            chrtmp[tnum] = serialGetchar(hs1);  		// 从接收缓存区读取一个字节

            if (f_h_flag == 1)  // 有帧头，判断帧尾，接收消息
            {
                if (f_t1_flag == 1) //有帧头，有帧尾1
                {
                    if (chrtmp[tnum] == Frame_Tail2)
                    {
                        // 用户处理代码 //
                        // 将接收到符合帧定义的帧，原路发送回去 //
                        int i = 0;
                        for (i = 0; i < (tnum + 1); i++)
                        {
                            serialPutchar(hs1, chrtmp[i]);	// 通过串口发送字节
                        }

                        //  处理完用户代码，重新接收计数 //
                        tnum = 0;
                    }
                    else
                    {
                        f_t1_flag = 0;
                        tnum ++;
                    }
                }
                else						// 有帧头，无帧尾1
                {
                    if (chrtmp[tnum] == Frame_Tail1)
                    {
                        f_t1_flag = 1;
                        tnum ++;
                    }
                    else					// 接收消息包中间内容
                    {
                        tnum ++;
                    }
                }
            }
            else						// 没有接收到帧头
            {
                if (f_h1_flag == 1)			        //没有帧头，有帧头1。下一步判断是否为第2个字节
                {
                    if (chrtmp[tnum] == Frame_Header2)          // 如果为帧头的第2个字节，接收到帧头标志位标志位置1，tnum自增
                    {
                        f_h_flag = 1;
                        tnum ++;
                    }
                    else
                    {
                        f_h1_flag = 0;
                        tnum = 0;
                    }
                }
                else						//没有帧头，没有有帧头1。下一步判断，是否为帧头的第1个字节
                {
                    if (chrtmp[tnum] == Frame_Header1)  // 如果为帧头的第1个字节，标志位置1，tnum自增
                    {
                        f_h1_flag = 1;
                        tnum ++;
                    }
                    else                                // 否则，标志位清0，tnum清0
                    {
                        tnum = 0;
                    }
                }
            }

            // 大于MaxFrameLength个字节的帧不接收
            if (tnum > (MaxFrameLength - 1) )
            {
                tnum = 0;
                f_h1_flag = 0;
                f_h_flag = 0;
                f_t1_flag = 0;
                continue;
            }

        }
    }

    return 0;
    */
