

#include "downloadmanager.h"

#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <stdio.h>
#include <QTextCodec>
//#include <QDomDocument>
#include <QJsonObject>
#include <QStack>
#include "mydbio.h"
QUuid GetUniqueGuid();


class CBaseObject
{
public :
	CBaseObject(QUuid uuid);
	~CBaseObject(  );
	
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

QMap<QUuid, CBaseObject*> CBaseObject::g_mapAllObject;

CBaseObject::CBaseObject( QUuid uuid)
	:m_objID(uuid)
{
	if (m_objID == 0)
	{
		m_objID = GetUniqueGuid();
	}
	if (m_objID != QUuid("1"))
	{	
		g_mapAllObject[m_objID] = this;
	}
	m_bModified = true;
}

CBaseObject::~CBaseObject()
{
	if (g_mapAllObject.contains(m_objID))
	{
		g_mapAllObject.remove(m_objID);
	}
}

bool CBaseObject::Serialize(bool bSave)
{

	QString strTable = "object";

	// save
	if (bSave)
	{
		if (!m_bModified)
			return false;

		m_bModified = false;

		QStringList strFieldList;
		strFieldList << "ID";
		strFieldList << "Type";
		strFieldList << "childrenID";
		QList<QVariant> vtList;
		
		vtList.append(m_objID.toString());
		vtList.append(GetObjectType());

		QStringList strChildList;
		for (int i = 0; i<m_lstChildren.size(); ++i)
			strChildList.append(m_lstChildren.at(i)->m_objID.toString());
		QString strChildren = strChildList.join("\r");
		vtList.append(strChildren);

		return CMyDBIO::GetDBIO()->UpdateInsert(strTable, strFieldList, vtList, m_objID.toString());
	}
	else
	{
		m_bModified = false;

		return true;
	}
}

void CBaseObject::AddChild(CBaseObject* p)
{
	p->m_parentID = m_objID;
	if (m_lstChildren.contains(p)==false)
	{
		m_lstChildren.append(p);
	}
}

CBaseObject* CBaseObject::GetObject(QUuid id)
{
	if (g_mapAllObject.contains(id))
	{
		return g_mapAllObject[id];
	}
	return NULL;
}

struct CHtmlhref
{
	QString name;
	QString href;
};
class CHtmlNode:public CBaseObject
{
public:
	CHtmlNode(QString t, QUuid uuid =0);

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


CHtmlNode::CHtmlNode(QString t, QUuid uuid ):CBaseObject(uuid)
{
	_text = t; m_refCount = 0; GetUniqueGuid(); _parse(); 
}


bool CHtmlNode::Serialize(bool bSave)
{
	if (!CBaseObject::Serialize(bSave))
		return false;

	QString strTable = "htmlnode";

	QStringList strFieldList;
	strFieldList << "ID";
	strFieldList << "Title";
	strFieldList << "m_Href";
	strFieldList << "Author";
	strFieldList << "DateTime";
	strFieldList << "NameHref";
	// save
	if (bSave)
	{

		QList<QVariant> vtList;

		vtList.append(m_objID.toString());
		vtList.append(m_Title);
		vtList.append(m_Href);
		vtList.append(m_Author);
		vtList.append(m_dateTime.toString());


		QStringList listPair;
		for (int i = 0; i < m_lsthref.size(); ++i)
		{
			listPair.append(m_lsthref[i].name);
			listPair.append(m_lsthref[i].href);
		}
		vtList.append(listPair.join('\r'));


		return CMyDBIO::GetDBIO()->UpdateInsert(strTable, strFieldList, vtList, m_objID.toString());
	}
	else
	{
		QSqlQuery query = CMyDBIO::GetDBIO()->Select(strTable, strFieldList, m_objID.toString());

		if (query.first())
		{
			int nIndex = -1;

			m_Title = query.value(++nIndex).toString();
			m_Href = query.value(++nIndex).toString();
			m_Author = query.value(++nIndex).toString();
			m_dateTime.fromString( query.value(++nIndex).toString());

			QString listPair = query.value(++nIndex).toString();
			QStringList strListPair = listPair.split("\r");
			m_lsthref.clear();
			for (int i = 0; i < strListPair.size() / 2; ++i)
			{
				CHtmlhref href0;
				href0.name= strListPair[2 * i].toDouble();
				href0.href= strListPair[2 * i + 1].toDouble();

				m_lsthref.append(href0);
			}
		}

		return true;
	}
}


bool CHtmlNode::_parse()
{
	if (_text.isEmpty())
	{
		return true;
	}
	int i;
	QStringList lsthref = _text.split("<a href=\"", QString::KeepEmptyParts, Qt::CaseInsensitive);
	//lsthref.removeFirst();

	for (i = 1; i < lsthref.count(); i++)
	{
		CHtmlhref href0;
		QStringList lstText = lsthref[i].split("\"", QString::KeepEmptyParts, Qt::CaseInsensitive);
		if (lstText.count()>0)
		{
			href0.href = lstText.first();
		}
		else
		{
			continue;
		}
		lstText = lsthref[i].split(">", QString::KeepEmptyParts, Qt::CaseInsensitive);
		if (lstText.count()>1)
		{
			lstText = lstText[1].split("<", QString::KeepEmptyParts, Qt::CaseInsensitive);
			if (lstText.count()>0)
			{
				QString check0 = lstText.first().simplified();

				if (check0.size()<2 )
				{
					continue;
				}
				href0.name = lstText.first();
			}
			else
			{
				continue;
			}
		}
		else
		{
			continue;
		}
		m_lsthref.append(href0);
		if (lsthref[i - 1].contains("\"num\""))
		{
			m_refCount = href0.name.toInt();
		}
		if (lsthref[i].contains("</cite>"))
		{
			m_Author = href0.name;
		}
		QDateTime dt = QDateTime::fromString(href0.name);
		if (dt != QDateTime())
		{
			m_dateTime = dt;
		}
	}

	for (i = 0; i < m_lsthref.count(); i++)
	{
		if (m_Title.size()<m_lsthref[i].name.size())
		{
			m_Title = m_lsthref[i].name;
		}
	}
	return true;
}

class CHtml :public CBaseObject
{
public:
	CHtml(QUuid uuid=0);

