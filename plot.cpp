#include "plot.h"
#include "ui_plot.h"
#include <stdio.h>

Plot::Plot(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Plot)
{
    ui->setupUi(this);
    setWindowTitle("Plot");

    // 给widget绘图控件，设置个别名，方便书写
    pPlot1 = ui->winPlot;
    // 状态栏指针
    sBar = statusBar();

    // 初始化图表1
    QPlot_init(pPlot1);
    // 绘图图表的设置控件初始化，主要用于关联控件的信号槽
    QPlot_widget_init();

    // 创建定时器，用于定时生成曲线坐标点数据
    timer = new QTimer(this);
    timer->setInterval(10);
    connect(timer,SIGNAL(timeout()),this,SLOT(TimeData_Update()));
    //timer->start(10);

    // 关联控件初始化
    ui->txtPointOriginX->setEnabled(false);
    // 图表重绘后，刷新原点坐标和范围
    connect(pPlot1,SIGNAL(afterReplot()),this,SLOT(repPlotCoordinate()));
}

Plot::~Plot()
{
    delete ui;
}


// 绘图图表初始化
void Plot::QPlot_init(QCustomPlot *customPlot)
{
    // 添加曲线名称
    QStringList lineNames;//设置图例的文本
    lineNames << "波形1" << "波形2" << "波形3" << "波形4" << "波形5" << "波形6" << "波形7" << "波形8" << "波形9" << "波形10"\
              << "波形11" << "波形12" << "波形13" << "波形14" << "波形15" << "波形16" << "波形17" << "波形18" << "波形19" << "波形20";
    // 曲线初始颜色
    QColor initColor[20] = {QColor(0,146,152), QColor(162,0,124), QColor(241,175,0), QColor(27,79,147), QColor(229,70,70),\
                           QColor(0,140,94), QColor(178,0,31), QColor(91,189,43), QColor(0,219,219), QColor(172,172,172),\
                           QColor(0,178,191), QColor(197,124,172), QColor(243,194,70), QColor(115,136,193), QColor(245,168,154),\
                           QColor(152,208,185), QColor(223,70,41), QColor(175,215,136), QColor(157,255,255), QColor(0,0,0)};//QColor(255,255,255)};//白色
    // 图表添加20条曲线，并设置初始颜色，和图例名称
    for(int i=0; i<20; i++){
        pCurve[i] = customPlot->addGraph();
        pCurve[i]->setPen(QPen(QColor(initColor[i])));
        pCurve[i]->setName(lineNames.at(i));
    }

    // 设置背景颜色
    customPlot->setBackground(QColor(255,255,255));
    // 设置背景选择框颜色
    ui->btnColourBack->setStyleSheet(QString("border:0px solid;background-color: %1;").arg(QColor(255,255,255).name()));

    // 曲线选择框颜色，与曲线同步颜色。这样写太复杂了，用控件指针数组在下面写过了，记得要在addGraph()之后才有效。
    //ui->btnColourCurve1->setStyleSheet("border:0px solid;background-color:rgb(0,146,152)");
    //ui->btnColourCurve1->setStyleSheet(QString("border:0px solid;background-color: %1;").arg(initColor[0].name()));
    //ui->btnColourCurve20->setStyleSheet(QString("border:0px solid;background-color: %1;").arg(pCurve[]->pen().color().name()));

    // 设置坐标轴名称
    customPlot->xAxis->setLabel("X");
    customPlot->yAxis->setLabel("Y");

    // 设置x,y坐标轴显示范围
    pointCountX = ui->txtPointCountX->text().toUInt();
    pointCountY = ui->txtPointCountY->text().toUInt();
    customPlot->xAxis->setRange(0,pointCountX);
    customPlot->yAxis->setRange(pointCountY/2*-1,pointCountY/2);

    //customPlot->axisRect()->setupFullAxesBox();//四边安装轴并显示
    //customPlot->xAxis->ticker()->setTickOrigin(1);//改变刻度原点为1
    //customPlot->xAxis->setNumberFormat("gbc");//g灵活的格式,b漂亮的指数形式，c乘号改成×
    //customPlot->xAxis->setNumberPrecision(1);//精度1
    customPlot->xAxis->ticker()->setTickCount(ui->txtMainScaleNumX->text().toUInt());//11个主刻度
    customPlot->yAxis->ticker()->setTickCount(ui->txtMainScaleNumY->text().toUInt());//11个主刻度
    customPlot->xAxis->ticker()->setTickStepStrategy(QCPAxisTicker::tssReadability);//可读性优于设置
    customPlot->yAxis->ticker()->setTickStepStrategy(QCPAxisTicker::tssReadability);//可读性优于设置

    // 显示图表的图例
    customPlot->legend->setVisible(true);

    // 设置波形曲线的复选框字体颜色
    //ui->chkVisibleCurve1->setStyleSheet("QCheckBox{color:rgb(255,0,0)}");//设定前景颜色,就是字体颜色

    // 允许用户用鼠标拖动轴范围，以鼠标为中心滚轮缩放，点击选择图形:
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    // 设置鼠标滚轮的缩放倍率，如果不设置默认为0.85，大于1反方向缩放
    //customPlot->axisRect()->setRangeZoomFactor(0.5);
    // 设置鼠标滚轮缩放的轴方向，仅设置垂直轴。垂直轴和水平轴全选使用：Qt::Vertical | Qt::Horizontal
    customPlot->axisRect()->setRangeZoom(Qt::Vertical);
}

