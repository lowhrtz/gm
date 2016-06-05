#include "PythonInterpreter.h"
#include "ListObject.h"

#include <iostream>
using namespace std;

PythonInterpreter::PythonInterpreter(QString systemPath) {
    this->systemPath = systemPath;
    Py_SetProgramName((char *) "GM");
    QString pythonPath("./pylib:");
    pythonPath.append(systemPath);

#ifdef _WIN32
    int pyPathStringSize = strlen("PYTHONPATH")+1+strlen(pythonPath.toStdString().data())+1;
    char *pyPathString = (char *) malloc(pyPathStringSize);
    sprintf(pyPathString,"PYTHONPATH=%s",pythonPath.toStdString().data());
    putenv(pyPathString);
    putenv("PYTHONDONTWRITEBYTECODE=True");
    putenv("PYTHONIOENCODING=UTF-8");
#else
    setenv("PYTHONPATH", pythonPath.toStdString().data(), 0);
    setenv("PYTHONDONTWRITEBYTECODE", "True", 1);
    setenv("PYTHONIOENCODING", "UTF-8", 1);
#endif
    initPython();
}

void PythonInterpreter::initPython() {
    Py_Initialize();

#ifdef _WIN32
    QString pyPath("pylib;");
    pyPath.append(systemPath);
    PySys_SetPath((char *) pyPath.toStdString().data());
#endif

}

void PythonInterpreter::loadSettings(DatabaseHandler *db) {
    PyObject *settingsModule, *moduleDict, *key, *value, *valueRepr;
    Py_ssize_t pos = 0;
    QString keyString, reprString;

//    initPython();

    settingsModule = PyImport_ImportModule("SystemSettings");
    if(!settingsModule) {
        printf("Module import failed!\n");
//        Py_Finalize();
        PyErr_Print();
        return;
    }

    moduleDict = PyModule_GetDict(settingsModule);
    if(!moduleDict) {
        printf("Error getting module dict.\n");
//        Py_Finalize();
        PyErr_Print();
        return;
    }

    while(PyDict_Next(moduleDict, &pos, &key, &value)) {
        valueRepr = PyObject_Repr(value);
//        printf("Key: %s, Value: %s\n", PyString_AsString(key), PyString_AsString(valueRepr));
        keyString = PyString_AsString(key);
        reprString = PyString_AsString(valueRepr);
        if(!keyString.startsWith("__")) {
            bool set = db->setInMemoryValue("Settings", keyString, reprString);
            if(!set) {
                printf("Error writing to in-memory database!\n");
            }
//            printf("Setting: %s   Value: %s\n",keyString.toAscii().data(), db->getInMemoryValue("Settings", keyString).toAscii().data());
        }
    }

//    Py_DecRef(settingsModule);
//    Py_DecRef(moduleDict);
//    Py_DecRef(key);
//    Py_DecRef(value);
//    Py_DecRef(valueRepr);
//    Py_Finalize();
}

