#pragma once
#include <QDialog>
#include <QTimer>
#include "ui_QtTipsDlgView.h"

class QtTipsDlgView : public QDialog  //显示地点信息
{
	Q_OBJECT

public:
	QtTipsDlgView(const QString& msg, QWidget* parent = Q_NULLPTR);
	~QtTipsDlgView();
	void startTimer(int);

private:
	Ui::QtTipsDlgView ui;
	QTimer* myTimer;
	void initFrame(const QString& msg);
};