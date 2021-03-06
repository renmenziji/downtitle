﻿#pragma once
#include <QWidget>
#include "ui_mainform.h"

class runThread;
class CMainForm : public QWidget {
	Q_OBJECT

public:
	CMainForm(QWidget * parent = Q_NULLPTR);
	~CMainForm();

	Ui::CMainForm ui;
	//runThread *m_runThread;

	void load();
	void save();
	public slots:
	void slotRun();
	void slotQuit();
	void slotOutByCount();

protected:
	void timerEvent(QTimerEvent *event);
	int m_nTimerId;
};
