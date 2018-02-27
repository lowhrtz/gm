#include "DatabaseHandler.h"
#include <QDebug>
#include <QSqlField>
#include <QSqlRecord>
#include <QRegExp>
#include <QSqlQuery>
#include <QSqlError>

#include <iostream>
using namespace std;

/*
The DatabaseHandler class performs all operations on the embeded database.
*/

/*
Constructor for the DatabaseHandler class.

Parameters: QString - the path to the RPG-system data folder
*/
DatabaseHandler::DatabaseHandler(QString systemPath)
{
    this->systemPath = systemPath;
//    cout << "System Path: " << this->systemPath.toStdString() << endl;

    if(!openInMemoryDB())
    {
        cout << "Error Opening In-Memory Database" << endl;
        exit(1);
    }
    if(!createInMemoryTable("Settings"))
    {
        cout << "Error Creating Table" << endl;
        exit(1);
    }
    if(!openPersistentDB())
    {
        cout << "Error Opening Persistent Database";
    }
    activateForeignKeys();
}

bool DatabaseHandler::openInMemoryDB()
{
    // Find QSLite driver
    inMemoryDb = QSqlDatabase::addDatabase("QSQLITE");
    // This tells Sqlite to save the db to ram
    inMemoryDb.setDatabaseName(":memory:");

    bool dbOpen = inMemoryDb.open();

    //Test Code
//    QSqlQuery q_create = inMemoryDb.exec("CREATE TABLE qdn (id int, name varchar(50))");
//    QSqlQuery q_insert(inMemoryDb);
//    q_insert.prepare("INSERT INTO qdn (id, name) VALUES (:id, :name)");
//    q_insert.bindValue(":id", QVariant::fromValue(1));
//    q_insert.bindValue(":name", "Volker");
//    q_insert.exec();
//    q_insert.bindValue(":id", QVariant::fromValue(2));
//    q_insert.bindValue(":name", "Root");
//    q_insert.exec();
//    QSqlQuery q_select = inMemoryDb.exec("SELECT * FROM qdn");
//    cout << "Table:\nid name" << endl;
//    while(q_select.next())
//    {
//        cout << q_select.value(0).toString().toStdString() << " " << q_select.value(1).toString().toStdString() << endl;
//    }

    // Open databasee
    return dbOpen;
}

bool DatabaseHandler::closeInMemoryDB()
{
    inMemoryDb.close();
    return !inMemoryDb.isOpen();
}

bool DatabaseHandler::createInMemoryTable(QString tableName)
{
    QString createString("CREATE TABLE ");
    createString.append(tableName);
    createString.append(" (id INTEGER PRIMARY KEY, name VARCHAR(50), value CLOB)");
    QSqlQuery createTable = inMemoryDb.exec(createString);
    return createTable.isActive();
}

QString DatabaseHandler::getInMemoryValue(QString tableName, QString settingName)
{
    QString execString("SELECT value FROM ");
    execString.append(tableName);
    execString.append(" WHERE name IS '");
    execString.append(settingName);
    execString.append("'");
//    cout << execString.toStdString() << endl;
    QSqlQuery valueQuery = inMemoryDb.exec(execString);
    if(valueQuery.next())
    {
//        cout << valueQuery.size() << endl;
        QVariant result = valueQuery.value(0);
        return result.toString();
    }

    return QString();
}

bool DatabaseHandler::setInMemoryValue(QString tableName, QString settingName, QString value)
{
    QString execString("INSERT INTO ");
    execString.append(tableName);
    execString.append(" (name, value) VALUES(:settingName, :value)");
//    cout << execString.toStdString() << endl;
    QSqlQuery getValueQuery(inMemoryDb);
    getValueQuery.prepare(execString);
    getValueQuery.bindValue(":settingName", settingName);
    getValueQuery.bindValue(":value", value);
    return getValueQuery.exec();
}

bool DatabaseHandler::openPersistentDB()
{
    QString dbPath(systemPath);
    dbPath.append("/db");
//    cout << "dbPath: " << dbPath.toStdString() << endl;
    QStringList avail_drivers = QSqlDatabase::drivers();
//    foreach(QString driver, avail_drivers) {
//        qInfo("driver: %s", driver.toStdString().data());
//    }
    db = QSqlDatabase::addDatabase("QSQLITE", dbPath);
    db.setDatabaseName(dbPath);
    bool dbOpen = db.open();
    return dbOpen;
}

