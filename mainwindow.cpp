#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Qt Serial Debugger");

    // 状态栏
    QStatusBar *sBar = statusBar();
    // 状态栏的收、发计数标签
    lblSendNum = new QLabel(this);
    lblRecvNum = new QLabel(this);
    lblSendRate = new QLabel(this);
    lblRecvRate = new QLabel(this);
    // 设置标签最小大小
    lblSendNum->setMinimumSize(100, 20);
    lblRecvNum->setMinimumSize(100, 20);
    lblSendRate->setMinimumSize(100, 20);
    lblRecvRate->setMinimumSize(100, 20);
    //statusBar()->showMessage("留言", 5000);// 留言显示，过期时间单位为ms，过期后不再有显示
    //statusBar()->setSizeGripEnabled(false); // 是否显示右下角拖放控制点，默认显示
    //statusBar()->setStyleSheet(QString("QStatusBar::item{border: 0px}")); // 设置不显示label的边框
    //lblSendNum->setAlignment(Qt::AlignHCenter);// 设置label属性
    //sBar->addPermanentWidget();//addSeparator();// 添加分割线，不能用
    // 状态栏显示计数值
    //lblSendNum->setText("S: 0");
    //lblRecvNum->setText("R: 0");
    setNumOnLabel(lblSendNum, "S: ", sendNum);
    setNumOnLabel(lblRecvNum, "R: ", recvNum);
    setNumOnLabel(lblSendRate, "Byte/s: ", 0);
    setNumOnLabel(lblRecvRate, "Byte/s: ", 0);
    // 从右往左依次添加
    sBar->addPermanentWidget(lblSendNum);
    sBar->addPermanentWidget(lblSendRate);
    sBar->addPermanentWidget(lblRecvNum);
    sBar->addPermanentWidget(lblRecvRate);

    // 状态栏添加超链接
    QLabel *lblLinkBlog = new QLabel(this);
    lblLinkBlog->setOpenExternalLinks(true);
    //lblLinkBlog->setText("<a href=\"https://blog.csdn.net/Mark_md/article/details/108928314\">博客");// 有下划线
    lblLinkBlog->setText("<style> a {text-decoration: none} </style> <a href=\"https://blog.csdn.net/Mark_md/article/details/108928314\">博客");// 无下划线
    QLabel *lblLinkSource = new QLabel(this);
    lblLinkSource->setOpenExternalLinks(true);
    //lblLinkSource->setText("<a href=\"https://github.com/ZhiliangMa/Qt-SerialDebuger\">源码下载");
    lblLinkSource->setText("<style> a {text-decoration: none} </style> <a href=\"https://github.com/ZhiliangMa/Qt-SerialDebuger\">源码下载");// 无下划线
    lblLinkBlog->setMinimumSize(40, 20);
    lblLinkSource->setMinimumSize(60, 20);
    // 从左往右依次添加
    sBar->addWidget(lblLinkBlog);
    sBar->addWidget(lblLinkSource);

    lblRecvFrameNum = new QLabel(this);
    lblFrameRate = new QLabel(this);
    lblRecvFrameNum->setMinimumSize(100, 20);
    lblFrameRate->setMinimumSize(80, 20);
    setNumOnLabel(lblRecvFrameNum, "FNum: ", recvFrameNum);
    setNumOnLabel(lblFrameRate, "FPS/s: ", recvFrameRate);
    // 从右往左依次添加
    sBar->addPermanentWidget(lblRecvFrameNum);
    sBar->addPermanentWidget(lblFrameRate);

    // 定时发送-定时器
    timSend = new QTimer;
    timSend->setInterval(1000);// 设置默认定时时长1000ms
    connect(timSend, &QTimer::timeout, this, [=](){
        on_btnSend_clicked();
    });

    // 发送速率、接收速率统计-定时器
    timRate = new QTimer;
    timRate->start(1000);
    connect(timRate, &QTimer::timeout, this, [=](){
        dataRateCalculate();
    });

    // 新建一串口对象
    mySerialPort = new QSerialPort(this);

    // 串口接收，信号槽关联
    connect(mySerialPort, SIGNAL(readyRead()), this, SLOT(serialPortRead_Slot()));

    // 隐藏高级收码显示区域
    ui->widget_5->hide();
    ui->frame_3->hide();

    // 新建波形显示界面
    plot = new Plot;
}