	QString m_href;
	QString m_host;

	virtual QString GetObjectType() { return "CHtml"; }
	bool parse(QString strText);
	bool Serialize(bool bSave);
};

CHtml::CHtml(QUuid uuid):CBaseObject(uuid)
{

}

bool CHtml::Serialize(bool bSave)
{
	if (!CBaseObject::Serialize(bSave))
		return false;

	QString strTable = "html";

	QStringList strFieldList;
	strFieldList << "ID";
	strFieldList << "Href";
	strFieldList << "Host";
	// save
	if (bSave)
	{

		QList<QVariant> vtList;

		vtList.append(m_objID.toString());
		vtList.append(m_href);
		vtList.append(m_host);

		return CMyDBIO::GetDBIO()->UpdateInsert(strTable, strFieldList, vtList, m_objID.toString());
	}
	else
	{
		QSqlQuery query = CMyDBIO::GetDBIO()->Select(strTable, strFieldList, m_objID.toString());

		if (query.first())
		{
			int nIndex = -1;

			m_href = query.value(++nIndex).toString();
			m_host = query.value(++nIndex).toString();
		}

		return true;
	}
}
bool CHtml::parse(QString strText)
{
	int i;
	for (i = 0; i < m_lstChildren.count(); i++)
	{
		delete m_lstChildren[i];
	}
	m_lstChildren.clear();

	QStringList lstTBody = strText.split("<tbody", QString::KeepEmptyParts, Qt::CaseInsensitive);
	lstTBody.removeFirst();
	for (i = 0; i < lstTBody.count(); i++)
	{
		QStringList lstText = lstTBody[i].split("/tbody>", QString::KeepEmptyParts, Qt::CaseInsensitive);
		if (lstText.count()>0)
		{
			CHtmlNode* pNode = new CHtmlNode(lstText.first());
			m_lstChildren.append(pNode);
		}
	}
	return true;
}



class CHtmlProject :public CBaseObject
{
public:
	CHtmlProject(QUuid uuid = 0);
	bool Load();
	bool Save();
	
	CHtml* GetorCreateHtml(QString url);

	virtual QString GetObjectType() { return "CHtmlProject"; }
	bool Serialize(bool bSave);