// 绘图图表的设置控件初始化，主要用于关联控件的信号槽
void Plot::QPlot_widget_init(void)
{
    // 获取控件指针数组，方便设置时编码书写
    pChkVisibleCurve[0] = ui->chkVisibleCurve1; pBtnColourCurve[0] = ui->btnColourCurve1; pTxtValueCurve[0] = ui->txtValueCurve1; pRdoBoldCurve[0] = ui->rdoBoldCurve1;
    pChkVisibleCurve[1] = ui->chkVisibleCurve2; pBtnColourCurve[1] = ui->btnColourCurve2; pTxtValueCurve[1] = ui->txtValueCurve2; pRdoBoldCurve[1] = ui->rdoBoldCurve2;
    pChkVisibleCurve[2] = ui->chkVisibleCurve3; pBtnColourCurve[2] = ui->btnColourCurve3; pTxtValueCurve[2] = ui->txtValueCurve3; pRdoBoldCurve[2] = ui->rdoBoldCurve3;
    pChkVisibleCurve[3] = ui->chkVisibleCurve4; pBtnColourCurve[3] = ui->btnColourCurve4; pTxtValueCurve[3] = ui->txtValueCurve4; pRdoBoldCurve[3] = ui->rdoBoldCurve4;
    pChkVisibleCurve[4] = ui->chkVisibleCurve5; pBtnColourCurve[4] = ui->btnColourCurve5; pTxtValueCurve[4] = ui->txtValueCurve5; pRdoBoldCurve[4] = ui->rdoBoldCurve5;
    pChkVisibleCurve[5] = ui->chkVisibleCurve6; pBtnColourCurve[5] = ui->btnColourCurve6; pTxtValueCurve[5] = ui->txtValueCurve6; pRdoBoldCurve[5] = ui->rdoBoldCurve6;
    pChkVisibleCurve[6] = ui->chkVisibleCurve7; pBtnColourCurve[6] = ui->btnColourCurve7; pTxtValueCurve[6] = ui->txtValueCurve7; pRdoBoldCurve[6] = ui->rdoBoldCurve7;
    pChkVisibleCurve[7] = ui->chkVisibleCurve8; pBtnColourCurve[7] = ui->btnColourCurve8; pTxtValueCurve[7] = ui->txtValueCurve8; pRdoBoldCurve[7] = ui->rdoBoldCurve8;
    pChkVisibleCurve[8] = ui->chkVisibleCurve9; pBtnColourCurve[8] = ui->btnColourCurve9; pTxtValueCurve[8] = ui->txtValueCurve9; pRdoBoldCurve[8] = ui->rdoBoldCurve9;
    pChkVisibleCurve[9] = ui->chkVisibleCurve10; pBtnColourCurve[9] = ui->btnColourCurve10; pTxtValueCurve[9] = ui->txtValueCurve10; pRdoBoldCurve[9] = ui->rdoBoldCurve10;
    pChkVisibleCurve[10] = ui->chkVisibleCurve11; pBtnColourCurve[10] = ui->btnColourCurve11; pTxtValueCurve[10] = ui->txtValueCurve11; pRdoBoldCurve[10] = ui->rdoBoldCurve11;
    pChkVisibleCurve[11] = ui->chkVisibleCurve12; pBtnColourCurve[11] = ui->btnColourCurve12; pTxtValueCurve[11] = ui->txtValueCurve12; pRdoBoldCurve[11] = ui->rdoBoldCurve12;
    pChkVisibleCurve[12] = ui->chkVisibleCurve13; pBtnColourCurve[12] = ui->btnColourCurve13; pTxtValueCurve[12] = ui->txtValueCurve13; pRdoBoldCurve[12] = ui->rdoBoldCurve13;
    pChkVisibleCurve[13] = ui->chkVisibleCurve14; pBtnColourCurve[13] = ui->btnColourCurve14; pTxtValueCurve[13] = ui->txtValueCurve14; pRdoBoldCurve[13] = ui->rdoBoldCurve14;
    pChkVisibleCurve[14] = ui->chkVisibleCurve15; pBtnColourCurve[14] = ui->btnColourCurve15; pTxtValueCurve[14] = ui->txtValueCurve15; pRdoBoldCurve[14] = ui->rdoBoldCurve15;
    pChkVisibleCurve[15] = ui->chkVisibleCurve16; pBtnColourCurve[15] = ui->btnColourCurve16; pTxtValueCurve[15] = ui->txtValueCurve16; pRdoBoldCurve[15] = ui->rdoBoldCurve16;
    pChkVisibleCurve[16] = ui->chkVisibleCurve17; pBtnColourCurve[16] = ui->btnColourCurve17; pTxtValueCurve[16] = ui->txtValueCurve17; pRdoBoldCurve[16] = ui->rdoBoldCurve17;
    pChkVisibleCurve[17] = ui->chkVisibleCurve18; pBtnColourCurve[17] = ui->btnColourCurve18; pTxtValueCurve[17] = ui->txtValueCurve18; pRdoBoldCurve[17] = ui->rdoBoldCurve18;
    pChkVisibleCurve[18] = ui->chkVisibleCurve19; pBtnColourCurve[18] = ui->btnColourCurve19; pTxtValueCurve[18] = ui->txtValueCurve19; pRdoBoldCurve[18] = ui->rdoBoldCurve19;
    pChkVisibleCurve[19] = ui->chkVisibleCurve20; pBtnColourCurve[19] = ui->btnColourCurve20; pTxtValueCurve[19] = ui->txtValueCurve20; pRdoBoldCurve[19] = ui->rdoBoldCurve20;

    pCmbLineStyle[0] = ui->cmbLineStyle1; pCmbScatterStyle[0] = ui->cmbScatterStyle1;
    pCmbLineStyle[1] = ui->cmbLineStyle2; pCmbScatterStyle[1] = ui->cmbScatterStyle2;
    pCmbLineStyle[2] = ui->cmbLineStyle3; pCmbScatterStyle[2] = ui->cmbScatterStyle3;
    pCmbLineStyle[3] = ui->cmbLineStyle4; pCmbScatterStyle[3] = ui->cmbScatterStyle4;
    pCmbLineStyle[4] = ui->cmbLineStyle5; pCmbScatterStyle[4] = ui->cmbScatterStyle5;
    pCmbLineStyle[5] = ui->cmbLineStyle6; pCmbScatterStyle[5] = ui->cmbScatterStyle6;
    pCmbLineStyle[6] = ui->cmbLineStyle7; pCmbScatterStyle[6] = ui->cmbScatterStyle7;
    pCmbLineStyle[7] = ui->cmbLineStyle8; pCmbScatterStyle[7] = ui->cmbScatterStyle8;
    pCmbLineStyle[8] = ui->cmbLineStyle9; pCmbScatterStyle[8] = ui->cmbScatterStyle9;
    pCmbLineStyle[9] = ui->cmbLineStyle10; pCmbScatterStyle[9] = ui->cmbScatterStyle10;
    pCmbLineStyle[10] = ui->cmbLineStyle11; pCmbScatterStyle[10] = ui->cmbScatterStyle11;
    pCmbLineStyle[11] = ui->cmbLineStyle12; pCmbScatterStyle[11] = ui->cmbScatterStyle12;
    pCmbLineStyle[12] = ui->cmbLineStyle13; pCmbScatterStyle[12] = ui->cmbScatterStyle13;
    pCmbLineStyle[13] = ui->cmbLineStyle14; pCmbScatterStyle[13] = ui->cmbScatterStyle14;
    pCmbLineStyle[14] = ui->cmbLineStyle15; pCmbScatterStyle[14] = ui->cmbScatterStyle15;
    pCmbLineStyle[15] = ui->cmbLineStyle16; pCmbScatterStyle[15] = ui->cmbScatterStyle16;
    pCmbLineStyle[16] = ui->cmbLineStyle17; pCmbScatterStyle[16] = ui->cmbScatterStyle17;
    pCmbLineStyle[17] = ui->cmbLineStyle18; pCmbScatterStyle[17] = ui->cmbScatterStyle18;
    pCmbLineStyle[18] = ui->cmbLineStyle19; pCmbScatterStyle[18] = ui->cmbScatterStyle19;
    pCmbLineStyle[19] = ui->cmbLineStyle20; pCmbScatterStyle[19] = ui->cmbScatterStyle20;

    // 设置颜色选择框的初始背景颜色，与曲线同步颜色
    for(int i=0; i<20; i++){
        pBtnColourCurve[i]->setStyleSheet(QString("border:0px solid;background-color: %1;").arg(QColor(pCurve[i]->pen().color()).name()));
    }

    // 可见性选择框关联
    for(int i=0; i<20; i++){
        connect(pChkVisibleCurve[i], &QCheckBox::clicked, [=](){
            curveSetVisible(pPlot1, pCurve[i], pChkVisibleCurve[i]->checkState());
        });
    }

    // 颜色选择框关联
    for(int i=0; i<20; i++){
        connect(pBtnColourCurve[i], &QPushButton::clicked, [=](){
            curveSetColor(pPlot1, pCurve[i], pBtnColourCurve[i]);
        });
    }

    // 加粗显示多选框关联。尽量别用，会导致CPU使用率升高
    for(int i=0; i<20; i++){
        connect(pRdoBoldCurve[i], &QRadioButton::clicked, [=](){
            curveSetBold(pPlot1, pCurve[i], pRdoBoldCurve[i]->isChecked());
        });
    }

    // 曲线样式选择关联
    for(int i=0; i<20; i++){
        connect(pCmbLineStyle[i], &QComboBox::currentTextChanged, [=](){
            curveSetLineStyle(pPlot1, pCurve[i], pCmbLineStyle[i]->currentIndex());
        });
    }

    // 散点样式选择关联
    for(int i=0; i<20; i++){
        connect(pCmbScatterStyle[i], &QComboBox::currentTextChanged, [=](){
            curveSetScatterStyle(pPlot1, pCurve[i], pCmbScatterStyle[i]->currentIndex()+1);
        });
    }

    //QIcon ssCircleIcon (":/pic/ssCircle.png");
    //ui->cmbScatterStyle1->addItem(ssCircleIcon,"空心圆");
    for(int i=0; i<20; i++){
        pCmbScatterStyle[i]->setIconSize(QSize(25,17)); // 设置图片显示像素大小，不然会默认大小显示会模糊
    }

}

