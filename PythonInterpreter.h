#ifndef PYTHONINTERPRETER_H
#define PYTHONINTERPRETER_H

#include <Python.h>
#include "DatabaseHandler.h"
#include <QString>

class PythonInterpreter
{
public:
    PythonInterpreter(QString systemPath);
    ~PythonInterpreter();
    void initPython();
//    PyObject * getTableClass();
//    void runModule(QString *module);
    void loadSettings(DatabaseHandler *db);
    void importTables(DatabaseHandler *db);
    QList<QString> getTableNames();
    QList<QString> getColList(QString tableName);
    QList<QString> getColDefsList(QString tableName);
    int getDisplayCol(QString tableName);
//    int getDisplayColWithoutInit(QString tableName);
    QString getSystemPath();
    QList<PyObject *> getWizardPages();
    QList<QString> settingAsStringList(QString settingName);
    QList<QString> getMenuOrderedTableNames();
    QString getMetaTableName(QString tableName);

private:
    QList<QString> getStringList(PyObject *pyListObject);
    QList<QString> getColDefString(PyObject *cols, PyObject *colDefs);
    QList<QList<QVariant> *> getDataList(PyObject *data);
    void finalizePython();

private:
    QString systemPath;
};

#endif // PYTHONINTERPRETER_H