MainWindow::~MainWindow()
{
    delete plot;
    delete ui;
}

// 绘图事件
void MainWindow::paintEvent(QPaintEvent *)
{
    // 绘图
    // 实例化画家对象，this指定绘图设备
    QPainter painter(this);

    // 设置画笔颜色
    QPen pen(QColor(0,0,0));
    // 设置画笔线宽（只对点线圆起作用，对文字不起作用）
    pen.setWidth(1);
    // 设置画笔线条风格，默认是SolidLine实线
    // DashLine虚线，DotLine点线，DashDotLine、DashDotDotLine点划线
    pen.setStyle(Qt::DashDotDotLine);
    // 让画家使用这个画笔
    painter.setPen(pen);

    //painter.drawLine(QPoint(ui->txtRec->x() + ui->txtRec->width(), ui->txtRec->y()), QPoint(this->width(), ui->txtRec->y()));
    //painter.drawLine(QPoint(ui->txtSend->x() + ui->txtSend->width(), ui->txtSend->y()), QPoint(this->width(), ui->txtSend->y()));
    painter.drawLine(QPoint(ui->statusbar->x(), ui->statusbar->y()-2), QPoint(this->width(), ui->statusbar->y()-2));

}

// 串口接收显示，槽函数
void MainWindow::serialPortRead_Slot()
{
    /*QString recBuf;
    recBuf = QString(mySerialPort->readAll());*/

    QByteArray recBuf;
    recBuf = mySerialPort->readAll();

    /* 帧过滤部分代码 */
    short wmValue[20] = {0};
    xFrameDataFilter(&recBuf, wmValue);

    // 调试信息输出，显示缓冲区内容（16进制显示）及接收标志位
    if(!ui->widget_5->isHidden()){
        QByteArray str1;
        //for(int i=0; i<(tnum + 1); i++)
        for(int i=0; i<BufferSize; i++)
        {
            str1.append(chrtmp[i]);
        }
        //ui->txtFrameTemp->setPlainText(str1.toHex().toUpper());
        str1 = str1.toHex().toUpper();
        QString str2;
        for(int i = 0; i<str1.length (); i+=2)
        {
            str2 += str1.mid (i,2);
            str2 += " ";
        }
        ui->txtFrameBuffer->setPlainText(str2);
        // 显示标志位
        ui->txtFrameTnum->setText(QString::number(tnum));
        ui->txtFrameH1->setText(QString::number(f_h1_flag));
        ui->txtFrameH->setText(QString::number(f_h_flag));
        ui->txtFrameFun->setText(QString::number(f_fun_word));
        ui->txtFrameLen->setText(QString::number(f_length));
        ui->txtFrameErrorNum->setText(QString::number(recvErrorNum));
    }

    // 接收字节计数
    recvNum += recBuf.size();
    // 状态栏显示计数值
    setNumOnLabel(lblRecvNum, "R: ", recvNum);

    // 判断是否为16进制接收，将以后接收的数据全部转换为16进制显示（先前接收的部分在多选框槽函数中进行转换。最好多选框和接收区组成一个自定义控件，方便以后调用）
    if(ui->chkRec->checkState() == false){
        // GB2312编码输入
        QString strb = QString::fromLocal8Bit(recBuf);//QString::fromUtf8(recBuf);//QString::fromLatin1(recBuf);
        // 在当前位置插入文本，不会发生换行。如果没有移动光标到文件结尾，会导致文件超出当前界面显示范围，界面也不会向下滚动。
        ui->txtRec->insertPlainText(strb);
    }else{
        // 16进制显示，并转换为大写
        QString str1 = recBuf.toHex().toUpper();//.data();
        // 添加空格
        QString str2;
        for(int i = 0; i<str1.length (); i+=2)
        {
            str2 += str1.mid (i,2);
            str2 += " ";
        }
        ui->txtRec->insertPlainText(str2);
        //ui->txtRec->insertPlainText(recBuf.toHex());
    }

    // 移动光标到文本结尾
    ui->txtRec->moveCursor(QTextCursor::End);

    // 将文本追加到末尾显示，会导致插入的文本换行
    /*ui->txtRec->appendPlainText(recBuf);*/

    /*// 在当前位置插入文本，不会发生换行。如果没有移动光标到文件结尾，会导致文件超出当前界面显示范围，界面也不会向下滚动。
    ui->txtRec->insertPlainText(recBuf);
    ui->txtRec->moveCursor(QTextCursor::End);*/

    // 利用一个QString去获取消息框文本，再将新接收到的消息添加到QString尾部，但感觉效率会比当前位置插入低。也不会发生换行
    /*QString txtBuf;
    txtBuf = ui->txtRec->toPlainText();
    txtBuf += recBuf;
    ui->txtRec->setPlainText(txtBuf);
    ui->txtRec->moveCursor(QTextCursor::End);*/

    // 利用一个QString去缓存接收到的所有消息，效率会比上面高一点。但清空接收的时候，要将QString一并清空。
    /*static QString txtBuf;
    txtBuf += recBuf;
    ui->txtRec->setPlainText(txtBuf);
    ui->txtRec->moveCursor(QTextCursor::End);*/
}