void PythonInterpreter::importTables(DatabaseHandler *db) {
    PyObject *dbModule, *dbDict, *key, *value, *tableType,
            *tableInstance, *getCols, *getColDefs, *getData,
            *tableName;
    Py_ssize_t pos;
    QString keyString;
    QList<QString> colDefsList;
    QList<QList<QVariant>*> dataList;

//    initPython();

    dbModule = PyImport_ImportModule("Db");
    if(!dbModule) {
        printf("Module import failed!\n");
//        Py_Finalize();
        PyErr_Print();
        return;
    }
    dbDict = PyModule_GetDict(dbModule);
    if(!dbDict) {
        printf("Error getting module dict.\n");
//        Py_Finalize();
        PyErr_Print();
        return;
    }

    tableType = PyObject_GetAttrString(dbModule, "Table");
    if(!tableType) {
        printf("Table class is missing!");
//        Py_Finalize();
        PyErr_Print();
        return;
    }

    pos = 0;
    while(PyDict_Next(dbDict, &pos, &key, &value)) {
        keyString = PyString_AsString(key);
        if(PyType_Check(value) &&
                PyType_IsSubtype((PyTypeObject*)value, (PyTypeObject*)tableType) &&
                PyObject_GetAttrString(value, "__name__") != PyObject_GetAttrString(tableType, "__name__")) {
//            printf("Key: %s\n", keyString.toAscii().data());
            tableInstance = PyObject_CallObject(value, NULL);
            getCols = PyObject_CallMethod(tableInstance, (char *) "get_cols", NULL);
            if(!PyList_Check(getCols)) {
                printf("get_cols does not return a list in the %s class.\n", PyString_AsString(PyObject_GetAttrString(value, "__name__")));
//                Py_Finalize();
                PyErr_Print();
                return;
            }
            getColDefs = PyObject_CallMethod(tableInstance, (char *) "get_colDefs", NULL);
            if(!PyList_Check(getColDefs)) {
                printf("get_colDefs does not return a list in the %s class.\n", PyString_AsString(PyObject_GetAttrString(value, "__name__")));
//                Py_Finalize();
                PyErr_Print();
                return;
            }
            getData = PyObject_CallMethod(tableInstance, (char *) "get_data", NULL);
            if(!PyList_Check(getData)) {
                printf("get_data does not return a list in the %s class.\n", PyString_AsString(PyObject_GetAttrString(value, "__name__")));
//                Py_Finalize();
                PyErr_Print();
                return;
            }
            tableName = PyObject_CallMethod(tableInstance, (char *) "__str__", NULL);
            cout << "Table Name: " << PyString_AsString(tableName) << endl;
            if(PyList_Size(getCols) != PyList_Size(getColDefs)) {
                printf("Wrong cols or colDefs list length for %s!\n", PyString_AsString(key));
//                Py_Finalize();
                PyErr_Print();
                return;
            }
            colDefsList = getColDefString(getCols, getColDefs);
            dataList = getDataList(getData);
            db->activateForeignKeys(false);
            db->createPersistentTable(PyString_AsString(tableName), colDefsList);
//            printf("Table, %s, created? %d\n",PyString_AsString(tableName), db->createPersistentTable(PyString_AsString(tableName), colDefsList));
            db->fillPersistentTable(PyString_AsString(tableName), dataList);
            db->activateForeignKeys(true);
        }
    }

//    Py_Finalize();
}

QList<QString> PythonInterpreter::getTableNames()
{
    PyObject *dbModule, *dbDict, *tableType, *key, *value,
             *tableInstance, *tableName;
    Py_ssize_t pos;
    QString keyString, tableNameString;
    QList<QString> tableList;

//    initPython();

    dbModule = PyImport_ImportModule("Db");
    if(!dbModule) {
        printf("Module import failed!\n");
//        Py_Finalize();
        PyErr_Print();
        return tableList;
    }
    dbDict = PyModule_GetDict(dbModule);
    if(!dbDict) {
        printf("Error getting module dict.\n");
//        Py_Finalize();
        PyErr_Print();
        return tableList;
    }
    tableType = PyObject_GetAttrString(dbModule, "Table");
    if(!tableType) {
        printf("Table class is missing!");
//        Py_Finalize();
        PyErr_Print();
        return tableList;
    }

    pos = 0;
    while(PyDict_Next(dbDict, &pos, &key, &value)) {
        keyString = PyString_AsString(key);
        if(PyType_Check(value) &&
                PyType_IsSubtype((PyTypeObject*)value, (PyTypeObject*)tableType) &&
                PyObject_GetAttrString(value, "__name__") != PyObject_GetAttrString(tableType, "__name__")) {
            tableInstance = PyObject_CallObject(value, NULL);
            tableName = PyObject_CallMethod(tableInstance, (char *) "__str__", NULL);
            tableNameString = PyString_AsString(tableName);
            tableList.append(tableNameString);
        }
    }

//    Py_Finalize();
    return tableList;
}

