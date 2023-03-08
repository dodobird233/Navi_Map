#pragma once
#include "ui_QtWidgetsFL.h"
#include <QWidget>
class QtWidgetsFL : public QWidget
{
    Q_OBJECT

public:
    QtWidgetsFL(QWidget *parent = Q_NULLPTR);
    ~QtWidgetsFL();
    QGraphicsView* par;
    void myShow(QGraphicsView*p);
private:
    Ui::QtWidgetsFL ui;
private slots:
    //检查文件、输入站点、线路、地名
    void checkFile();
    void inputLine();
    void inputOrg();
    void inputStop();
};