bool DatabaseHandler::checkPersistentTable(QString tableName) {
    QString queryString("SELECT * FROM ");

    queryString.append(tableName);

    QSqlQuery testQuery(db);
    testQuery.prepare(queryString);
    return  testQuery.exec();
}

bool DatabaseHandler::createPersistentTable(const char *tableName, QList<QString> colDefList)
{
    QSqlQuery deleteTable = QSqlQuery(db);
    QString deleteString("DROP TABLE IF EXISTS ");
    deleteString.append(tableName);
    deleteTable.prepare(deleteString);
    if(!deleteTable.exec())
    {
        qDebug() << deleteTable.lastError();
        cout << deleteTable.lastQuery().toStdString() << endl;
    }

    QString createTableString = QString("CREATE TABLE ");
    createTableString.append(tableName).append(" (");
    for(int i = 0 ; i < colDefList.size() ; i++)
    {
        if(i == colDefList.size() - 1) {
            createTableString.append(colDefList.at(i));
        }
        else {
            createTableString.append(colDefList.at(i)).append(", ");
        }
    }
    createTableString.append(")");
//    cout << createTableString.toStdString() << endl;

    QSqlQuery createTableQuery = db.exec(createTableString);
    bool createReturn = createTableQuery.isActive();
//    db.commit();

    return createReturn;
}

int DatabaseHandler::fillPersistentTable( const char *tableName, QList<QList<QVariant> *> dataList ) {
    int rowCount = 0;
    for( int i = 0 ; i < dataList.size() ; i++ )     {
        QList<QVariant> *record = dataList.at( i );
        int rowID = insertRow( tableName, record );
        if( rowID > 0 ) {
            rowCount++;
        }
    }

    return rowCount;
}

int DatabaseHandler::insertRow(const char *tableName, QList<QVariant> *row)
{
    QString insertString = QString("INSERT INTO ");
    insertString.append(tableName).append(" VALUES (");
    for(int i = 0 ; i < row->size() ; i++)
    {
        insertString.append("?");
        if(i != row->size() -1) insertString.append(", ");
    }
    insertString.append(")");
    QSqlQuery insertQuery(db);
    insertQuery.prepare(insertString);
    for(int i = 0 ; i < row->size() ; i++)
    {
        QVariant datum = row->at(i);
        insertQuery.addBindValue(datum);

    }

    insertQuery.exec();
    int lastInsertID = insertQuery.lastInsertId().toInt();
//    cout << "Insert ID: " << lastInsertID << endl;
    if(lastInsertID < 1) {
        cout << "Error Inserting Row: " << insertQuery.lastError().text().toStdString() << endl;
        cout << "First Column Data: " << row->at(0).toString().toStdString() << endl;
    }
//    db.commit();
    return lastInsertID;
}

bool DatabaseHandler::updateRow( const char *table_name, QStringList cols, const char *where_col, const char *where, QList<QVariant> *row ) {
    QString update_string = QString( "UPDATE ");
    update_string.append( table_name ).append( " SET " );
    for ( int i = 0; i < cols.size(); i++ ) {
        QString col = cols.at( i );
        update_string.append( col ).append( " = ?" );
        if ( i < cols.size() - 1 ) {
            update_string.append(", ");
        }
    }
    update_string.append( " WHERE " ).append( where_col ).append( " = \"" ).append( where ).append( "\"" );
    QSqlQuery update_query( db );
    update_query.prepare( update_string );
    for ( int i = 0; i < row->size(); i++ ) {
        QVariant datum = row->at( i );
        update_query.addBindValue( datum );
    }

    return update_query.exec();
}

bool DatabaseHandler::deleteRow(const char *table_name, const char *where_col, const char *where) {
    QString delete_string = QString( "DELETE FROM " ).append( table_name ).append( " WHERE ");
    delete_string.append( where_col ).append( " = \"" ).append( where ).append( "\"" );
//    qInfo( delete_string.toStdString().data() );
    QSqlQuery delete_query( db );
    delete_query.prepare( delete_string );

    return delete_query.exec();
}

