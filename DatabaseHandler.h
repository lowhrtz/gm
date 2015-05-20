#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include <QSqlDatabase>
#include <QVariant>
#include <QString>

class DatabaseHandler
{
public:
    DatabaseHandler(QString systemPath);
    bool openInMemoryDB();
    bool closeInMemoryDB();
    bool createInMemoryTable(QString tableName);
    QString getInMemoryValue(QString tableName, QString settingName);
    bool setInMemoryValue(QString tableName, QString settingName, QString value);
    bool openPersistentDB();
    bool checkPersistentTable(QString tableName);
    bool createPersistentTable(const char *tableName, QList<QString> colDefList);
    int fillPersistentTable(const char *tableName, QList<QList<QVariant> *> dataList);
    int insertRow(const char *tableName, QList<QVariant> *row);
    QList<QList<QVariant> *> getRows(const char *tableName, const char *column, const char *value);
    QList<QList<QVariant> *> getRows(QString tableName, QString column, QString value);
    QList<QList<QVariant> *> getRows(const char *tableName);
    QList<QList<QVariant> *> getRows(QString tableName);
    QList<QString> getDisplayColList(QString tableName, QString displayCol);
    QString getColName(QString tableName, int columnIndex);
    void activateForeignKeys(bool enabled = true);

private:
    QString sanitizeForSQL(QString sqlString);

private:
    QSqlDatabase inMemoryDb;
    QString systemPath;
    QSqlDatabase db;
};

#endif // DATABASEHANDLER_H
