

#include <QCoreApplication>
#include <QStringList>
#include <QXmlStreamReader>
#include <QFile>
#include <stdio.h>
#include <qjsonobject.h>

#include "mainform.hpp"
int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	QStringList arguments = app.arguments();

	CMainForm *mainf = new CMainForm(NULL);

	mainf->show();
	
	//QStringList lstMe;
	//lstMe << "http://www.1080.net/forum-2-1.html";
	//lstMe << "http://www.1080.net/forum-2-2.html";

	////////lstMe << "http://finance.sina.com.cn/";
	//lstMe << "http://bbs.a9vg.com/forum-261-1.html";
	//lstMe << "http://bbs.popiano.org/forum-133-1.html";

	//DownloadManager manager;
	//manager.append(lstMe);

	//QObject::connect(&manager, SIGNAL(finished()), &app, SLOT(quit()));


	//QObject::connect(mainf->ui.pushButtonQuit, SIGNAL(clicked()), &app, SLOT(quit()));
	return  app.exec();
}
