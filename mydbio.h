#ifndef MYDBIO_H
#define MYDBIO_H

#include <qstringlist.h>
#include <qvariant.h>
#include <qsqlquery.h>
#include <QUuid>

class QSqlDatabase;

class  CMyDBIO
{
public:
	CMyDBIO();
	~CMyDBIO();

	static CMyDBIO* GetDBIO();
	///@brief Close the DB
	void Close();

	///@brief Get the the file name of the default database
	static QString GetDefaultDBFileName();
	
	///@brief Get the the file name of the template
	static QString GetTemplateFileName();

	///@brief Get the directory of the colormap
	static QString GetColormapFilePath();

	///@brief Get the directory of the shader
	static QString GetShaderFilePath();

	///@brief Get the object type from ID in the database
	QString GetObjectTypeFromID(const QString& objectID);

	///@brief Connect one data base
	bool ConnectDB(const QString& strFullDBName, bool bInMemory=false);

	///@brief Update or Insert
	bool UpdateInsert(const QString& strTable, const QStringList& strFieldList, const QList<QVariant>& valueList, const QString& strKey);

	///@brief Update DB
	bool Update(const QString& strTable, const QStringList& strFieldList, const QList<QVariant>& valueList, const QString& strKey);

	///@brief Insert DB
	bool Insert(const QString& strTable, const QStringList& strFieldList, const QList<QVariant>& valueList);

	///@breif Select one record
	QSqlQuery Select(const QString& strTable, const QStringList& strFieldList, const QString& strKey);

	///@brief Get the project ID
	QString GetProjectID();

	///@breif Get the children IDs
	QStringList GetChildrenIDs(const QString& objectID);


	///@brief Delete one record
	void Delete(const QString& strTable, const QString& strKey);

	///@brief Delete all related records
	void Delete(const QString& strKey);

	///@brief Transanction
	bool Transaction();

	///@brief Commit
	void Commit();

	///@brief Update or Insert (Support multiple-column primary key)
	bool UpdateInsert(const QString& strTable, const QStringList& strFieldList, const QList<QVariant>& valueList, const QStringList& strListKeyName, const QStringList& strListKeyValue);

	///@brief Update (Support multiple-column primary key)
	bool Update(const QString& strTable, const QStringList& strFieldList, const QList<QVariant>& valueList, const QStringList& strListKeyName, const QStringList& strListKeyValue);
	
	///@breif Select one record (Support multiple-column primary key)
	QSqlQuery Select(const QString& strTable, const QStringList& strFieldList, const QStringList& strListKeyName, const QStringList& strListKeyValue);

	///@brief Get DB
	QSqlDatabase* GetDB() {return m_pDB;}

	///@brief Merge and update database
	static void MergeDB(const QString& strProjectDBFileName);

	///@brief Vacuum the database
	void Vacuum();
	void SetAutoVaccum();


private:
	void _NewDB();

	static QString _GetDBFileName(const QString& strDBName);

private:
	QSqlDatabase* m_pDB;
	static CMyDBIO* ____p;

};

#endif // RLDBIO_H
