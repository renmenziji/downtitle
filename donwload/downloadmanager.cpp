

#include "downloadmanager.h"
#include <qapplication.h>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <stdio.h>
#include <QTextCodec>
#include <QMessageBox>
//#include <QDomDocument>
#include <QJsonObject>
#include <QTimer>
#include <QStack>
#include <QDesktopServices>
#include "mydbio.h"
QUuid GetUniqueGuid();


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
	m_bModified = true;
}

CBaseObject* CBaseObject::GetObject(QUuid id)
{
	if (g_mapAllObject.contains(id))
	{
		return g_mapAllObject[id];
	}
	return NULL;
}



CHtmlNode::CHtmlNode(QString t, QUuid uuid ):CBaseObject(uuid)
{
	_text = t; m_refCount = 0; GetUniqueGuid();
}


bool CHtmlNode::Serialize(bool bSave)
{
	if (!CBaseObject::Serialize(bSave))
		return false;

	QString strTable = "htmlnode";

	QStringList strFieldList;
	strFieldList << "ID";
	strFieldList << "Title";
	strFieldList << "Href";
	strFieldList << "Author";
	strFieldList << "refCount";
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
		vtList.append(QString::number(m_refCount));
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
			int nIndex = 0;

			m_Title = query.value(++nIndex).toString();
			m_Href = query.value(++nIndex).toString();
			m_Author = query.value(++nIndex).toString();
			m_refCount = query.value(++nIndex).toInt();
			m_dateTime.fromString( query.value(++nIndex).toString());

			QString listPair = query.value(++nIndex).toString();
			QStringList strListPair = listPair.split("\r");
			m_lsthref.clear();
			for (int i = 0; i < strListPair.size() / 2; ++i)
			{
				CHtmlhref href0;
				href0.name= strListPair[2 * i];
				href0.href= strListPair[2 * i + 1];

				m_lsthref.append(href0);
			}
		}

		return true;
	}
}


bool CHtmlNode::_parse(int type)
{
	if (_text.isEmpty())
	{
		return true;
	}
	int i;
	m_refCount = -1;
	//lsthref.removeFirst();


	if (type ==0)
	{

		QStringList lsthref = _text.split("<a href=\"", QString::KeepEmptyParts, Qt::CaseInsensitive);
		for (i = 1; i < lsthref.count(); i++)
		{
			CHtmlhref href0;
			QStringList lstText = lsthref[i].split("\"", QString::KeepEmptyParts, Qt::CaseInsensitive);
			if (lstText.count()>0)
			{
				href0.href = lstText.first();
				if (href0.href[0] == '/')
				{
					href0.href.remove(0, 1);
				}
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

					if (check0.size()<2)
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
				m_Href = m_lsthref[i].href;
			}
		}
	}
	else if (type == 1)//tianya
	{
		//QString m_Title;//like tr td...
		//QString m_Author;
		//QString m_Href;//like tr td...
		//QDateTime m_dateTime;
		//int m_refCount;
		//QList<CHtmlhref> m_lsthref;


		//5个<td ,2个href
		QStringList lstTD = _text.split("<td", QString::KeepEmptyParts, Qt::CaseInsensitive);
		for (i = 1; i < lstTD.count(); i++)
		{
			if (i == 1 || i == 2)
			{
				CHtmlhref href0;
				QStringList lsthref = lstTD[i].split("href=\"", QString::KeepEmptyParts, Qt::CaseInsensitive);
				if (lsthref.count() > 1)
				{
					QStringList lstText = lsthref[1].split("\"", QString::KeepEmptyParts, Qt::CaseInsensitive);
					if (lstText.count() > 0)
					{
						href0.href = lstText.first();
						if (href0.href.isEmpty())
						{
							continue;
						}
						if (href0.href[0]=='/')
						{
							href0.href.remove(0, 1);
						}
					}
					else {
						continue;
					}

					lstText = lsthref[1].split(">", QString::KeepEmptyParts, Qt::CaseInsensitive);
					if (lstText.count() > 1)
					{
						href0.name = lstText[1].split("<", QString::KeepEmptyParts, Qt::CaseInsensitive).first();
						if (href0.name.isEmpty())
						{
							continue;
						}
					}
					else {
						continue;
					}
					m_lsthref.append(href0);

					if (i == 1 )
					{
						m_Title = href0.name;
						m_Href = href0.href;
					}
					else
					{ 
						m_Author = href0.name;
					}
				}

			}
			else if (i == 4)
			{
				QStringList lsthref = lstTD[i].split("<", QString::KeepEmptyParts, Qt::CaseInsensitive);
				if (lsthref.count() > 0)
				{
					m_refCount = lsthref.first().remove(">").toInt();
				}
			}
			else if (i == 5)
			{
				QStringList lsthref = lstTD[i].split("<", QString::KeepEmptyParts, Qt::CaseInsensitive);
				if (lsthref.count() > 0)
				{
					m_dateTime.fromString( lsthref.first().remove(">"));
				}
			}
		}
	}

	return true;
}


