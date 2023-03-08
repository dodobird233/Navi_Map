#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ex5_mapNavi.h"
#include <QGraphicsRectItem>
#include "QtWidgetsFL.h"
#include "routing.h"
#include "SearchLoc.h"
#include <vector>
class QtWidgetsFL;
class ex5_mapNavi : public QMainWindow
{
    Q_OBJECT

public:
    ex5_mapNavi(QWidget *parent = 0);
    ~ex5_mapNavi();

private:
    Ui::ex5_mapNaviClass ui;
	QtWidgetsFL* fl;
	SearchLoc* search_p;//指向搜索界面的指针
	QTimer* m_Timer;
	QGraphicsItemGroup* gItem;
	void deleteItems();
protected:
	void closeEvent(QCloseEvent* event);
private slots:
    void loadmap();//加载地图
    void closewnd();//关闭窗口
    void zdjl();//最短距离
    void zszc();//最少转乘
	void searchLoc();//搜索地点
};

//定义自己的场景MyScene，以便能够捕获鼠标或键盘事件
class MyScene :public QGraphicsScene {
public:
	explicit MyScene(QObject* parent = 0);
	void stopLines(QGraphicsView*);
protected:
	QGraphicsView* qgv;
	void mouseMoveEvent(QGraphicsSceneMouseEvent* event);	//覆盖mousePressEvent以捕获鼠标事件
	void mousePressEvent(QGraphicsSceneMouseEvent* event);	//覆盖mousePressEvent以捕获鼠标事件
};

class MyItem : public QGraphicsRectItem {
private:
	int cx, cy;			//点击时的坐标
	int cf;				//左键点击=1，表示地点，右键点击=2表示终点
	int cs;				//靠近该点的坐标个数
	int bs[60];		//存放站点的站点编号
public:
	MyItem(int x, int y, int f);
	MyItem& operator<<(int s);
	int operator()(int x, int y);
	int& x();
	int& y();
	int& f();
	int& c();
	int& operator[](int);
	int checkAllStops();	//检索所有站点
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
};


