#ifndef PLOT_H
#define PLOT_H

#include <QMainWindow>
#include "qcustomplot.h"

namespace Ui {
class Plot;
}

class Plot : public QMainWindow
{
    Q_OBJECT

public:
    explicit Plot(QWidget *parent = nullptr);
    ~Plot();
    // 绘图控件的指针
    QCustomPlot *pPlot1;

    void ShowPlot_TimeDemo(QCustomPlot *customPlot, double num);
    void ShowPlot_WaveForm(QCustomPlot *customPlot, short value[]);

private slots:
    void TimeData_Update(void);

    void on_chkShowLegend_stateChanged(int arg1);

    void on_chkDrawDemo_stateChanged(int arg1);

    void on_txtPointCountX_returnPressed();

    void on_txtPointCountY_returnPressed();

    void on_btnColourBack_clicked();

    void on_txtPointOriginX_returnPressed();

    void on_chkTrackX_stateChanged(int arg1);

    void on_chkAdjustY_stateChanged(int arg1);

    void on_txtPointOriginY_returnPressed();

    void repPlotCoordinate();

    void on_btnClearGraphs_clicked();

    void on_txtMainScaleNumX_returnPressed();

    void on_txtMainScaleNumY_returnPressed();

private:
    Ui::Plot *ui;

    // 状态栏指针
    QStatusBar *sBar;
    // 定时器指针
    QTimer *timer;
    // 绘图控件中曲线的指针
    QCPGraph *pCurve[20];
    // 绘图框X轴显示的坐标点数
    int pointOriginX=0;
    int pointOriginY=0;
    int pointCountX=0;
    int pointCountY=0;

    double cnt=0;// 当前绘图的X坐标

    // ui界面中，选择曲线可见性的checkBox的指针。方便用指针数组写代码，不然很占地方
    QCheckBox *pChkVisibleCurve[20];
    // ui界面中，选择曲线颜色的pushButton的指针。方便用指针数组写代码，不然很占地方
    QPushButton *pBtnColourCurve[20];
    // ui界面中，曲线当前值的lineEdit的指针。方便用指针数组写代码，不然很占地方
    QLineEdit *pTxtValueCurve[20];
    // ui界面中，选择曲线粗细的radioButton的指针。方便用指针数组写代码，不然很占地方
    QRadioButton *pRdoBoldCurve[20];
    // ui界面中，选择曲线样式的cmbLineStyle的指针。方便用指针数组写代码，不然很占地方
    QComboBox *pCmbLineStyle[20];
    // ui界面中，选择散点样式的cmbScatterStyle的指针。方便用指针数组写代码，不然很占地方
    QComboBox *pCmbScatterStyle[20];

    void QPlot_init(QCustomPlot *customPlot);
    void QPlot_widget_init(void);

    void curveSetVisible(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1);
    void curveSetColor(QCustomPlot *pPlot, QCPGraph *pCurve, QPushButton *btn);
    void curveSetBold(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1);
    void curveSetLineStyle(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1);
    void curveSetScatterStyle(QCustomPlot *pPlot, QCPGraph *pCurve, int arg1);

    void setAutoTrackX(QCustomPlot *pPlot);
    void setManualSettingX(QCustomPlot *pPlot);
    void setAutoTrackY(QCustomPlot *pPlot);
};

#endif // PLOT_H