// 打开/关闭串口 槽函数
void MainWindow::on_btnSwitch_clicked()
{

    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;
    QSerialPort::Parity   checkBits;

    // 获取串口波特率
    baudRate = (QSerialPort::BaudRate)ui->cmbBaudRate->currentText().toUInt();
    // 获取串口数据位
    dataBits = (QSerialPort::DataBits)ui->cmbData->currentText().toUInt();
    // 获取串口停止位
    if(ui->cmbStop->currentText() == "1"){
        stopBits = QSerialPort::OneStop;
    }else if(ui->cmbStop->currentText() == "1.5"){
        stopBits = QSerialPort::OneAndHalfStop;
    }else if(ui->cmbStop->currentText() == "2"){
        stopBits = QSerialPort::TwoStop;
    }else{
        stopBits = QSerialPort::OneStop;
    }

    // 获取串口奇偶校验位
    if(ui->cmbCheck->currentText() == "无"){
        checkBits = QSerialPort::NoParity;
    }else if(ui->cmbCheck->currentText() == "奇校验"){
        checkBits = QSerialPort::OddParity;
    }else if(ui->cmbCheck->currentText() == "偶校验"){
        checkBits = QSerialPort::EvenParity;
    }else{
        checkBits = QSerialPort::NoParity;
    }

    // 想想用 substr strchr怎么从带有信息的字符串中提前串口号字符串
    // 初始化串口属性，设置 端口号、波特率、数据位、停止位、奇偶校验位数
    mySerialPort->setBaudRate(baudRate);
    mySerialPort->setDataBits(dataBits);
    mySerialPort->setStopBits(stopBits);
    mySerialPort->setParity(checkBits);
    //mySerialPort->setPortName(ui->cmbSerialPort->currentText());// 不匹配带有串口设备信息的文本
    // 匹配带有串口设备信息的文本
    QString spTxt = ui->cmbSerialPort->currentText();
    spTxt = spTxt.section(':', 0, 0);//spTxt.mid(0, spTxt.indexOf(":"));
    //qDebug() << spTxt;
    mySerialPort->setPortName(spTxt);

    // 根据初始化好的串口属性，打开串口
    // 如果打开成功，反转打开按钮显示和功能。打开失败，无变化，并且弹出错误对话框。
    if(ui->btnSwitch->text() == "打开串口"){
        if(mySerialPort->open(QIODevice::ReadWrite) == true){
            //QMessageBox::
            ui->btnSwitch->setText("关闭串口");
            // 让端口号下拉框不可选，避免误操作（选择功能不可用，控件背景为灰色）
            ui->cmbSerialPort->setEnabled(false);
            ui->cmbBaudRate->setEnabled(false);
            ui->cmbStop->setEnabled(false);
            ui->cmbData->setEnabled(false);
            ui->cmbCheck->setEnabled(false);
        }else{
            QMessageBox::critical(this, "错误提示", "串口打开失败！！！\r\n\r\n该串口可能被占用，请选择正确的串口\r\n或者波特率过高，超出硬件支持范围");
        }
    }else{
        mySerialPort->close();
        ui->btnSwitch->setText("打开串口");
        // 端口号下拉框恢复可选，避免误操作
        ui->cmbSerialPort->setEnabled(true);
        ui->cmbBaudRate->setEnabled(true);
        ui->cmbStop->setEnabled(true);
        ui->cmbData->setEnabled(true);
        ui->cmbCheck->setEnabled(true);
    }

}

