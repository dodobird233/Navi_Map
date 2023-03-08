#include "QtTipsDlgView.h"

QtTipsDlgView::QtTipsDlgView(const QString& msg, QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_TranslucentBackground);
	initFrame(msg);
	myTimer = new QTimer(this);
	myTimer->setSingleShot(true);  //定时器执行一次
	connect(myTimer, &QTimer::timeout, this, [=]() {this->close(); });
}

QtTipsDlgView::~QtTipsDlgView() {
	if (this->myTimer) this->myTimer->deleteLater();
}

void QtTipsDlgView::startTimer(int ms) { this->myTimer->start(ms); }

void QtTipsDlgView::initFrame(const QString& msg) { ui.labelMsg->setText(msg); }