void DatabaseHandler::begin() {
    db.transaction();
}

bool DatabaseHandler::commit() {
    return db.commit();
}

QList<QList<QVariant> *> DatabaseHandler::getRows(const char *tableName, const char *column, const char *value)
{
    QList<QList<QVariant> *> rowList;
    QString tableQueryString("SELECT * FROM ");
    tableQueryString.append(tableName);
    tableQueryString.append(" WHERE ").append(column).append("=");
    tableQueryString.append("'").append(value).append("'");
    //cout << tableQueryString.toStdString().data() << endl;
    QSqlQuery tableQuery(db);
    tableQuery.prepare(tableQueryString);
    tableQuery.setForwardOnly(true);
    tableQuery.exec();
    db.record(QString(tableName));
    while(tableQuery.next())
    {
        QList<QVariant> *row = new QList<QVariant>;
        QSqlRecord record = tableQuery.record();
        for(int i = 0 ; i < record.count() ; i++)
        {
            QVariant item = record.value(i);
            row->append(item);
        }
        rowList.append(row);
    }

    return rowList;
}

QList<QList<QVariant> *> DatabaseHandler::getRows(QString tableName, QString column, QString value)
{
    return getRows(tableName.toStdString().data(), column.toStdString().data(), value.toStdString().data());
}

QList<QList<QVariant> *> DatabaseHandler::getRows(const char *tableName)
{
    QList<QList<QVariant> *> rowList;
    QString tableQueryString("SELECT * FROM ");
    tableQueryString.append(tableName);
    QSqlQuery tableQuery(db);
    tableQuery.setForwardOnly(true);
    tableQuery.prepare(tableQueryString);
    bool query_successful = tableQuery.exec(tableQueryString);
    if(!query_successful) {
        qInfo("Problem with database query: %s", tableQueryString.toStdString().data());
    }
//    qInfo("tableQuery: %s", query_successful ? "true" : "false");
    db.record(QString(tableName));
    while(tableQuery.next())
    {
        QList<QVariant> *row = new QList<QVariant>;
        QSqlRecord record = tableQuery.record();
//        qInfo("record count: %i", record.count());
        for(int i = 0 ; i < record.count() ; i++)
        {
            QVariant item = record.value(i);
//            qInfo("%s:%s", record.fieldName(i).toStdString().data(), item.toString().toStdString().data());
//            cout << record.field(i).value().toString().toStdString() << endl;
            row->append(item);
        }
        rowList.append(row);
    }

    return rowList;
}

QList<QList<QVariant> *> DatabaseHandler::getRows(QString tableName) {
    return getRows(tableName.toStdString().data());
}

QList<QList<QVariant> *> DatabaseHandler::getColRows(const char *tableName, const char *cols, const char *whereCol, const char *value) {
    QList<QList<QVariant> *> rowList;
    QString tableQueryString;
    QTextStream(&tableQueryString) << "SELECT " << cols << " FROM " << tableName;
//    qInfo("whereCol: %s", whereCol);
    if(whereCol != NULL) {
        QTextStream(&tableQueryString) << " WHERE " << whereCol << "='" << value << "'";
    }
    QSqlQuery tableQuery(db);
    tableQuery.setForwardOnly(true);
    tableQuery.prepare(tableQueryString);
    bool query_successful = tableQuery.exec(tableQueryString);
    if(!query_successful) {
        qInfo("Problem with database querty: %s", tableQueryString.toStdString().data());
    }
    db.record(QString(tableName));
    while(tableQuery.next()) {
        QList<QVariant> *row = new QList<QVariant>;
        QSqlRecord record = tableQuery.record();
        for(int i = 0 ; i < record.count() ; i++) {
            QVariant item = record.value(i);
            row->append(item);
        }
        rowList.append(row);
    }
    return rowList;
}

QList<QList<QVariant> *> DatabaseHandler::getColRows(QString tableName, QString cols, QString whereCol, QString value) {
//    char *whereColChar;
    if(whereCol == NULL) {
       return getColRows(tableName.toStdString().data(), cols.toStdString().data());
    }
//    return getColRows(tableName.toStdString().data(), cols.toStdString().data(), whereColChar, value.toStdString().data());
    return getColRows(tableName.toStdString().data(), cols.toStdString().data(), whereCol.toStdString().data(), value.toStdString().data());
}

