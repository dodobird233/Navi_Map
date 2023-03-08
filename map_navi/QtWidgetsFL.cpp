#include <QFileDialog>
#include <QMessageBox>
#include "QtWidgetsFL.h"
#include "ex5_mapNavi.h"
#include "SearchLoc.h"
#include "routing.h"

//自定义的站点及线路输入界面
QtWidgetsFL::QtWidgetsFL(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);//连接按钮和函数
	connect(ui.pushButtonStop, SIGNAL(clicked()), this, SLOT(inputStop()), Qt::UniqueConnection);
	connect(ui.pushButtonLine, SIGNAL(clicked()), this, SLOT(inputLine()), Qt::UniqueConnection);
	connect(ui.pushButtonOrg, SIGNAL(clicked()), this, SLOT(inputOrg()), Qt::UniqueConnection);
	connect(ui.pushButtonDone, SIGNAL(clicked()), this, SLOT(checkFile()), Qt::UniqueConnection);
	connect(ui.pushButtonQuit, SIGNAL(clicked()), this, SLOT(close()), Qt::UniqueConnection);
}
void QtWidgetsFL::myShow(QGraphicsView* p) {
	par = p;
	show();
}
QtWidgetsFL::~QtWidgetsFL()
{
}
void QtWidgetsFL::inputStop() {//文本框输入函数
	ui.labelHits->setText("");
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("*.txt"));
	ui.textEditStop->setText(fileName);
}
void QtWidgetsFL::inputLine() {//文本框输入函数
	ui.labelHits->setText("");
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "lines", tr("*.txt"));
	ui.textEditLine->setText(fileName);
}
void QtWidgetsFL::inputOrg() {//输入组织文件
	ui.labelHits->setText("");
	QString fname = QFileDialog::getOpenFileName(this, tr("Open File"), ".", tr("*.txt"));
	ui.textEditOrg->setText(fname);
}
void QtWidgetsFL::checkFile() {//输入文件
	QString  fs = ui.textEditStop->toPlainText();
	QString  fl = ui.textEditLine->toPlainText();
	QString  fo = ui.textEditOrg->toPlainText();
	if (fs.isEmpty() && fl.isEmpty() && fo.isEmpty()) {//检查文件存在
		ui.labelHits->setStyleSheet("color: rgb(255, 0, 0)");
		ui.labelHits->setText(QString::fromLocal8Bit("have not input any file!")); //不用fromLocal8Bit显示乱码
		ui.textEditStop->setFocus();
		return;
	}
	if (fs.isEmpty()) {
		ui.labelHits->setStyleSheet("color: rgb(255, 0, 0)");
		ui.labelHits->setText(QString::fromLocal8Bit("have not input stop file!"));
		ui.textEditStop->setFocus();
		return;
	}
	if (fl.isEmpty()) {
		ui.labelHits->setStyleSheet("color: rgb(255, 0, 0)");
		ui.labelHits->setText(QString::fromLocal8Bit("have not input line file!"));
		ui.textEditLine->setFocus();
		return;
	}
	if (fo.isEmpty()) {
		ui.labelHits->setStyleSheet("color: rgb(255, 0, 0)");
		ui.labelHits->setText(QString::fromLocal8Bit("have not input org file!"));
		ui.textEditLine->setFocus();
		return;
	}
	//处理站点文件
	ui.labelHits->setStyleSheet("color: rgb(255, 0, 0)");
	ui.labelHits->setText(QString::fromLocal8Bit("have not input file!"));
	try {//读入站点及线路文件并初始化
		if (gis != 0) delete gis;
		gis = new GIS(fs.toStdString().c_str(), fl.toStdString().c_str(),fo.toStdString().c_str());
		((MyScene*)(par->scene()))->stopLines(par); //在背景地图上画出站点及公交线路
	}
	catch (...) {//读入失败
		gis = 0;
		close();
		QMessageBox::information(NULL, QString::fromLocal8Bit("操作提示"), QString::fromLocal8Bit("公交站点或公交线路文件读入及初始化失败！"));
	}
	ui.labelHits->setText("");
	close();
}