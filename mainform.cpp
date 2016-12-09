﻿#include "mainform.hpp"
#include "downloadmanager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
CMainForm::CMainForm(QWidget * parent) : QWidget(parent) {
	ui.setupUi(this);

	load();
	//ui.tableWidget->setRowCount(200);

	m_runThread = new runThread();
	QObject::connect(ui.pushButtonRun, SIGNAL(clicked()), this, SLOT(slotRun()));
	QObject::connect(ui.pushButtonQuit, SIGNAL(clicked()), this, SLOT(slotQuit()));
	QObject::connect(ui.pushButtonOutByCount, SIGNAL(clicked()), this, SLOT(slotOutByCount()));
	m_nTimerId = startTimer(3600000);
}

CMainForm::~CMainForm() {
	save();
}

void  CMainForm::load()
{

	QString strFilename = "list.dat";
	QFile file(strFilename);
	if (file.open(QFile::ReadOnly) == false)
	{
		return;
	}


	QByteArray saveData = file.readAll();
	QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
	//QJsonDocument loadDoc(QJsonDocument::fromBinaryData(saveData));
	QJsonObject firstLevel = loadDoc.object();
	file.close();
	if (firstLevel["jsontype"] != "CMainForm")
	{
		return ;
	}

	ui.lineEditMaxCount->setText(QString::number(firstLevel["lineEditMaxCount"].toInt()));
	ui.lineEditMinCount->setText(QString::number(firstLevel["lineEditMinCount"].toInt()));

	QJsonArray ja = firstLevel["ja"].toArray();
	ui.tableWidget->setRowCount(0);
	ui.tableWidget->setRowCount(1000);
	for (int i = 0; i < ja.count(); i++)
	{
		QJsonObject ob = ja[i].toObject();
		ui.tableWidget->setItem(i, 0, new QTableWidgetItem(ob["href"].toString()));
		ui.tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number( ob["count"].toInt())));
	}
	return ;
}
void  CMainForm::save()
{
	QFile file("list.dat");
	if (!file.open(QFile::WriteOnly) )
	{
		return;
	}
	
	QJsonObject firstLevel;
	firstLevel["jsontype"] = "CMainForm";
	firstLevel["lineEditMaxCount"] = ui.lineEditMaxCount->text().toInt();
	firstLevel["lineEditMinCount"] = ui.lineEditMinCount->text().toInt();
	QJsonArray ja;
	int i;
	MyHrefCount h0;
	for (i = 0; i < ui.tableWidget->rowCount(); i++)
	{
		if (ui.tableWidget->item(i, 0))
		{
			h0.href = ui.tableWidget->item(i, 0)->text();
			if (h0.href.isEmpty())
			{
				continue;
			}
			if (ui.tableWidget->item(i, 1))
			{
				h0.count = (qMax(ui.tableWidget->item(i, 1)->text().toInt(), 1));
			}
			else
			{
				h0.count = (1);
			}
			QJsonObject jo;
			jo["href"] = h0.href;
			jo["count"] = h0.count;
			ja.append(jo);
		}

	}
	firstLevel["ja"] = ja;
	QJsonDocument jd(firstLevel);
	file.write(jd.toJson());
	file.close();
}

void CMainForm::slotRun()
{
	if (m_runThread->isRunning())
	{
		m_runThread->terminate();
		return;
	}

	
	QList<MyHrefCount> lstData;
	//QStringList lstHref;
	//QList<int> maxCount;

	int uiMaxCount = ui.lineEditMaxCount->text().toInt();
	int i;
	MyHrefCount h0;
	for ( i = 0; i < ui.tableWidget->rowCount(); i++)
	{
		if (ui.tableWidget->item(i,0))
		{
			h0.href = ui.tableWidget->item(i, 0)->text();
			if (h0.href.isEmpty())
			{
				continue;
			}
			if (uiMaxCount>=1)
			{
				h0.count = uiMaxCount;
			}
			else if (ui.tableWidget->item(i,1))
			{
				h0.count=(qMax(ui.tableWidget->item(i, 1)->text().toInt(),1));
			}
			else
			{
				h0.count =(1);
			}
			lstData.append(h0);
		}
	}

	//m_runThread->m_lstData = lstData;
	//m_runThread->start();

	if (ui.lineEditText->text().isEmpty()==false)
	{
		lstData.clear();
		h0.href = ui.lineEditText->text();
		h0.count = uiMaxCount;
		lstData.append(h0);
	}

	GetProject()->startDown(lstData);
	//DownloadManager manager;
	//manager.append(lstMe);

	//QObject::connect(&manager, SIGNAL(finished()), &app, SLOT(quit()));
}
void CMainForm::slotQuit()
{

	save();
	qApp->quit();
}
void CMainForm::slotOutByCount()
{
	GetProject()->outputHtmlAll(2,ui.lineEditMinCount->text().toInt());
}

void CMainForm::timerEvent(QTimerEvent * event)
{
	qDebug("timer event, id %d", event->timerId());
	slotRun();
}