QList<QSqlRecord> DatabaseHandler::getColRowsAsSqlRecord(const char *tableName, const char *cols, const char *whereCol, const char *value, QList<char *> additionalValues) {
    QList<QSqlRecord> recordList;
    QString tableQueryString;
    if(cols == NULL) {
        cols = "*";
    }
    QTextStream(&tableQueryString) << "SELECT " << cols << " FROM " << tableName;
    if(whereCol != NULL) {
        QTextStream(&tableQueryString) << " WHERE " << whereCol << "='" << value << "'";
        foreach(char *addValue, additionalValues) {
            QTextStream(&tableQueryString) << " OR " << whereCol << "='" << addValue << "'";
        }
    }

//    qInfo("tableQueryString: %s", tableQueryString.toStdString().data());

    QSqlQuery tableQuery(db);
    tableQuery.setForwardOnly(true);
    tableQuery.prepare(tableQueryString);
    bool query_successful = tableQuery.exec(tableQueryString);
    if(!query_successful) {
        qInfo("Problem with database query: %s", tableQueryString.toStdString().data());
    }
    db.record(QString(tableName));
    while(tableQuery.next()) {
        QSqlRecord record = tableQuery.record();
        recordList.append(record);
    }
    return recordList;
}

QList<QSqlRecord> DatabaseHandler::getColRowsAsSqlRecord(QString tableName, QString cols, QString whereCol, QString value, QList<QString> additionalValues) {
    if(cols == NULL) {
        return getColRowsAsSqlRecord(tableName.toStdString().data());
    }
    if(whereCol == NULL) {
        return getColRowsAsSqlRecord(tableName.toStdString().data(), cols.toStdString().data());
    }
    QList<char *> addValueCharList;
    foreach(QString addValueQString, additionalValues) {
        addValueCharList << (char *) addValueQString.toStdString().data();
    }

    return getColRowsAsSqlRecord(tableName.toStdString().data(), cols.toStdString().data(), whereCol.toStdString().data(), value.toStdString().data(), addValueCharList);
}

QList<QString> DatabaseHandler::getDisplayColList(QString tableName, QString displayCol) {
    QList<QString> displayColList;
    QString queryString("SELECT ");

    queryString.append(displayCol).append(" FROM ").append(tableName);


    return displayColList;
}

QString DatabaseHandler::getColName(QString tableName, int columnIndex)
{
    QSqlRecord record = db.record(tableName);
    return record.fieldName(columnIndex);
}

int DatabaseHandler::getColFromName(QString tableName, QString colName)
{
    QSqlRecord record = db.record(tableName);
    return record.indexOf(colName);
}

void DatabaseHandler::activateForeignKeys(bool enabled) {
//    QString pragmaQueryString = "PRAGMA foreign_keys";
//    QSqlQuery pragmaQuery = db.exec(pragmaQueryString);
//    pragmaQuery.next();
//    QSqlRecord answer = pragmaQuery.record();
//    cout << answer.value(0).toString().toInt() << endl;
//    bool foreignKeysOn = answer.value(0).toInt();
//    if(foreignKeysOn) return;
//    cout << "passed the return" << endl;
    QString activationString = "PRAGMA foreign_keys = ON";
    if(!enabled) activationString = "PRAGMA foreign_keys = OFF";
    db.exec(activationString);
//    pragmaQuery = db.exec(pragmaQueryString);
//    pragmaQuery.next();
//    answer = pragmaQuery.record();
//    cout << answer.value(0).toString().toStdString() << endl;
}

QString DatabaseHandler::sanitizeForSQL(QString sqlString)
{
    QRegExp unrepeatedSingleQuote = QRegExp("([^']|^)[']([^']|$)");
    if(!sqlString.contains(unrepeatedSingleQuote))
    {
        return sqlString;
    }

    int matchIdx = sqlString.indexOf(unrepeatedSingleQuote);
//    cout << "Apostrophy Index: " << apostIdx << endl;
    sqlString.insert(matchIdx + 1, "'");
//    cout << "Sanitized String: " << sqlString.toStdString() << endl;

    return sanitizeForSQL(sqlString);
}