QList<QString> PythonInterpreter::getColList(QString tableName) {
    PyObject *dbModule, *dbDict, *tableType, *key, *value,
            *tableInstance, *cols, *item;
    Py_ssize_t pos, colsSize;
    QList<QString> colList;

    initPython();

    dbModule = PyImport_ImportModule("Db");
    if(!dbModule) {
        printf("Module import failed!\n");
//        Py_Finalize();
        PyErr_Print();
        return colList;
    }
    dbDict = PyModule_GetDict(dbModule);
    if(!dbDict) {
        printf("Error getting module dict.\n");
//        Py_Finalize();
        PyErr_Print();
        return colList;
    }
    tableType = PyObject_GetAttrString(dbModule, "Table");
    if(!tableType) {
        printf("Table class is missing!");
//        Py_Finalize();
        PyErr_Print();
        return colList;
    }

    pos = 0;
    while(PyDict_Next(dbDict, &pos, &key, &value)) {
        if(PyType_Check(value) &&
                PyType_IsSubtype((PyTypeObject*) value, (PyTypeObject*) tableType) &&
                PyObject_GetAttrString(value, "__name__") != PyObject_GetAttrString(tableType, "__name__") &&
                strcmp(PyString_AsString(PyObject_GetAttrString(value, "table_name")), tableName.toStdString().data()) == 0) {
//            printf("Key: %s, Value: %s\n", PyString_AsString(key), PyString_AsString(PyObject_Repr(value)));
            tableInstance = PyObject_CallObject(value, NULL);
            cols = PyObject_CallMethod(tableInstance, (char *) "get_cols", NULL);
            if(!PyList_Check(cols)) {
                printf("get_cols method did not return a list");
//                Py_Finalize();
                PyErr_Print();
                return colList;
            }
            colsSize = PyList_Size(cols);
            for(int i = 0;i < colsSize;i++) {
                item = PyList_GetItem(cols, i);
                QString colString(PyString_AsString(item));
                colList.append(colString);
            }
//            Py_Finalize();
            PyErr_Clear();
            return colList;
        }
    }

//    Py_Finalize();
    PyErr_Clear();
    return colList;
}

QList<QString> PythonInterpreter::getColDefsList(QString tableName) {
    PyObject *dbModule, *dbDict, *tableType, *key, *value,
            *tableInstance, *colDefs, *item;
    Py_ssize_t pos, colDefsSize;
    QList<QString> colDefsList;
//    PyObject *pyDict = initPython();
    initPython();

    dbModule = PyImport_ImportModule("Db");
    if(!dbModule) {
        printf("Module import failed!\n");
//        Py_Finalize();
        PyErr_Print();
        return colDefsList;
    }
    dbDict = PyModule_GetDict(dbModule);
    if(!dbDict) {
        printf("Error getting module dict.\n");
//        Py_Finalize();
        PyErr_Print();
        return colDefsList;
    }
    tableType = PyObject_GetAttrString(dbModule, "Table");
    if(!tableType) {
        printf("Table class is missing!");
//        Py_Finalize();
        PyErr_Print();
        return colDefsList;
    }

    pos = 0;
    while(PyDict_Next(dbDict, &pos, &key, &value)) {
        if(PyType_Check(value) &&
                PyType_IsSubtype((PyTypeObject*) value, (PyTypeObject*) tableType) &&
                PyObject_GetAttrString(value, "__name__") != PyObject_GetAttrString(tableType, "__name__") &&
                strcmp(PyString_AsString(PyObject_GetAttrString(value, "table_name")), tableName.toStdString().data()) == 0) {
//            printf("Key: %s\n", PyString_AsString(key));
            tableInstance = PyObject_CallObject(value, NULL);
            colDefs = PyObject_CallMethod(tableInstance, (char *) "get_colDefs", NULL);
            if(!PyList_Check(colDefs)) {
                printf("get_colDefs method did not return a list");
//                Py_Finalize();
                PyErr_Print();
                return colDefsList;
            }
            colDefsSize = PyList_Size(colDefs);
            for(int i = 0;i < colDefsSize;i++) {
                item = PyList_GetItem(colDefs, i);
                QString colDefsString(PyString_AsString(item));
                colDefsList.append(colDefsString);
            }
//            Py_Finalize();
            PyErr_Clear();
            return colDefsList;
        }
    }

//    Py_Finalize();
    PyErr_Clear();
    return colDefsList;
}

