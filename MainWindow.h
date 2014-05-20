#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "PythonInterpreter.h"
#include "DatabaseHandler.h"
#include <QMainWindow>
#include <QApplication>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString systemPath, QWidget *parent = 0);
    void center();

signals:
    void clickedDB(QString tableName);

private slots:
    void clickOpen();
    void resetDB();
    void openDBWindow(QString tableName);
    void openCharacterCreationWizard();

private:
    PythonInterpreter *interpreter;
    DatabaseHandler *db;
};

#endif