	static CHtmlProject* ____p;
};

void RecreateProject()
{
	delete  CHtmlProject::____p;
	CHtmlProject::____p = NULL;
}
CHtmlProject* GetProject()
{
	if (CHtmlProject::____p == NULL)
	{
		CHtmlProject::____p = new CHtmlProject();
		CHtmlProject::____p->Load();
	}
	return CHtmlProject::____p;
}

CHtmlProject* CHtmlProject::____p =NULL;
CHtmlProject::CHtmlProject(QUuid uuid) :CBaseObject(uuid)
{
	CMyDBIO::GetDBIO();
}

bool CHtmlProject::Serialize(bool bSave)
{
	// base object
	if (!CBaseObject::Serialize(bSave))
		return false;

	QString strTable = "htmlproject";

	// save
	if (bSave)
	{
		QStringList strFieldList;
		strFieldList << "ID";

		QList<QVariant> vtList;

		vtList.append(m_objID.toString());

		return CMyDBIO::GetDBIO()->UpdateInsert(strTable, strFieldList, vtList, m_objID.toString());
	}
	else
	{
		return true;
	}
}
bool CHtmlProject::Load()
{



	// load project
	QString projectID = CMyDBIO::GetDBIO()->GetProjectID();
	if (projectID.isEmpty())
	{
		return false;
	}
	CMyDBIO::GetDBIO()->Transaction();
	this->m_objID = (projectID);

	
	CBaseObject::g_mapAllObject.clear();
	this->Serialize(false);
	CBaseObject::g_mapAllObject.insert(this->m_objID, this);

	
	// load the rest
	QStack<CBaseObject*> stack;
	stack.push(this);

	while (!stack.empty())
	{
		CBaseObject* pParentObj = stack.pop();

		QStringList childrenIDs = CMyDBIO::GetDBIO()->GetChildrenIDs(pParentObj->m_objID.toString());

		for (int i = 0; i<childrenIDs.size(); ++i)
		{
			QString eObjType = CMyDBIO::GetDBIO()->GetObjectTypeFromID(childrenIDs.at(i));

			CBaseObject* pObj = NULL;
			if (eObjType == "CHtmlNode")
			{
				pObj = new CHtmlNode("", QUuid("1"));
			}
			else if (eObjType == "CHtml")
			{
				pObj = new CHtml(QUuid("1"));

			}

			if (!pObj)
				continue;

			pObj->m_objID = (childrenIDs.at(i));

			pObj->Serialize(false);

			pParentObj->AddChild(pObj);

			stack.push(pObj);
		}
	}

	CMyDBIO::GetDBIO()->Commit();

	return true;
}
bool CHtmlProject::Save()
{
	CMyDBIO::GetDBIO()->Transaction();

	QStack<CBaseObject*> stack;
	stack.push(this);

	while (!stack.empty())
	{
		CBaseObject* pObj = stack.pop();

		pObj->Serialize(true);
		
		QList<CBaseObject*> listChildren = pObj->m_lstChildren;

		for (int i = listChildren.size() - 1; i >= 0; --i)
		{
			stack.push(listChildren.at(i));
		}
	}


	CMyDBIO::GetDBIO()->Commit();
	return true;
}

CHtml* CHtmlProject::GetorCreateHtml(QString url)
{
	for (int i = 0; i < m_lstChildren.count(); i++)
	{
		CHtml* p = dynamic_cast<CHtml*>(m_lstChildren[i]);
		if (p && p->m_href == url)
		{
			return p;
		}
	}
	CHtml* p = new CHtml();
	AddChild(p);
	return p;
}

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent), downloadedCount(0), totalCount(0)
{
}

void DownloadManager::append(const QStringList &urlList)
{
    foreach (QString url, urlList)
        append(QUrl::fromEncoded(url.toLocal8Bit()));

    if (downloadQueue.isEmpty())
        QTimer::singleShot(0, this, SIGNAL(finished()));
}

void DownloadManager::append(const QUrl &url)
{
    if (downloadQueue.isEmpty())
        QTimer::singleShot(0, this, SLOT(startNextDownload()));

    downloadQueue.enqueue(url);
    ++totalCount;
}

QString saveFileName(const QUrl &url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();

    if (basename.isEmpty())
        basename = "download";

    //if (QFile::exists(basename)) {
    //    // already exists, don't overwrite
    //    int i = 0;
    //    basename += '.';
    //    while (QFile::exists(basename + QString::number(i)))
    //        ++i;

    //    basename += QString::number(i);
    //}

    return basename;
}

void DownloadManager::startNextDownload()
{
    if (downloadQueue.isEmpty()) {
        printf("%d/%d files downloaded successfully\n", downloadedCount, totalCount);
        emit finished();
        return;
    }

    QUrl url = downloadQueue.dequeue();

	m_strDownLoad = "";
     _filename = saveFileName(url);
    output.setFileName(_filename);
    if (!output.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "Problem opening save file '%s' for download '%s': %s\n",
                qPrintable(_filename), url.toEncoded().constData(),
                qPrintable(output.errorString()));

        startNextDownload();
        return;                 // skip this download
    }

    QNetworkRequest request(url);
    currentDownload = manager.get(request);
    connect(currentDownload, SIGNAL(downloadProgress(qint64,qint64)),
            SLOT(downloadProgress(qint64,qint64)));
    connect(currentDownload, SIGNAL(finished()),
            SLOT(downloadFinished()));
    connect(currentDownload, SIGNAL(readyRead()),
            SLOT(downloadReadyRead()));