// 定时器溢出处理槽函数。用来生成曲线的坐标数据。
void Plot::TimeData_Update(void)
{
    // 生成坐标数据
    static float f;
    f += 0.01;
    //qDebug() << sin(f)*100;
    // 将坐标数据，传递给曲线
    ShowPlot_TimeDemo(pPlot1, sin(f)*100);
}

// 曲线更新绘图，定时器绘图演示
void Plot::ShowPlot_TimeDemo(QCustomPlot *customPlot, double num)
{
    cnt++;
    // 给曲线添加数据
    for(int i=0; i<10; i++){
        pTxtValueCurve[i]->setText(QString::number(num-i*10,'g',8));// 显示曲线当前值
        pCurve[i]->addData(cnt, num-i*10);
    }
    for(int i=10; i<20; i++){
        pTxtValueCurve[i]->setText(QString::number(num+(i-9)*10,'g',8));// 显示曲线当前值
        pCurve[i]->addData(cnt, num+(i-9)*10);
    }

    // 设置x坐标轴显示范围，使其自适应缩放x轴，x轴最大显示pointCountX个点。与chkTrackX复选框有关
    if(ui->chkTrackX->checkState()){
        //customPlot->xAxis->setRange((pCurve[0]->dataCount()>pointCountX)?(pCurve[0]->dataCount()-pointCountX):0, pCurve[0]->dataCount());
        setAutoTrackX(customPlot);
    }
    // 设置y坐标轴显示范围，使其自适应曲线缩放
    if(ui->chkAdjustY->checkState()){
        setAutoTrackY(customPlot);
    }

    // 更新绘图，这种方式在高填充下太浪费资源。有另一种方式rpQueuedReplot，可避免重复绘图。
    // 最好的方法还是将数据填充、和更新绘图分隔开。将更新绘图单独用定时器更新。例程数据量较少没用单独定时器更新，实际工程中建议大家加上。
    //customPlot->replot();
    customPlot->replot(QCustomPlot::rpQueuedReplot);

    static QTime time(QTime::currentTime());
    double key = time.elapsed()/1000.0; // 开始到现在的时间，单位秒
    ////计算帧数
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 1) // 每1秒求一次平均值
    {
        //状态栏显示帧数和数据总数
        ui->statusbar->showMessage(
            QString("%1 FPS, Total Data points: %2")
            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
            .arg(customPlot->graph(0)->data()->size()+customPlot->graph(1)->data()->size())
            , 0);
        lastFpsKey = key;
        frameCount = 0;
    }

}