int PythonInterpreter::getDisplayCol(QString tableName) {
    PyObject *dbModule, *dbDict, *tableType, *key, *value,
            *tableInstance, *displayCol;
    Py_ssize_t pos;

//    PyObject *pyDict = initPython();
//    initPython();

    dbModule = PyImport_ImportModule("Db");
    if(!dbModule) {
        printf("Module import failed!\n");
//        Py_Finalize();
        PyErr_Print();
        return 0;
    }
    dbDict = PyModule_GetDict(dbModule);
    if(!dbDict) {
        printf("Error getting module dict.\n");
//        Py_Finalize();
        PyErr_Print();
        return 0;
    }
    tableType = PyObject_GetAttrString(dbModule, "Table");
    if(!tableType) {
        printf("Table class is missing!");
//        Py_Finalize();
        PyErr_Print();
        return 0;
    }

    pos = 0;
    while(PyDict_Next(dbDict, &pos, &key, &value)) {
        if(PyType_Check(value) &&
                PyType_IsSubtype((PyTypeObject*) value, (PyTypeObject*) tableType) &&
                PyObject_GetAttrString(value, "__name__") != PyObject_GetAttrString(tableType, "__name__") &&
                strcmp(PyString_AsString(PyObject_GetAttrString(value, "table_name")), tableName.toStdString().data()) == 0) {
            tableInstance = PyObject_CallObject(value, NULL);
            displayCol = PyObject_CallMethod(tableInstance, (char *)"get_display_col", NULL);
            if(!PyInt_Check(displayCol)) {
                printf("The method get_display_col does not return an integer.\n");
//                Py_Finalize();
                PyErr_Print();
                return 0;
            }
//            Py_Finalize();
            PyErr_Clear();
            return PyInt_AsSsize_t(displayCol);
        }
    }

//    Py_Finalize();
    PyErr_Clear();
    return 0;
}

//int PythonInterpreter::getDisplayColWithoutInit(QString tableName) {
//    PyObject *dbModule, *dbDict, *tableType, *key, *value,
//            *tableInstance, *displayCol;
//    Py_ssize_t pos;

//    PyErr_Clear();
//    dbModule = PyImport_ImportModule("Db");
//    if(!dbModule) {
//        printf("Module import failed!\n");
//        PyErr_Print();
//        return 0;
//    }
//    dbDict = PyModule_GetDict(dbModule);
//    if(!dbDict) {
//        printf("Error getting module dict.\n");
//        PyErr_Print();
//        return 0;
//    }
//    tableType = PyObject_GetAttrString(dbModule, "Table");
//    if(!tableType) {
//        printf("Table class is missing!");
//        PyErr_Print();
//        return 0;
//    }

//    pos = 0;
//    while(PyDict_Next(dbDict, &pos, &key, &value)) {
//        if(PyType_Check(value) &&
//                PyType_IsSubtype((PyTypeObject*) value, (PyTypeObject*) tableType) &&
//                PyObject_GetAttrString(value, "__name__") != PyObject_GetAttrString(tableType, "__name__") &&
//                strcmp(PyString_AsString(PyObject_GetAttrString(value, "table_name")), tableName.toStdString().data()) == 0) {
//            tableInstance = PyObject_CallObject(value, NULL);
//            displayCol = PyObject_CallMethod(tableInstance, (char *)"get_display_col", NULL);
//            if(!PyInt_Check(displayCol)) {
//                printf("The method get_display_col does not return an integer.\n");
//                PyErr_Print();
//                return 0;
//            }
//            return PyInt_AsSsize_t(displayCol);
//        }
//    }