    // prepare the output
    printf("Downloading %s...\n", url.toEncoded().constData());
    downloadTime.start();
}

void DownloadManager::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    progressBar.setStatus(bytesReceived, bytesTotal);

    // calculate the download speed
    double speed = bytesReceived * 1000.0 / downloadTime.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bytes/sec";
    } else if (speed < 1024*1024) {
        speed /= 1024;
        unit = "kB/s";
    } else {
        speed /= 1024*1024;
        unit = "MB/s";
    }

    progressBar.setMessage(QString::fromLatin1("%1 %2")
                           .arg(speed, 3, 'f', 1).arg(unit));
    progressBar.update();
}



void parseText(QString strFilename, QString url)
{
	QFile file0(strFilename);
	if (!file0.open(QFile::ReadOnly))
	{
		return;
	}
	//QTextStream ts(&file0);
	//QString str = ts.readAll();
	QString str;
	QByteArray strread = file0.readAll();
	str = strread;


	QTextCodec* tc=NULL;
	int i;

	QStringList lstCodec = str.split("charset");
	if (lstCodec.count()>1)//È¡ºóÕß
	{
		str = lstCodec[1];
		str = str.left(20);
		if (str.contains("gbk"))
		{
			tc = QTextCodec::codecForName("gbk");
		}
		else if (str.contains("utf-8"))
		{
			tc = QTextCodec::codecForName("utf-8");
		}
	}
	
	if (tc)
	{
		//str = tc->fromUnicode(strread);
		str = tc->toUnicode(strread);
	}
	else
	{
		str = strread;
	}


	QUrl url0(url);
	CHtml *pHtml = GetProject()->GetorCreateHtml(url);

	pHtml->parse(str);
	pHtml->m_href = url;
	pHtml->m_host = "http://"+url0.host()+"/";
	GetProject()->AddChild(pHtml);
	GetProject()->Save();
		//for (int i = 0; i < pHtml->m_lstChildren.count(); i++)
		//{
		//	CHtmlNode* pNode = pHtml->m_lstChildren[i];
		//	qDebug() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";

		//	qDebug() << pNode->m_Author;
		//	qDebug() << pNode->m_refCount;
		//	qDebug() << pNode->m_dateTime.toString();
		//	for (int j = 0; j < pNode->m_lsthref.count(); j++)
		//	{
		//		qDebug() << pNode->m_lsthref[j].name<<"\t\t\t\t\t"<< pNode->m_lsthref[j].href;

		//	}

		//	qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<";
		//}




	QString strText;
	for ( i = 0; i < pHtml->m_lstChildren.count(); i++)
	{
		CHtmlNode* pNode = (CHtmlNode*)pHtml->m_lstChildren[i];
		//qDebug() << pNode->m_Author;
		//qDebug() << pNode->m_refCount;
		//qDebug() << pNode->m_dateTime.toString();
		strText += QString::number(pNode->m_refCount);
		for (int j = 0; j < pNode->m_lsthref.count(); j++)
		{
			strText += "<a href=\"" + pHtml->m_host + pNode->m_lsthref[j].href + "\"> "
				+ pNode->m_lsthref[j].name
				+ " </a>";

		}
		strText += "<br>";
	}
	{

	QFile fileOut(saveFileName(url0) + "change.html");

	fileOut.open(QFile::WriteOnly);
	fileOut.write(strText.toUtf8());
	fileOut.close();
	}

	{

		QFile fileOut("all.html");

		fileOut.open(QFile::ReadWrite);
		fileOut.seek(fileOut.size());
		QString strLine = url+"-------------------------------------<br>";
		fileOut.write(strLine.toUtf8());
		fileOut.write(strText.toUtf8());
		fileOut.close();
	}

}

void DownloadManager::downloadFinished()
{
    progressBar.clear();
    output.close();

    if (currentDownload->error()) {
        // download failed
        fprintf(stderr, "Failed: %s\n", qPrintable(currentDownload->errorString()));
		currentDownload->deleteLater();
		startNextDownload();
		return;
    }

    printf("Succeeded.\n");
    ++downloadedCount;

	QString strUrl = currentDownload->url().url();
	parseText(_filename, strUrl);


	currentDownload->deleteLater();
	startNextDownload();
}

void DownloadManager::downloadReadyRead()
{
	QByteArray strRead = currentDownload->readAll();
    output.write(strRead);
	m_strDownLoad += strRead;
}