// 曲线更新绘图，波形数据绘图
void Plot::ShowPlot_WaveForm(QCustomPlot *customPlot, short value[])
{
    cnt++;
    // 给曲线添加数据
    for(int i=0; i<20; i++){
        //QString strNum = QString::number(num,'g',8);// double类型
        pTxtValueCurve[i]->setText(QString::number(value[i]));// 显示曲线当前值
        pCurve[i]->addData(cnt, value[i]);// 从原值获取数据
        //pCurve[i]->addData(cnt, pTxtValueCurve[i]->text().toShort());// 从输入框获取数据
        // 因为20条线重叠在一起，所以QCustomPlot输入为0时看起来像不显示，隐藏其他后观察单条曲线是可以看到显示的
    }

    // 设置x坐标轴显示范围，使其自适应缩放x轴，x轴最大显示pointCountX个点。与chkTrackX复选框有关
    if(ui->chkTrackX->checkState()){
        //customPlot->xAxis->setRange((pCurve[0]->dataCount()>pointCountX)?(pCurve[0]->dataCount()-pointCountX):0, pCurve[0]->dataCount());
        setAutoTrackX(customPlot);
    }
    // 设置y坐标轴显示范围，使其自适应曲线缩放
    if(ui->chkAdjustY->checkState()){
        setAutoTrackY(customPlot);
    }

    // 更新绘图，这种方式在高填充下太浪费资源。有另一种方式rpQueuedReplot，可避免重复绘图。
    // 最好的方法还是将数据填充、和更新绘图分隔开。将更新绘图单独用定时器更新。例程数据量较少没用单独定时器更新，实际工程中建议大家加上。
    //customPlot->replot();
    customPlot->replot(QCustomPlot::rpQueuedReplot);

    static QTime time(QTime::currentTime());
    double key = time.elapsed()/1000.0; // 开始到现在的时间，单位秒
    ////计算帧数
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 1) // 每1秒求一次平均值
    {
        //状态栏显示帧数和数据总数
        ui->statusbar->showMessage(
            QString("%1 FPS, Total Data points: %2")
            .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
            .arg(customPlot->graph(0)->data()->size()+customPlot->graph(1)->data()->size())
            , 0);
        lastFpsKey = key;
        frameCount = 0;
    }

}