//    return 0;
//}

QString PythonInterpreter::getSystemPath() {
    return systemPath;
}

QList<PyObject *> PythonInterpreter::getWizardPages() {
    QList<PyObject *> pageList;
    PyObject *wizModule, *wizardPageType, *wizardPageList,
            *wizPageListItem;
    Py_ssize_t wizPageListSize;

    wizModule = PyImport_ImportModule("Wizard");
    if(!wizModule) {
        printf("Error importing module!\n");
        PyErr_Print();
//        PyErr_Clear();
        return pageList;
    }

    wizardPageType = PyObject_GetAttrString(wizModule, "WizardPage");
    if(!wizardPageType) {
        printf("WizardPage class is missing!");
        PyErr_Print();
//        PyErr_Clear();
        return pageList;
    }

    wizardPageList = PyObject_GetAttrString(wizModule, "page_order");
    if(!wizardPageList ||
            !PyList_Check(wizardPageList)) {
        printf("Error getting page_order.\n");
        PyErr_Print();
//        PyErr_Clear();
        return pageList;
    }
    wizPageListSize = PyList_Size(wizardPageList);
    for(int i = 0;i < wizPageListSize ; i++) {
        wizPageListItem = PyList_GetItem(wizardPageList, i);
        if(PyType_Check(wizPageListItem) &&
                PyType_IsSubtype((PyTypeObject*) wizPageListItem, (PyTypeObject*) wizardPageType)) {
            pageList.append(wizPageListItem);
        }
    }

    return pageList;
}

QList<QString> PythonInterpreter::settingAsStringList(QString settingName) {
    QList<QString> settingList;
    PyObject *settingsModule, *setting, *settingItem;
    Py_ssize_t settingSize;

    PyErr_Clear();
    settingsModule = PyImport_ImportModule("SystemSettings");
    if(!settingsModule) {
        printf("Error importing SystemSettings module!\n");
        PyErr_Print();
        return settingList;
    }

    setting = PyObject_GetAttrString(settingsModule, settingName.toStdString().data());
    if(!setting ||
            !PyList_Check(setting)) {
        printf("Setting, %s, not a Python list!\n", settingName.toStdString().data());
        PyErr_Print();
        return settingList;
    }

    settingSize = PyList_Size(setting);
    for(int i = 0;i < settingSize;i++) {
        settingItem = PyList_GetItem(setting, i);
        if(!settingItem || !PyString_Check(settingItem)) {
            printf("Setting, %s, contains objects other than strings!\n", settingName.toStdString().data());
            PyErr_Print();
            return settingList;
        }
        settingList.append(PyString_AsString(settingItem));
    }

    return settingList;
}

QList<QString> PythonInterpreter::getMenuOrderedTableNames() {
    QList<QString> tableNameList;
    QString tableName;
    PyObject *dbLayoutModule, *dbOrder, *pyTableName;
    Py_ssize_t nameListSize;

//    initPython();
//    printf("Before PyErr_Clear()\n");
    PyErr_Clear();
    dbLayoutModule = PyImport_ImportModule("DbLayout");
    if(!dbLayoutModule) {
//        Py_Finalize();
        PyErr_Print();
        return getTableNames();
    }

    dbOrder = PyObject_GetAttrString(dbLayoutModule, (char *) "db_order");
    if(!dbOrder || !PyList_Check(dbOrder)) {
//        Py_Finalize();
        PyErr_Print();
        return getTableNames();
    }

    nameListSize = PyList_Size(dbOrder);
    for(int i = 0;i < nameListSize;i++) {
        pyTableName = PyList_GetItem(dbOrder, i);
        if(!pyTableName || !PyString_Check(pyTableName)) continue;
        tableName = PyString_AsString(pyTableName);
        tableNameList.append(tableName);
    }

//    Py_Finalize();
    PyErr_Clear();
    return tableNameList;
}