// 发送按键槽函数
// 如果勾选16进制发送，按照asc2的16进制发送
void MainWindow::on_btnSend_clicked()
{
    QByteArray sendData;
    // 判断是否为16进制发送，将发送区全部的asc2转换为16进制字符串显示，发送的时候转换为16进制发送
    if(ui->chkSend->checkState() == false){
        // 字符串形式发送，GB2312编码用以兼容大多数单片机
        sendData = ui->txtSend->toPlainText().toLocal8Bit();// GB2312编码输出
        //sendData = ui->txtSend->toPlainText().toUtf8();// Unicode编码输出
        //sendData = ui->txtSend->toPlainText().toLatin1();
    }else{
        // 16进制发送，不要用.data()，.data()返回的是字符数组，0x00在ASC2中的意义为NUL，也就是'\0'结束符，所以遇到0x00就会终止
        //sendData = QByteArray::fromHex(ui->txtSend->toPlainText().toUtf8());// Unicode编码输出
        sendData = QByteArray::fromHex(ui->txtSend->toPlainText().toLocal8Bit());// GB2312编码输出
    }

    // 如发送成功，会返回发送的字节长度。失败，返回-1。
    int a = mySerialPort->write(sendData);
    // 发送字节计数并显示
    if(a > 0)
    {
        // 发送字节计数
        sendNum += a;
        // 状态栏显示计数值
        setNumOnLabel(lblSendNum, "S: ", sendNum);
    }

}

// 状态栏标签显示计数值
void MainWindow::setNumOnLabel(QLabel *lbl, QString strS, long num)
{
    // 标签显示
    QString strN;
    strN.sprintf("%ld", num);
    QString str = strS + strN;
    lbl->setText(str);
}

void MainWindow::on_btnClearRec_clicked()
{
    ui->txtRec->clear();
    // 清除发送、接收字节计数
    sendNum = 0;
    recvNum = 0;
    tSend = 0;
    tRecv = 0;
    // 状态栏显示计数值
    setNumOnLabel(lblSendNum, "S: ", sendNum);
    setNumOnLabel(lblRecvNum, "R: ", recvNum);
    // 清空帧数量
    recvFrameNum=0, recvFrameRate=0, recvErrorNum=0, tFrame=0;
    setNumOnLabel(lblRecvFrameNum, "FNum: ", recvFrameNum);
    ui->txtFrameErrorNum->setText(QString::number(recvErrorNum));
}

void MainWindow::on_btnClearSend_clicked()
{
    ui->txtSend->clear();
    // 清除发送字节计数
    sendNum = 0;
    tSend = 0;
    // 状态栏显示计数值
    setNumOnLabel(lblSendNum, "S: ", sendNum);
}

