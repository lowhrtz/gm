#ifndef PYTHONINTERPRETER_H
#define PYTHONINTERPRETER_H

#ifdef _WIN32
    #include "DatabaseHandler.h"
    #include <Python.h>
    #include <QString>
#else
    #include <Python.h>
    #include "DatabaseHandler.h"
    #include <QString>
#endif

extern "C" PyObject *dice_rollString(PyObject *self, PyObject *args);
extern "C" PyObject *dice_randomInt(PyObject *self, PyObject *args);
static PyMethodDef diceMethods[] = {
    {"rollString", dice_rollString, METH_VARARGS,
         "Return the result of the given dice string."},
    {"randomInt", dice_randomInt, METH_VARARGS,
         "Return random integer from the lower to upper bounds."},
    {NULL, NULL, 0, NULL}
};

class PythonInterpreter
{
public:
    PythonInterpreter(QString systemPath);
    ~PythonInterpreter();
//    static PyObject *dice_rollString(PyObject *self, PyObject *args);
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
    int getMetaReferenceCol(QString metaTableName);
    QString getReferenceIndexName(QString metaTableName);
    PyObject *makeDictFromRow(QList<QVariant> *row, QString tableName, DatabaseHandler *db);
    PyObject *makeDictListFromRows(QList<QList<QVariant> *> rows, QString tableName, DatabaseHandler *db);
    PyObject *makeDictFromSqlRecord(QSqlRecord row);

private:
    QList<QString> getStringList(PyObject *pyListObject);
    QList<QString> getColDefString(PyObject *cols, PyObject *colDefs);
    QList<QList<QVariant> *> getDataList(PyObject *data);
    void finalizePython();

private:
    QString systemPath;
};

#endif // PYTHONINTERPRETER_H
