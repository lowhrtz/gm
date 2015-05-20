#ifndef DBWINDOW_H
#define DBWINDOW_H

#include "MainWindow.h"
#include "DatabaseHandler.h"
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

class DBListWidgetItem : public QListWidgetItem
{

public:
    DBListWidgetItem(QListWidget *parent, QList<QVariant> *row);
    QList<QVariant> *getRow();

private:
    QList<QVariant> *row;
};

class MetaDBButton : public QPushButton
{
    Q_OBJECT

public:
    MetaDBButton(QString metaTableName, QWidget *parent);

signals:
    void clicked(QString metaTableName, QString record_id);

private slots:
    void reemitClicked();

private:
    QString metaTableName;
};

class MetaDBWindow : public QMainWindow
{
    Q_OBJECT

public:
    MetaDBWindow(QWidget *parent, DatabaseHandler *db, PythonInterpreter *interpreter, QString tableName, QString recordID);

public:
    DatabaseHandler *db;
    PythonInterpreter *interpreter;
    QString tableName;
    QString recordID;
    int referenceCol;
};

class DBWindow : public QMainWindow
{
    Q_OBJECT

public:
    DBWindow(QWidget *parent, DatabaseHandler *db, PythonInterpreter *interpreter, QString tableName);

public:
    //QList<QLineEdit *> entryList;
    QList<QWidget *> entryList;
    int uniqueCol;

private:
    QPixmap getPortrait(QString name);

private:
    QLabel *iconLabel;
    DatabaseHandler *db;
    PythonInterpreter *interpreter;
    QString tableName;

private slots:
    void selectRecord(QListWidgetItem *recordWidgetItem, QListWidgetItem *previousRecord);
    void openMetaDBWindow(QString metaTableName, QString record_id);
};

#endif // DBWINDOW_H
