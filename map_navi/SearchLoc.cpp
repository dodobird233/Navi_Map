#include "SearchLoc.h"
#include "ex5_mapNavi.h"
#include <QCompleter>
extern GIS* gis;

SearchLoc::SearchLoc(QMainWindow* par, QWidget* parent): QWidget(parent), begin({ -1,-1 }), end({ -1,-1 })
{
	ui.setupUi(this);
	connect(ui.pushButtonQuit, SIGNAL(clicked()), this, SLOT(close()), Qt::UniqueConnection);
	connect(ui.pushButtonDone, SIGNAL(clicked()), this, SLOT(getstr()), Qt::UniqueConnection);
	connect(ui.pushButtonDone, SIGNAL(clicked()), par, SLOT(showPath()), Qt::UniqueConnection);
	if (gis)//构建用于模糊匹配的词表
	{
		for (int i = 0; i < gis->org_num; i++)
			wordList << gis->orgs[i].org_name();
	}
	QCompleter* MyInforom = new QCompleter(wordList);//自动填充功能
	MyInforom->setFilterMode(Qt::MatchContains);
	ui.lineEditStart->setCompleter(MyInforom);//起始点自动填充
	ui.lineEditEnd->setCompleter(MyInforom);//结束位置自动填充
}
//析构函数
SearchLoc::~SearchLoc()
{
	//就是空的
}
//获取输入字符串
void SearchLoc::getstr()
{
	ui.labelHit->setText("");
	QString s = ui.lineEditStart->text();//起始点字符串
	QString e = ui.lineEditEnd->text();
	if (s.isEmpty() || e.isEmpty()) //输入为空
	{  
		ui.labelHit->setStyleSheet("color: rgb(235, 10, 10)");
		ui.labelHit->setText(QString::fromLocal8Bit("操作提示：请输入起点和终点！"));
		return;
	}

	//查找输入的地点
	for (int i = 0; i < gis->org_num; i++) //遍历容器
	{
		Organization& find1 = gis->orgs[i];
		//找到了就设置坐标
		if (s == find1.org_name()) { begin.setX(find1.x()); begin.setY(find1.y()); }
		if (e == find1.org_name()) { end.setX(find1.x()); end.setY(find1.y()); }
	}

	//查找失败
	if (begin.x() == -1 || end.x() == -1) {
		ui.labelHit->setStyleSheet("color: rgb(235, 10, 10)");
		ui.labelHit->setText(QString::fromLocal8Bit("操作提示：请输入正确的起点和终点！"));
		return;
	}
	ui.labelHit->setText("");
	close();
}