// 先前接收的部分在多选框状态转换槽函数中进行转换。（最好多选框和接收区组成一个自定义控件，方便以后调用）
void MainWindow::on_chkRec_stateChanged(int arg1)
{
    // 获取文本字符串
    QString txtBuf = ui->txtRec->toPlainText();

    // 获取多选框状态，未选为0，选中为2
    // 为0时，多选框未被勾选，接收区先前接收的16进制数据转换为asc2字符串格式
    if(arg1 == 0){

        //QString str1 = QByteArray::fromHex(txtBuf.toUtf8());
        //QString str1 = QByteArray::fromHex(txtBuf.toLocal8Bit());
        //把gb2312编码转换成unicode
        QString str1 = QTextCodec::codecForName("GB2312")->toUnicode(QByteArray::fromHex(txtBuf.toLocal8Bit()));
        // 文本控件清屏，显示新文本
        ui->txtRec->clear();
        ui->txtRec->insertPlainText(str1);
        // 移动光标到文本结尾
        ui->txtRec->moveCursor(QTextCursor::End);

    }else{// 不为0时，多选框被勾选，接收区先前接收asc2字符串转换为16进制显示

        //QString str1 = txtBuf.toUtf8().toHex().toUpper();// Unicode编码输出
        QString str1 = txtBuf.toLocal8Bit().toHex().toUpper();// GB2312编码输出
        // 添加空格
        QByteArray str2;
        for(int i = 0; i<str1.length (); i+=2)
        {
            str2 += str1.mid (i,2);
            str2 += " ";
        }
        // 文本控件清屏，显示新文本
        ui->txtRec->clear();
        ui->txtRec->insertPlainText(str2);
        // 移动光标到文本结尾
        ui->txtRec->moveCursor(QTextCursor::End);

    }
}

// 先前发送区的部分在多选框状态转换槽函数中进行转换。（最好多选框和发送区组成一个自定义控件，方便以后调用）
void MainWindow::on_chkSend_stateChanged(int arg1)
{
    // 获取文本字符串
    QString txtBuf = ui->txtSend->toPlainText();

    // 获取多选框状态，未选为0，选中为2
    // 为0时，多选框未被勾选，将先前的发送区的16进制字符串转换为asc2字符串
    if(arg1 == 0){

        //QByteArray str1 = QByteArray::fromHex(txtBuf.toUtf8());//仅能处理Unicode编码，因为QString就是Unicode
        //QString str1 = QString::fromLocal8Bit(txtBuf.toUtf8());//仅能处理GB2312编码，Unicode的数据无论如何都会乱码
        //把gb2312编码转换成unicode
        QString str1 = QTextCodec::codecForName("GB2312")->toUnicode(QByteArray::fromHex(txtBuf.toLocal8Bit()));
        // 文本控件清屏，显示新文本
        ui->txtSend->clear();
        ui->txtSend->insertPlainText(str1);
        // 移动光标到文本结尾
        ui->txtSend->moveCursor(QTextCursor::End);

    }else{// 多选框被勾选，将先前的发送区的asc2字符串转换为16进制字符串

        //QByteArray str1 = txtBuf.toUtf8().toHex().toUpper();// Unicode编码输出
        QString str1 = txtBuf.toLocal8Bit().toHex().toUpper();// GB2312编码输出
        // 添加空格
        QString str2;
        for(int i = 0; i<str1.length (); i+=2)
        {
            str2 += str1.mid (i,2);
            str2 += " ";
        }
        // 文本控件清屏，显示新文本
        ui->txtSend->clear();
        ui->txtSend->insertPlainText(str2);
        // 移动光标到文本结尾
        ui->txtSend->moveCursor(QTextCursor::End);

    }
}

// 定时发送开关 选择复选框
void MainWindow::on_chkTimSend_stateChanged(int arg1)
{
    // 获取复选框状态，未选为0，选中为2
    if(arg1 == 0){
        timSend->stop();
        // 时间输入框恢复可选
        ui->txtSendMs->setEnabled(true);
    }else{
        // 对输入的值做限幅，小于20ms会弹出对话框提示
        if(ui->txtSendMs->text().toInt() >= 20){
            timSend->start(ui->txtSendMs->text().toInt());// 设置定时时长，重新计数
            // 让时间输入框不可选，避免误操作（输入功能不可用，控件背景为灰色）
            ui->txtSendMs->setEnabled(false);
        }else{
            ui->chkTimSend->setCheckState(Qt::Unchecked);
            QMessageBox::critical(this, "错误提示", "定时发送的最小间隔为 20ms\r\n请确保输入的值 >=20");
        }
    }
}