/* 功能：隐藏/显示曲线n
 * QCustomPlot *pPlot：父控件，绘图图表
 * QCPGraph *pCurve：图表的曲线
 * int arg1：曲线的可见性，>0可见，0不可见
 * */
void Plot::curveSetVisible(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1)
{
    if(arg1){
        pCurve->setVisible(true);
    }else{
        pCurve->setVisible(false);
    }
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

/* 功能：弹出颜色对话框，设置曲线n的颜色
 * QCustomPlot *pPlot：父控件，绘图图表
 * QCPGraph *pCurve：图表的曲线
 * QPushButton *btn：曲线颜色选择框的按键，与曲线的颜色同步
 * */
void Plot::curveSetColor(QCustomPlot *pPlot, QCPGraph *pCurve, QPushButton *btn)
{
    // 获取当前颜色
    QColor bgColor(0,0,0);
    //bgColor = btn->palette().color(QPalette::Background);// 由pushButton的背景色获得颜色
    bgColor = pCurve->pen().color();// 由curve曲线获得颜色
    // 以当前颜色打开调色板，父对象，标题，颜色对话框设置项（显示Alpha透明度通道）
    //QColor color = QColorDialog::getColor(bgColor);
    QColor color = QColorDialog::getColor(bgColor, this,
                                     tr("颜色对话框"),
                                     QColorDialog::ShowAlphaChannel);
    // 判断返回的颜色是否合法。若点击x关闭颜色对话框，会返回QColor(Invalid)无效值，直接使用会导致变为黑色。
    if(color.isValid()){
        // 设置选择框颜色
        btn->setStyleSheet(QString("border:0px solid;background-color: %1;").arg(color.name()));
        // 设置曲线颜色
        QPen pen = pCurve->pen();
        pen.setBrush(color);
        pCurve->setPen(pen);
        //pCurve->setPen(QPen(color));
    }
    // 更新绘图
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

/* 功能：加粗显示曲线n
 * QCustomPlot *pPlot：父控件，绘图图表
 * QCPGraph *pCurve：图表的曲线
 * int arg1：曲线的粗细，>0粗，0细
 * */
void Plot::curveSetBold(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1)
{
    // 预先读取曲线的颜色
    QPen pen = pCurve->pen();
    //pen.setBrush(pCurve->pen().color());// 由curve曲线获得颜色

    if(arg1){
        pen.setWidth(3);
        pCurve->setPen(pen);
    }else{
        pen.setWidth(1);
        pCurve->setPen(pen);
    }
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

/* 功能：选择曲线样式（线，点，积）
 * QCustomPlot *pPlot：父控件，绘图图表
 * QCPGraph *pCurve：图表的曲线
 * int arg1：曲线样式（线，点，积）
 * */
void Plot::curveSetLineStyle(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1)
{
    // 设置曲线样式
    //customPlot->graph(19)->setLineStyle(QCPGraph::lsLine); // 数据点通过直线连接
    //customPlot->graph(19)->setLineStyle((QCPGraph::LineStyle)i);//设置线性
    //pCurve->setLineStyle(QCPGraph::LineStyle(arg1));
    pCurve->setLineStyle((QCPGraph::LineStyle)arg1);
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

/* 功能：选择散点样式（空心圆、实心圆、正三角、倒三角）
 * QCustomPlot *pPlot：父控件，绘图图表
 * QCPGraph *pCurve：图表的曲线
 * int arg1：散点样式（空心圆、实心圆、正三角、倒三角）
 * */
void Plot::curveSetScatterStyle(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1)
{
    // 设置散点样式
    //customPlot->graph(19)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5)); // 空心圆
    //pCurve->setScatterStyle(QCPScatterStyle::ScatterShape(arg1)); // 散点样式
    //pCurve->setScatterStyle((QCPScatterStyle::ScatterShape)arg1); // 散点样式
    if(arg1 <= 10){
        pCurve->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)arg1, 5)); // 散点样式
    }else{ // 后面的散点图形略复杂，太小会看不清
        pCurve->setScatterStyle(QCPScatterStyle((QCPScatterStyle::ScatterShape)arg1, 8)); // 散点样式
    }
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

// 图例显示与否
void Plot::on_chkShowLegend_stateChanged(int arg1)
{
    if(arg1){
        // 显示图表的图例
        pPlot1->legend->setVisible(true);
    }else{
        // 不显示图表的图例
        pPlot1->legend->setVisible(false);
    }
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

// 绘图演示-曲线
void Plot::on_chkDrawDemo_stateChanged(int arg1)
{
    if(arg1){
        timer->start(10);
    }else{
        timer->stop();
    }
}

// 设置曲线x轴自动跟随
void Plot::setAutoTrackX(QCustomPlot *pPlot)
{
    pointCountX = ui->txtPointCountX->text().toUInt();
    if(pCurve[0]->dataCount() < pointCountX){
        pPlot->xAxis->setRange(0,pointCountX);
    }else{
        pPlot->xAxis->setRange((pCurve[0]->dataCount()>pointCountX)?(pCurve[0]->dataCount()-pointCountX):0, pCurve[0]->dataCount());
    }
}

// 设置曲线x轴手动设置范围（依照右下角输入框）
void Plot::setManualSettingX(QCustomPlot *pPlot)
{
    pointOriginX = ui->txtPointOriginX->text().toInt();
    pointCountX = ui->txtPointCountX->text().toUInt();
    pPlot->xAxis->setRange(pointOriginX, pointOriginX+pointCountX);
}

// 设置Y轴自适应
void Plot::setAutoTrackY(QCustomPlot *pPlot)
{
    pPlot->graph(0)->rescaleValueAxis();// y轴自适应，可放大可缩小
    for(int i=1; i<20; i++){
        pPlot->graph(i)->rescaleValueAxis(true);// y轴自适应，只能放大
    }
}

// 重新设置X轴显示的点数
void Plot::on_txtPointCountX_returnPressed()
{
    if(ui->chkTrackX->checkState()){
        setAutoTrackX(pPlot1);
    }else{
        setManualSettingX(pPlot1);
    }
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

void Plot::on_txtPointCountY_returnPressed()
{
    pointCountY = ui->txtPointCountY->text().toUInt();
    pPlot1->yAxis->setRange(pointCountY/2*-1,pointCountY/2);
    ui->txtPointOriginY->setText(QString::number(pointCountY/2*-1));
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

void Plot::on_btnColourBack_clicked()
{
    // 获取当前颜色
    QColor bgColor(0,0,0);
    bgColor = ui->btnColourBack->palette().color(QPalette::Background);// 由pushButton的背景色获得颜色
    // 以当前颜色打开调色板，父对象，标题，颜色对话框设置项（显示Alpha透明度通道）
    //QColor color = QColorDialog::getColor(bgColor);
    QColor color = QColorDialog::getColor(bgColor, this,
                                     tr("颜色对话框"),
                                     QColorDialog::ShowAlphaChannel);

    // 判断返回的颜色是否合法。若点击x关闭颜色对话框，会返回QColor(Invalid)无效值，直接使用会导致变为黑色。
    if(color.isValid()){
        // 设置背景颜色
        pPlot1->setBackground(color);
        // 设置背景选择框颜色
        ui->btnColourBack->setStyleSheet(QString("border:0px solid;background-color: %1;").arg(color.name()));
    }
    // 更新绘图
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

void Plot::on_txtPointOriginX_returnPressed()
{
    setManualSettingX(pPlot1);
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

void Plot::on_chkTrackX_stateChanged(int arg1)
{
    if(arg1){
        ui->txtPointOriginX->setEnabled(false);
        setAutoTrackX(pPlot1);
        pPlot1->replot(QCustomPlot::rpQueuedReplot);
    }else{
        ui->txtPointOriginX->setEnabled(true);
    }
}

void Plot::on_chkAdjustY_stateChanged(int arg1)
{
    if(arg1){
        ui->txtPointOriginY->setEnabled(false);
        ui->txtPointCountY->setEnabled(false);
        setAutoTrackY(pPlot1);
        pPlot1->replot(QCustomPlot::rpQueuedReplot);
    }else{
        ui->txtPointOriginY->setEnabled(true);
        ui->txtPointCountY->setEnabled(true);
    }
}

void Plot::on_txtPointOriginY_returnPressed()
{
    pointOriginY = ui->txtPointOriginY->text().toInt();
    pointCountY = ui->txtPointCountY->text().toUInt();
    pPlot1->yAxis->setRange(pointOriginY, pointOriginY+pointCountY);
    qDebug() << pointOriginY << pointCountY;
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

// 每次图表重绘后，都会更新当前显示的原点坐标与范围。与上次不同时才会更新显示，解决有曲线数据时无法输入y的参数的问题
void Plot::repPlotCoordinate()
{
    static int xOrigin, yOrigin, yCount;
    static int xOriginLast, yOriginLast, yCountLast;

    xOrigin = pPlot1->xAxis->range().lower;
    yOrigin = pPlot1->yAxis->range().lower;
    yCount = pPlot1->yAxis->range().size();
    // 与上次不同时才会更新显示，解决有曲线数据时无法输入y的参数的问题
    if(xOriginLast != xOrigin){
        ui->txtPointOriginX->setText(QString::number(xOrigin));
    }
    if(yOriginLast != yOrigin){
        ui->txtPointOriginY->setText(QString::number(yOrigin));
    }
    if(yCountLast != yCount){
        ui->txtPointCountY->setText(QString::number(yCount));
    }
    // 记录历史值
    xOriginLast = xOrigin;
    yOriginLast = yOrigin;
    yCountLast = yCount;
}

// 清空绘图
void Plot::on_btnClearGraphs_clicked()
{
    //pPlot1->clearGraphs(); // 清除图表的所有数据和设置，需要重新设置才能重新绘图
    //pPlot1->clearPlottables(); // 清除图表中所有曲线，需要重新添加曲线才能绘图
    for(int i=0; i<20; i++){
        pPlot1->graph(i)->data().data()->clear(); // 仅仅清除曲线的数据
    }
    cnt = 0;
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

// 设置X轴主刻度个数
void Plot::on_txtMainScaleNumX_returnPressed()
{
    pPlot1->xAxis->ticker()->setTickCount(ui->txtMainScaleNumX->text().toUInt());
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}

// 设置Y轴主刻度个数
void Plot::on_txtMainScaleNumY_returnPressed()
{
    pPlot1->yAxis->ticker()->setTickCount(ui->txtMainScaleNumY->text().toUInt());
    pPlot1->replot(QCustomPlot::rpQueuedReplot);
}
