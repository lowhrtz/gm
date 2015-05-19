#ifndef DBWINDOW_H
#define DBWINDOW_H

#include "MainWindow.h"
#include "DatabaseHandler.h"
#include <QLabel>
#include <QListWidget>

class DBListWidgetItem : public QListWidgetItem
{

public:
    DBListWidgetItem(QListWidget *parent, QList<QVariant> *row);
    QList<QVariant> *getRow();

private:
    QList<QVariant> *row;
};

class DBWindow : public QMainWindow
{
    Q_OBJECT

public:
    DBWindow(QWidget *parent, DatabaseHandler *db, PythonInterpreter *interpreter, QString tableName);

public:
    //QList<QLineEdit *> entryList;
    QList<QWidget *> entryList;

private:
    QPixmap getPortrait(QString name);

private:
    QLabel *iconLabel;
    PythonInterpreter *interpreter;
    QString tableName;


private slots:
    void selectRecord(QListWidgetItem *recordWidgetItem, QListWidgetItem *previousRecord);
};

#endif // DBWINDOW_H