// 发送速率、接收速率统计-定时器
void MainWindow::dataRateCalculate(void)
{
    sendRate = sendNum - tSend; // * ui->cmbData->currentText().toUInt();
    recvRate = recvNum - tRecv; // * ui->cmbData->currentText().toUInt();
    recvFrameRate = recvFrameNum - tFrame;

    setNumOnLabel(lblSendRate, "Byte/s: ", sendRate);
    setNumOnLabel(lblRecvRate, "Byte/s: ", recvRate);
    setNumOnLabel(lblFrameRate, "FPS/s: ", recvFrameRate);
    tSend = sendNum;
    tRecv = recvNum;
    tFrame = recvFrameNum;
}

// "显示波形界面" 按键槽函数
// 记得把plot在析构中释放掉，不然很容易下次运行崩溃
void MainWindow::on_pushButton_clicked()
{
    plot->show();// 显示波形绘图窗口
}

/*
void MainWindow::xFrameDataFilter(QByteArray *str)
{
    int num = str->size();
    if(num)
    {

    }
}*/

// 帧过滤
// 适用于有帧头、功能字、有效字段长度、校验位的接收，无帧尾
void MainWindow::xFrameDataFilter(QByteArray *str, short value[])
{
    int num = str->size();
    if(num)
    {
        for(int i=0; i<num; i++)
        {
            chrtmp[tnum] = str->at(i);  		// 从接收缓存区读取一个字节
            if (f_h_flag == 1)  // 有帧头。判断功能字、有效字段长度，接收消息
            {
                if (f_fun_word) // 有帧头，有功能字
                {
                    if (f_length) // 有帧头，有功能字，有有效字节长度
                    {
                        if((tnum-4) <f_length) // 有帧头，有功能字，未超出有效字节长度+校验位，接收数据
                        {
                            tnum ++;
                        }
                        else // 有帧头，有功能字，超出有效字节长度。判断校验位
                        {
                            // 累加和校验计算
                            unsigned char crc=0;
                            for(i=0; i<tnum;i++)
                            {
                                crc += chrtmp[i];
                            }

                            // 校验对比
                            if(crc == chrtmp[tnum]) // 校验通过，将缓冲区的数据打包发送
                            {
                                ++recvFrameNum;// 有效帧数量计数
                                setNumOnLabel(lblRecvFrameNum, "FNum: ", recvFrameNum);

                                // 调试信息输出，显示有效帧的内容（16进制显示）
                                if(!ui->widget_5->isHidden()){
                                    QByteArray str1;
                                    for(int i=0; i<(tnum+1); i++)
                                    {
                                        str1.append(chrtmp[i]);
                                    }
                                    //ui->txtFrameEffective->appendPlainText(str1.toHex().toUpper());
                                    str1 = str1.toHex().toUpper();
                                    QString str2;
                                    for(int i = 0; i<str1.length (); i+=2)
                                    {
                                        str2 += str1.mid (i,2);
                                        str2 += " ";
                                    }
                                    ui->txtFrameEffective->appendPlainText(str2);
                                }

                                // 根据功能字进行功能解析，自动根据帧长度解析为对应的short值。
                                if(f_fun_word == FunWord_WF)
                                {
                                    for(int i=0; i<(f_length/2); i++)
                                    {
                                        value[i] = ((short)chrtmp[i*2+4] << 8) | chrtmp[i*2+4+1];
                                    }
                                }

                                // 显示波形（在这里显示可以处理多帧粘包，避免多帧粘包只显示一帧的情况）
                                // 将解析出的short数组，传入波形图，进行绘图
                                if(!plot->isHidden()){
                                    plot->ShowPlot_WaveForm(plot->pPlot1, value);
                                }

                            }else{
                                ++recvErrorNum;// 误码帧数量计数
                            }

                            // 清0重新接收
                            tnum = 0;
                            // 清空标志位
                            f_h1_flag = 0;
                            f_h_flag = 0;
                            f_fun_word = 0;
                            f_length = 0;

                        }
                        // 把上面下面的判断标志位 == 1去掉


                    }           // 有帧头，有功能字，判断是否是有效字节长度
                    else
                    {
                        if(chrtmp[tnum] <= ValidByteLength)
                        {
                            f_length = chrtmp[tnum];// 记录当前帧的有效字节长度
                            tnum ++;
                        }
                        else
                        {
                            // 清0重新接收
                            tnum = 0;
                            // 清空标志位
                            f_h1_flag = 0;
                            f_h_flag = 0;
                            f_fun_word = 0;
                        }
                    }
                }
                else						// 有帧头，无功能字，判断是否为有效功能字
                {
                    if ((chrtmp[tnum] == FunWord_WF) || chrtmp[tnum] == FunWord_SM)
                    {
                        f_fun_word = chrtmp[tnum];//记录功能字
                        tnum ++;
                    }
                    else
                    {
                        // 清0重新接收
                        tnum = 0;
                        // 清空标志位
                        f_h1_flag = 0;
                        f_h_flag = 0;
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
                        // 这里再添加一个判断，出现 3A 3A 3B xx的情况，如果没有这个判断会重新计数，导致丢帧
                        if(chrtmp[tnum] == Frame_Header1){
                            f_h1_flag = 1;
                            tnum = 1;
                        }else{
                            // 重新计数，但如果出现 3A 3A 3B xx的情况，会导致丢帧，要加上上面的判断
                            f_h1_flag = 0;
                            tnum = 0;
                        }
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

            static int xx=0;
            // 判断多长的数据没有换行符，如果超过2000，会人为向数据接收区添加换行，来保证CPU占用率不会过高，不会导致卡顿
            // 但由于是先插入换行，后插入接收到的数据，所以每一箩数据并不是2000
            if(chrtmp[tnum] != 0x0A){
                ++xx;
                if(xx > 2000){
                    ui->txtRec->appendPlainText("");
                    ui->txtRec->appendPlainText("");
                    xx=0;
                }
            }else{
                xx=0;
            }

            // 大于MaxFrameLength个字节的帧不接收
            if (tnum > (MaxFrameLength - 1) )
            {
                tnum = 0;
                f_h1_flag = 0;
                f_h_flag = 0;
                f_t1_flag = 0;
                //f_fun_word = 0;
                //f_length = 0;
                continue;
            }
        }
    }
}

/*
// 适用于有帧头帧尾、无功能字和有效字段长度的接收
void MainWindow::xFrameDataFilter(QByteArray *str)
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
                        tnum ++;

                        // 接收到一帧有效帧
                        // 用户处理代码 //
                        // 根据接收到的数量，合成帧字段
                        // 调试信息输出，显示有效帧的内容（16进制显示）
                        if(!ui->widget_5->isHidden()){
                            QByteArray str1;
                            for(int i=0; i<tnum; i++)
                            {
                                str1.append(chrtmp[i]);
                            }
                            //ui->txtFrameEffective->appendPlainText(str1.toHex().toUpper());
                            str1 = str1.toHex().toUpper();
                            QString str2;
                            for(int i = 0; i<str1.length (); i+=2)
                            {
                                str2 += str1.mid (i,2);
                                str2 += " ";
                            }
                            ui->txtFrameEffective->appendPlainText(str2);
                        }

                        //  处理完用户代码，重新接收计数 //
                        tnum = 0;
                        // 清空标志位，之前一直忘了
                        f_h1_flag = 0;
                        f_h_flag = 0;
                        f_t1_flag = 0;

                        // 将接收到符合帧定义的帧，原路发送回去 //
                        //return str1;
                        //ui->lineEdit->setText(str1.toHex().toUpper());

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
                        // 这里再添加一个判断，出现 3A 3A 3B xx的情况，如果没有这个判断会重新计数，导致丢帧
                        if(chrtmp[tnum] == Frame_Header1){
                            f_h1_flag = 1;
                            tnum = 1;
                        }else{
                            // 重新计数，但如果出现 3A 3A 3B xx的情况，会导致丢帧，要加上上面的判断
                            f_h1_flag = 0;
                            tnum = 0;
                        }
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
}*/

void MainWindow::on_btnFramaDebug_clicked()
{
    if(ui->widget_5->isHidden()){
        ui->widget_5->show();
        ui->frame_3->show();
    }else{
        ui->widget_5->hide();
        ui->frame_3->hide();
    }
}