CHtmlNode* CHtml::GetHtmlNode(QString url)
{
	for (int i = 0; i < m_lstChildren.count(); i++)
	{
		CHtmlNode* p = dynamic_cast<CHtmlNode*>(m_lstChildren[i]);
		if (p && p->m_Href == url)
		{
			return p;
		}
	}
	return NULL;
}
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
			int nIndex = 0;

			m_href = query.value(++nIndex).toString();
			m_host = query.value(++nIndex).toString();
		}

		return true;
	}
}
bool CHtml::parse(QString strText)
{
	int type = 0;
	int i,j;
	if (m_host.contains("tianya"))
	{
		type = 1;
	}

	if (type== 1)//tbody中
	{

		QStringList lstTBody = strText.split("<tbody", QString::KeepEmptyParts, Qt::CaseInsensitive);
		lstTBody.removeFirst();
		for (i = 0; i < lstTBody.count(); i++)
		{
			QStringList lstTextBG = lstTBody[i].split("/tbody>", QString::KeepEmptyParts, Qt::CaseInsensitive);
			if (lstTextBG.count()>0)//这里后剩下 tr
			{
				if (lstTextBG.first().contains("href")==false)//没有链接
				{
					continue;
				}
				QStringList lstTextTr = lstTextBG.first().split("<tr", QString::KeepEmptyParts, Qt::CaseInsensitive);
				lstTextTr.removeFirst();
				for (j = 0; j < lstTextTr.count(); j++)
				{

					QStringList lstText = lstTextTr[j].split("/tr>", QString::KeepEmptyParts, Qt::CaseInsensitive);
					if (lstText.count() > 0)//这里后剩下 tr
					{
						CHtmlNode* pNode = new CHtmlNode(lstText.first());
						pNode->_parse(type);
						if (pNode->m_Title.isEmpty())
						{
							delete pNode;
							continue;
						}
						CHtmlNode* pFind = GetHtmlNode(pNode->m_Href);
						if (pFind)
						{
							pFind->m_bModified = true;
							pFind->m_Title = pNode->m_Title;
							pFind->m_Author = pNode->m_Author;
							pFind->m_dateTime = pNode->m_dateTime;
							pFind->m_refCount = pNode->m_refCount;
							pFind->m_lsthref = pNode->m_lsthref;

							delete pNode;
						}
						else
						{
							m_lstChildren.append(pNode);
						}
					}
				}
			}
		}

	}
	else
	{
		QStringList lstTBody = strText.split("<tbody", QString::KeepEmptyParts, Qt::CaseInsensitive);
		lstTBody.removeFirst();
		for (i = 0; i < lstTBody.count(); i++)
		{
			QStringList lstText = lstTBody[i].split("/tbody>", QString::KeepEmptyParts, Qt::CaseInsensitive);
			if (lstText.count()>0)
			{
				CHtmlNode* pNode = new CHtmlNode(lstText.first());
				pNode->_parse(type);
				if (pNode->m_Title.isEmpty())
				{
					delete pNode;
					continue;
				}
				CHtmlNode* pFind = GetHtmlNode(pNode->m_Href);
				if (pFind)
				{
					pFind->m_lsthref = pNode->m_lsthref;
					delete pNode;
				}
				else
				{
					m_lstChildren.append(pNode);
				}
			}
		}
	}
	return true;
}




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
	manager = new DownloadManager();
	g_ProgressBar = new QProgressBar();
	g_ProgressBar->resize(800, 100);
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

