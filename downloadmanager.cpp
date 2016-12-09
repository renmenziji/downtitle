

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


struct CHtmlhref
{
	QString name;
	QString href;
};
class CHtmlNode
{
public:
	CHtmlNode(QString t) { _text = t; m_refCount = 0; _parse(); };

	QString m_Title;//like tr td...
	QString getAttribute(QString);

	QString m_Author;
	QDateTime m_dateTime;
	int m_refCount;
	QList<CHtmlhref> m_lsthref;
	//QList<CHtmlNode*> m_lstChildren;	
private:
	QString _text;
	bool _parse();
	QJsonObject _jo;
};

bool CHtmlNode::_parse()
{
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

				if (check0.isEmpty() || check0 == " ")
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
	return true;
}

class CHtml
{
public:
	CHtml() {};

	QString m_host;
	QList<CHtmlNode*> m_lstRoots;

	bool parse(QString strText);
};

bool CHtml::parse(QString strText)
{
	int i;
	for (i = 0; i < m_lstRoots.count(); i++)
	{
		delete m_lstRoots[i];
	}
	m_lstRoots.clear();

	QStringList lstTBody = strText.split("<tbody", QString::KeepEmptyParts, Qt::CaseInsensitive);
	lstTBody.removeFirst();
	for (i = 0; i < lstTBody.count(); i++)
	{
		QStringList lstText = lstTBody[i].split("/tbody>", QString::KeepEmptyParts, Qt::CaseInsensitive);
		if (lstText.count()>0)
		{
			CHtmlNode* pNode = new CHtmlNode(lstText.first());
			m_lstRoots.append(pNode);
		}
	}
	return true;
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
	bool b;
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
	CHtml html0;
	html0.parse(str);
	html0.m_host = "http://"+url0.host()+"/";
		//for (int i = 0; i < html0.m_lstRoots.count(); i++)
		//{
		//	CHtmlNode* pNode = html0.m_lstRoots[i];
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
	for ( i = 0; i < html0.m_lstRoots.count(); i++)
	{
		CHtmlNode* pNode = html0.m_lstRoots[i];
		//qDebug() << pNode->m_Author;
		//qDebug() << pNode->m_refCount;
		//qDebug() << pNode->m_dateTime.toString();
		strText += QString::number(pNode->m_refCount);
		for (int j = 0; j < pNode->m_lsthref.count(); j++)
		{
			strText += "<a href=\"" + html0.m_host + pNode->m_lsthref[j].href + "\"> "
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
