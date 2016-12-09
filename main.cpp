

#include <QCoreApplication>
#include <QStringList>
#include <QXmlStreamReader>
#include <QFile>
#include "downloadmanager.h"
#include <stdio.h>
//#include "src/gumbo.h"
#include <qjsonobject.h>


//
//void get_title(GumboNode *node)
//{
//	GumboVector *children;
//	int i;
//
//	/* 如果当前节点不是一个元素的话直接返回 */
//	if (node->type != GUMBO_NODE_ELEMENT) return;
//	/* 获取该节点的所有子元素节点 */
//	children = &node->v.element.children;
//
//	/* 检查当前节点的标签是否为TITLE（title)
//	* 如果是则输出该节点下第一个节点的文本内容 */
//	if (node->v.element.tag == GUMBO_TAG_TITLE)
//	{
//		//qDebug() << QString(node->v.text.text);
//		//printf("%s\n", ((GumboNode *)children->data[0])->v.text.text);
//	}
//
//	//qDebug() << QString(node->v.text.text);
//	printf("%s\n", node->v.text.text);
//
//	if (node->v.element.tag == GUMBO_TAG_LINK)
//	{
//		//printf("%s\n", node->v.text.text);
//	}
//	/* 递归该节点下的所有子节点 */
//	for (i = 0; i < children->length; ++i)
//		get_title((GumboNode *)children->data[i]);
//}

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