void CHtmlProject::startDown(QList<MyHrefCount> lstData)
{
	m_mapRealHerfOrihref.clear();
	QStringList lstHref;
	int i;
	for ( i = 0; i < lstData.count(); i++)
	{
		if (lstData[i].href.contains("%x%"))
		{
			for (int j = 0; j < lstData[i].count; j++)
			{
				QString str = lstData[i].href;
				str = str.replace("%x%", QString::number(j + 1));
				lstHref.append(str);
				m_mapRealHerfOrihref[str] = lstData[i].href;
			}
		}
		else
		{
			lstHref.append(lstData[i].href);
		}
	}


	manager->append(lstHref);
	QTimer::singleShot(0, manager, SLOT(startNextDownload()));
	//manager->startNextDownload();

}
bool lstrefLess(CHtmlNode* p1, CHtmlNode* p2)
{
	return p1->m_refCount > p2->m_refCount;
}

void CHtmlProject::outputHtmlAll(int order, int minRefCount)//order 0前后顺序 1refcount 2 all refcount
{
	int j;
	CHtmlProject* pProject = GetProject();

	QString strOut;
	switch (order)
	{
	case 1:
		strOut = "byrefcount.html";
		break;
	case 2:
		strOut = "allbyrefcount.html";

		break;
	default:
		strOut = "allnormal.html";
		//qSort(lst.begin(), lst.end(), lstCKEWellCompDataLessThan);
		break;
	}
	int i;

	QString strText;
	QString strTTTTT;
	for (  j = 0; j < pProject->m_lstChildren.count(); j++)
	{
		CHtml* pHtml = dynamic_cast<CHtml*>(pProject->m_lstChildren[j]);
		if (pHtml)
		{

			strText += "-------------------------------------"+pHtml->m_href + "-------------------------------------<br>";


			QList<CHtmlNode*> lst;
			for (i = 0; i< pHtml->m_lstChildren.count(); i++)
			{
				CHtmlNode* pNode = dynamic_cast<CHtmlNode*>(pHtml->m_lstChildren[i]);
				if (pNode)
				{
					lst.append(pNode);
				}
			}
			if (order == 2)
			{
				qSort(lst.begin(), lst.end(), lstrefLess);
			}
			for (i = 0; i < lst.count(); i++)
			{
				CHtmlNode* pNode = lst[i];
				if (pNode->m_refCount<minRefCount)
				{
					continue;
				}
				strTTTTT += pNode->m_Title + "\n";
				//qDebug() << pNode->m_Author;
				//qDebug() << pNode->m_refCount;
				//qDebug() << pNode->m_dateTime.toString();
				strText += QString::number(pNode->m_refCount);
				for (int j = 0; j < pNode->m_lsthref.count(); j++)
				{
					if (pNode->m_lsthref[j].href.contains("http:"))
					{
						strText += "<a href=\"" +  pNode->m_lsthref[j].href + "\"> "
							+ pNode->m_lsthref[j].name
							+ " </a>";
					}
					else
					{
						strText += "<a href=\"" + pHtml->m_host + pNode->m_lsthref[j].href + "\"> "
							+ pNode->m_lsthref[j].name
							+ " </a>";
					}

				}
				strText += "<br>";
			}
		}
	}
	{

		QFile fileOut(strOut);

		fileOut.open(QFile::WriteOnly);
		fileOut.write(strText.toUtf8());
		fileOut.close();
	}

	QDesktopServices::openUrl(QUrl(strOut));

	//{

	//	QFile fileOut("test.txt");

	//	fileOut.open(QFile::WriteOnly);
	//	fileOut.write(strTTTTT.toUtf8());
	//	fileOut.close();
	//}



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



void runThread::stop()
{
	m_bStop = true;
}


void runThread::run()
{
	m_bStop = false;


	GetProject()->startDown(m_lstData);

	while (GetProject()->manager->m_bRuning)
	{
		//sleep(1000);
	}
}

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent), downloadedCount(0), totalCount(0)
{
}

