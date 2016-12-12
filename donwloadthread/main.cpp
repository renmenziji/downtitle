

#include <QApplication>
#include <QStringList>
#include <QXmlStreamReader>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <stdio.h>
#include <qjsonobject.h>
#include "downloadmanager.h"
#include <qmessagebox.h>

struct CRun
{
	QString lineEditMaxCount;
	QString lineEditMinCount;
	QString lineEditText;
	QList<MyHrefCount> lstData;
};
CRun ui;


void  load()
{
	QString strFilename = "list.dat";
	QFile file(strFilename);
	if (file.open(QFile::ReadOnly) == false)
	{
		QMessageBox::warning(NULL, "LOAD FAILED", "LOAD FAILED");
		return;
	}


	QByteArray saveData = file.readAll();
	QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));
	//QJsonDocument loadDoc(QJsonDocument::fromBinaryData(saveData));
	QJsonObject firstLevel = loadDoc.object();
	file.close();
	if (firstLevel["jsontype"] != "CMainForm")
	{
		QMessageBox::warning(NULL, "LOAD  CMainForm FAILED", "LOAD CMainForm FAILED");
		return;
	}

	ui.lineEditMaxCount=(QString::number(firstLevel["lineEditMaxCount"].toInt()));
	ui.lineEditMinCount=(QString::number(firstLevel["lineEditMinCount"].toInt()));

	ui.lstData.clear();
	QJsonArray ja = firstLevel["ja"].toArray();
	for (int i = 0; i < ja.count(); i++)
	{
		MyHrefCount h;
		QJsonObject ob = ja[i].toObject();
		h.href = (ob["href"].toString());
		h.count =qMax(1, ob["count"].toInt());
		ui.lstData.append(h);
	}
	return;
}


void slotRun()
{
	load();
	//if (m_runThread->isRunning())
	//{
	//	m_runThread->terminate();
	//	return;
	//}


	//QStringList lstHref;
	//QList<int> maxCount;

	int uiMaxCount = ui.lineEditMaxCount.toInt();
	int i;

	if (uiMaxCount >= 1)
	{
		for (i = 0; i < ui.lstData.count(); i++)
		{
			ui.lstData[i].count = uiMaxCount;
		}
	}


	//QMessageBox::warning(NULL, "startDown", "startDown"+ QString::number(ui.lstData.count()));
	GetProject()->startDown(ui.lstData);
	//DownloadManager manager;
	//manager.append(lstMe);
	QObject::connect(GetProject()->manager, SIGNAL(finished()), qApp, SLOT(quit()));

	//QObject::connect(&manager, SIGNAL(finished()), &app, SLOT(quit()));
}



int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	QStringList arguments = app.arguments();

	if (1)
	{
		slotRun();
		GetProject()->g_ProgressBar->show();
		app.exec();;
		GetProject()->outputHtmlAll(2, -1);
		return 1;
	}

	//QMessageBox::warning(NULL, arguments.join("/"), QString::number(arguments.count()));
	if (arguments.count()==3)
	{
		GetProject()->outputHtmlAll(arguments[1].toInt(), arguments[2].toInt());
	}
	else
	{
		slotRun();
		GetProject()->g_ProgressBar->show();
	}


	app.exec();;
	return  1;
}
