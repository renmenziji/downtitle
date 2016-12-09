

#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QFile>
#include <QObject>
#include <QQueue>
#include <QUuid>
#include <QTime>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QJsonObject>

#include "textprogressbar.h"
#include "mainform.hpp"

class DownloadManager;
class CBaseObject
{
public:
	CBaseObject(QUuid uuid);
	~CBaseObject();

	void AddChild(CBaseObject* p);
	QList<CBaseObject*> m_lstChildren;
	QUuid m_parentID;
	QUuid m_objID;
	static QMap<QUuid, CBaseObject*> g_mapAllObject;

	virtual bool Serialize(bool bSave);
	CBaseObject* GetObject(QUuid id);
	bool m_bModified;

	virtual QString GetObjectType() { return "object"; }
};


struct CHtmlhref
{
	QString name;
	QString href;
};
class CHtmlNode :public CBaseObject
{
public:
	CHtmlNode(QString t, QUuid uuid = 0);

	QString m_Title;//like tr td...
	QString m_Author;
	QString m_Href;//like tr td...
	QDateTime m_dateTime;
	int m_refCount;
	QList<CHtmlhref> m_lsthref;
	//QList<CHtmlNode*> m_lstChildren;	

	bool Serialize(bool bSave);
	virtual QString GetObjectType() { return "CHtmlNode"; }
private:
	QString _text;
	bool _parse();
	QJsonObject _jo;
};

class CHtml :public CBaseObject
{
public:
	CHtml(QUuid uuid = 0);

	QString m_href;
	QString m_host;

	CHtmlNode* GetHtmlNode(QString url);
	virtual QString GetObjectType() { return "CHtml"; }
	bool parse(QString strText);
	bool Serialize(bool bSave);
};

class CHtmlProject :public CBaseObject
{
public:
	CHtmlProject(QUuid uuid = 0);
	bool Load();
	bool Save();

	void startDown(QList<MyHrefCount> lstData);

	void outputHtmlAll(int order);//order 0Ç°ºóË³Ðò 1refcount 2 all refcount

	CHtml* GetorCreateHtml(QString url);

	virtual QString GetObjectType() { return "CHtmlProject"; }
	bool Serialize(bool bSave);

	static CHtmlProject* ____p;
	QMap<QString, QString> m_mapRealHerfOrihref;
	DownloadManager *manager;
};

void RecreateProject();
CHtmlProject* GetProject();

#include <qthread.h>
class runThread:public QThread
{
	Q_OBJECT
public :
	runThread() { m_bStop = false; }
	void stop();
	void run();
	QList<MyHrefCount> m_lstData;
signals:
	void sigRunIndex(int);

private:
	bool m_bStop;
};


class DownloadManager: public QObject
{
    Q_OBJECT
public:
    DownloadManager(QObject *parent = 0);

    void append(const QStringList &urlList);
   // QString saveFileName(const QUrl &url);

	bool m_bRuning;
signals:
    void finished();

public slots:
    void startNextDownload();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();
    void downloadReadyRead();

private:
    QNetworkAccessManager manager;
    QQueue<QUrl> downloadQueue;
    QNetworkReply *currentDownload;
    QFile output;
	QString m_strDownLoad;
    QTime downloadTime;
    TextProgressBar progressBar;
	QString _filename;

    int downloadedCount;
    int totalCount;
};

#endif