QString PythonInterpreter::getMetaTableName(QString tableName) {
    QString metaTableName;
    PyObject *dbLayoutModule, *dbMetaMap, *mapTuple;
    Py_ssize_t metaMapSize;

//    initPython();
    PyErr_Clear();
    dbLayoutModule = PyImport_ImportModule("DbLayout");
    if(!dbLayoutModule) {
//        Py_Finalize();
        PyErr_Print();
        return metaTableName;
    }

    dbMetaMap = PyObject_GetAttrString(dbLayoutModule, (char *) "db_meta_map");
    if(!dbMetaMap || !PyList_Check(dbMetaMap)) {
//        Py_Finalize();
        PyErr_Print();
        return metaTableName;
    }

    metaMapSize = PyList_Size(dbMetaMap);
    for(int i = 0;i < metaMapSize;i++) {
        mapTuple = PyList_GetItem(dbMetaMap, i);
        if(!mapTuple || !PyTuple_Check(mapTuple)) continue;
        if(PyString_AsString(PyTuple_GetItem(mapTuple, 0)) == tableName) {
            metaTableName = PyString_AsString(PyTuple_GetItem(mapTuple, 1));
//            Py_Finalize();
            PyErr_Clear();
            return metaTableName;
        }
    }

//    Py_Finalize();
    PyErr_Clear();
    return metaTableName;
}

QList<QString> PythonInterpreter::getStringList(PyObject *pyListObject) {
    if(!PyList_Check(pyListObject)) {
        throw "Not a PyList object!!!";
    }

    QList<QString> qList = QList<QString>();
    int listSize = PyList_Size(pyListObject);
    for(int i = 0 ; i < listSize ; i++) {
        PyObject *item = PyList_GetItem(pyListObject, i);
        if(!PyString_Check(item)) {
            throw "This PyList doesn't contain Strings.";
        }
        QString itemString = PyString_AsString(item);
        qList.append(itemString);
    }

    return qList;
}

QList<QString> PythonInterpreter::getColDefString(PyObject *cols, PyObject *colDefs) {
    QList<QString> colDefStringList = QList<QString>();
    for(int i = 0 ; i < PyList_Size(cols) ; i++) {
        PyObject *colName = PyList_GetItem(cols, i);
        PyObject *colDefName = PyList_GetItem(colDefs, i);
        QString colNameQString(PyString_AsString(colName));
        if(colNameQString.contains("/") ||
                colNameQString.contains("-")) {
            colNameQString.prepend("'");
            colNameQString.append("'");
        }
        QString colDefString = QString(colNameQString);
        colDefString.append(" ").append(PyString_AsString(colDefName));
        colDefStringList.append(colDefString);
//        cout << colDefString.toStdString() << endl;
    }

    return colDefStringList;
}

QList<QList<QVariant> *> PythonInterpreter::getDataList(PyObject *data) {
    QList<QList<QVariant> *> dataList = QList<QList<QVariant> *>();
    for(int i = 0 ; i < PyList_Size(data) ; i++) {
        PyObject *recordPy = PyList_GetItem(data, i);
        QList<QVariant> *record = new QList<QVariant>();
        for(int j = 0 ; j < PyList_Size(recordPy) ; j++) {
            PyObject *datumPy = PyList_GetItem(recordPy, j);
            if(PyInt_Check(datumPy)) {
                QVariant datum = QVariant((int) PyInt_AsSsize_t(datumPy));
                record->append(datum);
            }
            else if (PyString_Check(datumPy)) {
                QVariant datum = QVariant(QObject::trUtf8(PyString_AsString(datumPy)));
                record->append(datum);
            }
            else if (PyBool_Check(datumPy)) {
                QVariant datum = QVariant(datumPy == Py_True);
                record->append(datum);
            }
        }
        dataList.append(record);
    }
    return dataList;
}

void PythonInterpreter::finalizePython()
{
    if (Py_IsInitialized()) Py_Finalize();
}