void DownloadManager::append(const QStringList &urlList)
{
	//downloadQueue.clear();
	m_bRuning = true;
    foreach (QString url, urlList)
		downloadQueue.enqueue(QUrl::fromEncoded(url.toLocal8Bit()));
	totalCount = downloadQueue.count();
}
//
//void DownloadManager::append(const QStringList &urlList)
//{
//	m_bRuning = true;
//	foreach(QString url, urlList)
//		append(QUrl::fromEncoded(url.toLocal8Bit()));
//
//	if (downloadQueue.isEmpty())
//		QTimer::singleShot(0, this, SIGNAL(finished()));
//}
//
//void DownloadManager::append(const QUrl &url)
//{
//	m_bRuning = true;
//	if (downloadQueue.isEmpty())
//		QTimer::singleShot(0, this, SLOT(startNextDownload()));
//
//	downloadQueue.enqueue(url);
//	++totalCount;
//}


#include <QDir>
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
	QDir dir;
	if (dir.exists("temp")==false)
	{
		dir.mkdir("temp");
	}
	basename = "temp/" + basename;
    return basename;
}

void DownloadManager::startNextDownload()
{
    if (downloadQueue.isEmpty()) {
		//QMessageBox::warning(NULL, "finished", "finished");
        printf("%d/%d files downloaded successfully\n", downloadedCount, totalCount);
        emit finished();
		m_bRuning = false;
        return;
    }

    QUrl url = downloadQueue.dequeue();

	//QMessageBox::warning(NULL, "startDown", url.toString());
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
	
	GetProject()->g_ProgressBar->setWindowTitle(QString("Downloading %1...%2/%3 ").arg(url.toEncoded().constData()).arg(downloadedCount).arg(totalCount));

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
//	int i;

	QStringList lstCodec = str.split("charset");
	if (lstCodec.count()>1)//取后者
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
	QString urlOri;
	if (GetProject()->m_mapRealHerfOrihref.contains(url) )
	{
		urlOri = GetProject()->m_mapRealHerfOrihref[url];
	}
	else
	{
		urlOri = url;
	}
	CHtml *pHtml = GetProject()->GetorCreateHtml(urlOri);

	pHtml->m_href = urlOri;
	pHtml->m_host = "http://"+url0.host()+"/";
	pHtml->parse(str);
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




	//QString strText;
	//for ( i = 0; i < pHtml->m_lstChildren.count(); i++)
	//{
	//	CHtmlNode* pNode = (CHtmlNode*)pHtml->m_lstChildren[i];
	//	//qDebug() << pNode->m_Author;
	//	//qDebug() << pNode->m_refCount;
	//	//qDebug() << pNode->m_dateTime.toString();
	//	strText += QString::number(pNode->m_refCount);
	//	for (int j = 0; j < pNode->m_lsthref.count(); j++)
	//	{
	//		strText += "<a href=\"" + pHtml->m_host + pNode->m_lsthref[j].href + "\"> "
	//			+ pNode->m_lsthref[j].name
	//			+ " </a>";

	//	}
	//	strText += "<br>";
	//}
	//{

	//QFile fileOut(saveFileName(url0) + "change.html");

	//fileOut.open(QFile::WriteOnly);
	//fileOut.write(strText.toUtf8());
	//fileOut.close();
	//}

	//{

	//	QFile fileOut("all.html");

	//	fileOut.open(QFile::ReadWrite);
	//	fileOut.seek(fileOut.size());
	//	QString strLine = url+"-------------------------------------<br>";
	//	fileOut.write(strLine.toUtf8());
	//	fileOut.write(strText.toUtf8());
	//	fileOut.close();
	//}

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
