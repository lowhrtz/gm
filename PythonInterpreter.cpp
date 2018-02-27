#include "PythonInterpreter.h"
#include "ListObject.h"
#include "Dice.h"

#include <QRegularExpression>
#include <QSqlRecord>
#include <iostream>
using namespace std;

DatabaseHandler *db_global;
PythonInterpreter *pi_global;

PythonInterpreter::PythonInterpreter(QString systemPath, DatabaseHandler *db) {
    db_global = db;
    pi_global = this;
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
//    setenv("PYTHONDONTWRITEBYTECODE", "True", 1);
    setenv("PYTHONIOENCODING", "UTF-8", 1);
#endif
    initPython();
}

PythonInterpreter::~PythonInterpreter() {
    finalizePython();
}

//PyObject *PythonInterpreter::dice_rollString(PyObject *self, PyObject *args) {
//    QString diceString;
//    Dice dice;
//    if(!PyArg_ParseTuple(args, "s", diceString)) {
//        return NULL;
//    }
//    return Py_BuildValue("i", dice.rollDice(diceString));
//}

void PythonInterpreter::initPython() {
    Py_DontWriteBytecodeFlag = 1;
    Py_Initialize();
    Py_InitModule( "Dice", diceMethods );
    Py_InitModule( "DbQuery", dbQueryMethods );

#ifdef _WIN32
    QString pyPath("/Python27/Lib;pylib;");
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

    Py_DecRef(settingsModule);
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

    db->activateForeignKeys(false);
    db->begin();
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
            db->createPersistentTable(PyString_AsString(tableName), colDefsList);
//            printf("Table, %s, created? %d\n",PyString_AsString(tableName), db->createPersistentTable(PyString_AsString(tableName), colDefsList));
            db->fillPersistentTable(PyString_AsString(tableName), dataList);
        }
    }
    db->activateForeignKeys(true);
    db->commit();
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
//        Py_Finalize();
        PyErr_Print();
        printf("Db module import failed!\n");
        return colList;
    }
    dbDict = PyModule_GetDict(dbModule);
    if(!dbDict) {
//        Py_Finalize();
        PyErr_Print();
        printf("Error getting Db module dict.\n");
        return colList;
    }
    tableType = PyObject_GetAttrString(dbModule, "Table");
    if(!tableType) {
//        Py_Finalize();
        PyErr_Print();
        printf("Table class is missing!");
        return colList;
    }

    pos = 0;
    while(PyDict_Next(dbDict, &pos, &key, &value)) {
        if(PyType_Check(value) &&
                PyType_IsSubtype((PyTypeObject*) value, (PyTypeObject*) tableType) &&
                PyObject_GetAttrString(value, "__name__") != PyObject_GetAttrString(tableType, "__name__") &&
//                strcmp(PyString_AsString(PyObject_GetAttrString(value, "table_name")), tableName.toStdString().data()) == 0) {
                QString::fromLocal8Bit((const char*) PyString_AsString(PyObject_GetAttrString(value, "table_name"))).toLower() == tableName.toLower()) {
//            printf("Key: %s, Value: %s\n", PyString_AsString(key), PyString_AsString(PyObject_Repr(value)));
            tableInstance = PyObject_CallObject(value, NULL);
            cols = PyObject_CallMethod(tableInstance, (char *) "get_cols", NULL);
            if(!PyList_Check(cols)) {
//                Py_Finalize();
                PyErr_Print();
                printf("get_cols method did not return a list");
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
//        Py_Finalize();
        PyErr_Print();
        printf("Db module import failed!\n");
        return colDefsList;
    }
    dbDict = PyModule_GetDict(dbModule);
    if(!dbDict) {
//        Py_Finalize();
        printf("Error getting Db module dict.\n");
        PyErr_Print();
        return colDefsList;
    }
    tableType = PyObject_GetAttrString(dbModule, "Table");
    if(!tableType) {
//        Py_Finalize();
        PyErr_Print();
        printf("Table class is missing!");
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
//                Py_Finalize();
                PyErr_Print();
                printf("get_colDefs method did not return a list");
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
//        Py_Finalize();
        PyErr_Print();
        printf("Db module import failed!\n");
        return 0;
    }
    dbDict = PyModule_GetDict(dbModule);
    if(!dbDict) {
//        Py_Finalize();
        PyErr_Print();
        printf("Error getting Db module dict.\n");
        return 0;
    }
    tableType = PyObject_GetAttrString(dbModule, "Table");
    if(!tableType) {
//        Py_Finalize();
        PyErr_Print();
        printf("Table class is missing!");
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
//                Py_Finalize();
                PyErr_Print();
                printf("The method get_display_col does not return an integer.\n");
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

std::pair< int, int > PythonInterpreter::getBase64ImageAndTypeCols( QString tableName ) {
    PyObject *dbModule, *dbDict, *tableType, *key, *value,
            *tableInstance, *base_64_image_col_obj, *image_type_col_obj;
    Py_ssize_t pos, base_64_image_col, image_type_col;
    std::pair< int, int > cols( -1, -1 );

    dbModule = PyImport_ImportModule( "Db" );
    if(!dbModule) {
        PyErr_Print();
        qInfo( "Db module import failed!\n" );
        return cols;
    }
    dbDict = PyModule_GetDict( dbModule );
    if( !dbDict ) {
        PyErr_Print();
        qInfo( "Error getting Db module dict.\n" );
        return cols;
    }
    tableType = PyObject_GetAttrString( dbModule, "Table" );
    if( !tableType ) {
        PyErr_Print();
        qInfo( "Table class is missing!" );
        return cols;
    }

    pos = 0;
    while( PyDict_Next( dbDict, &pos, &key, &value ) ) {
        if( PyType_Check( value ) &&
                PyType_IsSubtype( (PyTypeObject*) value, (PyTypeObject*) tableType ) &&
                PyObject_GetAttrString( value, "__name__" ) != PyObject_GetAttrString( tableType, "__name__" ) &&
                strcmp( PyString_AsString( PyObject_GetAttrString( value, "table_name" ) ), tableName.toStdString().data() ) == 0 ) {
            tableInstance = PyObject_CallObject( value, NULL );
            base_64_image_col_obj = PyObject_CallMethod( tableInstance, (char *) "get_base64_image_col", NULL );
            PyErr_Print();
            if ( base_64_image_col_obj == Py_None ) {
                base_64_image_col = -1;
            } else {
                base_64_image_col = PyInt_AsSsize_t( base_64_image_col_obj );
            }
            image_type_col_obj = PyObject_CallMethod( tableInstance, (char *) "get_image_type_col", NULL );
            PyErr_Print();
            if ( image_type_col_obj == Py_None ) {
                image_type_col == -1;
            } else {
                image_type_col = PyInt_AsSsize_t( image_type_col_obj );
            }
            cols.first = base_64_image_col;
            cols.second = image_type_col;
            return cols;
        }
    }

    return cols;
}

QString PythonInterpreter::getSystemPath() {
    return systemPath;
}

QList<PyObject *> PythonInterpreter::getWizardPages() {
    QList<PyObject *> pageList;
    PyObject *wizardPageType, *wizModuleDict, *key, *value, *wizardPageList,
            *wizPageListItem;
    Py_ssize_t pos = 0, wizPageListSize;
    QString keyString;

    wizModule = PyImport_ImportModule("Wizard");
    if(!wizModule) {
        PyErr_Print();
        printf("Error importing Wizard module!\n");
//        PyErr_Clear();
        return pageList;
    }

    wizardPageType = PyObject_GetAttrString(wizModule, "WizardPage");
    if(!wizardPageType) {
        PyErr_Print();
        printf("WizardPage class is missing!");
//        PyErr_Clear();
        return pageList;
    }

    wizModuleDict = PyModule_GetDict(wizModule);
    if(!wizModuleDict) {
        PyErr_Print();
        printf("Error getting Wizard module dict.\n");
        return pageList;
    }

    while(PyDict_Next(wizModuleDict, &pos, &key, &value)) {
        keyString = PyString_AsString(key);
        if(PyType_Check(value) &&
                PyType_IsSubtype((PyTypeObject*)value, (PyTypeObject*)wizardPageType) &&
                PyObject_GetAttrString(value, "__name__") != PyObject_GetAttrString(wizardPageType, "__name__") &&
                PyObject_IsTrue(PyObject_GetAttrString(value, "enabled"))) {
            pageList.append(value);
        }
    }

//    wizardPageList = PyObject_GetAttrString(wizModule, "page_order");
//    if(!wizardPageList ||
//            !PyList_Check(wizardPageList)) {
//        PyErr_Print();
//        printf("Error getting page_order.\n");
//        return pageList;
//    }
//    wizPageListSize = PyList_Size(wizardPageList);
//    for(int i = 0;i < wizPageListSize ; i++) {
//        wizPageListItem = PyList_GetItem(wizardPageList, i);
//        if(PyType_Check(wizPageListItem) &&
//                PyType_IsSubtype((PyTypeObject*) wizPageListItem, (PyTypeObject*) wizardPageType)) {
//            pageList.append(wizPageListItem);
//        }
//    }

    return pageList;
}

PyObject *PythonInterpreter::getWizardAccept() {
    return PyObject_GetAttrString( wizModule, "wizard_accept");
}

QList<QString> PythonInterpreter::settingAsStringList(QString settingName) {
    QList<QString> settingList;
    PyObject *settingsModule, *setting, *settingItem;
    Py_ssize_t settingSize;

    PyErr_Clear();
    settingsModule = PyImport_ImportModule("SystemSettings");
    if(!settingsModule) {
        PyErr_Print();
        printf("Error importing SystemSettings module!\n");
        return settingList;
    }

    setting = PyObject_GetAttrString(settingsModule, settingName.toStdString().data());
    if(!setting ||
            !PyList_Check(setting)) {
        PyErr_Print();
        printf("Setting, %s, not a Python list!\n", settingName.toStdString().data());
        return settingList;
    }

    settingSize = PyList_Size(setting);
    for(int i = 0;i < settingSize;i++) {
        settingItem = PyList_GetItem(setting, i);
        if(!settingItem || !PyString_Check(settingItem)) {
            PyErr_Print();
            printf("Setting, %s, contains objects other than strings!\n", settingName.toStdString().data());
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
        printf("Error importing DbLayout module!\n");
        return getTableNames();
    }

    dbOrder = PyObject_GetAttrString(dbLayoutModule, (char *) "db_order");
    if(!dbOrder || !PyList_Check(dbOrder)) {
//        Py_Finalize();
        PyErr_Print();
        printf("Method db_order doesn't exist or doesn't return a list!\n");
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
        printf("Error importing DbLayout module!\n");
        return metaTableName;
    }

    dbMetaMap = PyObject_GetAttrString(dbLayoutModule, (char *) "db_meta_map");
    if(!dbMetaMap || !PyList_Check(dbMetaMap)) {
//        Py_Finalize();
        PyErr_Print();
        printf("Method db_meta_map missing or doesn't return a list!\n");
        return metaTableName;
    }

    metaMapSize = PyList_Size(dbMetaMap);
    for(int i = 0;i < metaMapSize;i++) {
        mapTuple = PyList_GetItem(dbMetaMap, i);
        if(!mapTuple || !PyTuple_Check(mapTuple)) continue;
        QString mapTupleName = PyString_AsString(PyTuple_GetItem(mapTuple, 0));
        if(mapTupleName.toLower() == tableName.toLower()) {
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

int PythonInterpreter::getMetaReferenceCol(QString metaTableName) {
//    int reference_col;
    QList<QString> colDefsList = getColDefsList(metaTableName);
    for(int i = 0;i < colDefsList.size();i++) {
        QString col_def = colDefsList.at(i);
        if(col_def.contains("REFERENCES", Qt::CaseInsensitive)) {
//            reference_col = i;
//            break;
            return i;
        }
    }
    return -1;
}

QString PythonInterpreter::getReferenceIndexName(QString metaTableName) {
    QString indexName;
    QString ref = "REFERENCES";
    QList<QString> colDefsList = getColDefsList(metaTableName);
    for(int i = 0;i < colDefsList.size();i++) {
            QString col_def = colDefsList.at(i);
            if(col_def.contains(ref, Qt::CaseInsensitive)) {
//                qInfo("col_def: %s", col_def.toStdString().data());
                int ref_index = col_def.indexOf(ref);
//                qInfo("ref_index: %i", ref_index);
                int re_start_index = ref_index + ref.length();
                QRegularExpression re("(\\(.*\\))");
                QRegularExpressionMatch local_match = re.match(col_def, re_start_index);
                if(!local_match.hasMatch()){
                    qInfo("metaTable, %s, appears to be missing a reference column name", metaTableName.toStdString().data());
                } else {
                    QString match_string = local_match.captured(0);
                    match_string.remove(0, 1);
                    match_string.chop(1);
//                    qInfo("match_string: %s", match_string.toStdString().data());
                    indexName = match_string;
                }
                return indexName;
            }
        }
}

PyObject *PythonInterpreter::makeDictFromRow(QList<QVariant> *row, QString tableName, DatabaseHandler *db) {
    PyObject *row_dict = PyDict_New();
    for(int i = 0;i < row->length();i++) {
        QString col_name = db->getColName(tableName, i);
        QVariant value = row->at(i);
        if(strcmp(value.typeName(), "QString") == 0) {
            QString value_string = value.toString();
            PyDict_SetItemString(row_dict, col_name.toStdString().data(), Py_BuildValue("s", value_string.toStdString().data()));
        } else if(strcmp(value.typeName(), "qlonglong") == 0) {
            int value_int = value.toInt();
            PyDict_SetItemString(row_dict, col_name.toStdString().data(), Py_BuildValue("i", value_int));
        }
    }
    return row_dict;
}

PyObject *PythonInterpreter::makeDictListFromRows(QList<QList<QVariant> *> rows, QString tableName, DatabaseHandler *db) {
    PyObject *dictList = PyList_New(0);
    for(int i = 0;i < rows.length();i++) {
        qInfo("dict row: %i", i);
        PyObject *dict = makeDictFromRow(rows.at(i), tableName, db);
        PyList_Append(dictList, dict);
    }
    return dictList;
}

PyObject *PythonInterpreter::makeDictFromSqlRecord(QSqlRecord row) {
    PyObject *row_dict = PyDict_New();
    for(int i = 0;i < row.count();i++) {
        QString col_name = row.fieldName(i);
        QVariant value = row.value(i);
        if(strcmp(value.typeName(), "QString") == 0) {
            QString value_string = value.toString();
            PyDict_SetItemString(row_dict, col_name.toStdString().data(), Py_BuildValue("s", value_string.toStdString().data()));
        } else if(strcmp(value.typeName(), "qlonglong") == 0) {
            int value_int = value.toInt();
            PyDict_SetItemString(row_dict, col_name.toStdString().data(), Py_BuildValue("i", value_int));
        }
    }
    return row_dict;
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

QList<QVariant> *PythonInterpreter::getDataRow( PyObject *row_obj ) {
    QList<QVariant> *row = new QList<QVariant>();
    for ( int i = 0 ; i < PyList_Size( row_obj ) ; i++ ) {
        PyObject *datumPy = PyList_GetItem(row_obj, i);
        if(PyInt_Check(datumPy)) {
            QVariant datum = QVariant((int) PyInt_AsSsize_t(datumPy));
            row->append(datum);
        }
        else if (PyString_Check(datumPy)) {
            QVariant datum = QVariant(QObject::trUtf8(PyString_AsString(datumPy)));
            row->append(datum);
        }
        else if (PyBool_Check(datumPy)) {
            QVariant datum = QVariant(datumPy == Py_True);
            row->append(datum);
        }
    }
    return row;
}

QList<PyObject *> PythonInterpreter::getManageWindows() {
    QList<PyObject *> manage_list;
    PyObject *manage_module, *manage_type, *manage_module_dict, *key, *value, *instance;
    Py_ssize_t pos = 0;
//    QString key_string;

    manage_module = PyImport_ImportModule( "Manage" );
    if( !manage_module ) {
        PyErr_Print();
        qInfo( "Manage module import failed!" );
        return manage_list;
    }

    manage_type = PyObject_GetAttrString( manage_module, "Manage" );
    if ( !manage_type ) {
        PyErr_Print();
        qInfo( "Manage class is missing!" );
        return manage_list;
    }

    manage_module_dict = PyModule_GetDict( manage_module );
    if( !manage_module_dict ) {
        PyErr_Print();
        qInfo( "Error getting Manage module dict.");
        return manage_list;
    }

    while( PyDict_Next( manage_module_dict, &pos, &key, &value ) ) {
//        key_string = PyString_AsString( key );
        if( PyType_Check( value ) &&
                PyType_IsSubtype( (PyTypeObject*)value, (PyTypeObject*)manage_type ) &&
                PyObject_GetAttrString( value, "__name__" ) != PyObject_GetAttrString( manage_type, "__name__" ) &&
                PyObject_IsTrue( PyObject_GetAttrString( value, "enabled" ) ) ) {
            instance = PyObject_CallObject( value, NULL );
            PyErr_Print();
            manage_list.append( instance );
        }
    }

    return manage_list;
}

QList<QList<QVariant> *> PythonInterpreter::getDataList(PyObject *data) {
    QList<QList<QVariant> *> dataList = QList<QList<QVariant> *>();
    for(int i = 0 ; i < PyList_Size(data) ; i++) {
        PyObject *recordPy = PyList_GetItem(data, i);
        if(!PyList_Check(recordPy)) {
            printf("The data attibute contains items other than lists!\n");
            return dataList;
        }
//        QList<QVariant> *record = new QList<QVariant>();
//        for(int j = 0 ; j < PyList_Size(recordPy) ; j++) {
//            PyObject *datumPy = PyList_GetItem(recordPy, j);
//            if(PyInt_Check(datumPy)) {
//                QVariant datum = QVariant((int) PyInt_AsSsize_t(datumPy));
//                record->append(datum);
//            }
//            else if (PyString_Check(datumPy)) {
//                QVariant datum = QVariant(QObject::trUtf8(PyString_AsString(datumPy)));
//                record->append(datum);
//            }
//            else if (PyBool_Check(datumPy)) {
//                QVariant datum = QVariant(datumPy == Py_True);
//                record->append(datum);
//            }
//        }
        QList<QVariant> *record = getDataRow( recordPy );
        dataList.append(record);
    }
    return dataList;
}

void PythonInterpreter::finalizePython() {
    if (Py_IsInitialized()) Py_Finalize();
}

Dice dice;

PyObject *dice_rollString(PyObject *self, PyObject *args) {
    char *diceString;
//    Dice dice;
    if(!PyArg_ParseTuple(args, "s", &diceString)) {
        return NULL;
    }
    return Py_BuildValue("i", dice.rollDice(diceString));
}

PyObject *dice_randomInt(PyObject *self, PyObject *args) {
    int lower_int;
    int upper_int;
//    Dice dice;
    if(!PyArg_ParseTuple(args, "ii", &lower_int, &upper_int)) {
        return NULL;
    }
    return Py_BuildValue("i", dice.generateArbitraryRandomInt(lower_int, upper_int));
}

PyObject *dbQuery_getTable( PyObject *self, PyObject *args ) {
    char *table_name;
    if ( !PyArg_ParseTuple( args, "s", &table_name ) ) {
        return NULL;
    }

    DatabaseHandler *db = db_global;
    PythonInterpreter *pi = pi_global;

    PyObject *row_list_obj = PyList_New( 0 );
//    qInfo( "query_string: %s", query_string );
    QList<QSqlRecord> sql_rows = db->getColRowsAsSqlRecord( table_name );
    QString meta_table_name = pi->getMetaTableName( table_name );

    for( Py_ssize_t i = 0 ; i < sql_rows.length() ; i++ ) {
        QSqlRecord sql_row = sql_rows.at( i );
        PyObject *row_dict = pi->makeDictFromSqlRecord( sql_row );
        if( meta_table_name != NULL ) {
            int refCol = pi->getMetaReferenceCol( meta_table_name );
            QString refColName = db->getColName(meta_table_name, refCol);
            QString refIndexName = pi->getReferenceIndexName( meta_table_name );
            QString refName = PyString_AsString( PyDict_GetItemString( row_dict, refIndexName.toStdString().data() ) );
            QList<QSqlRecord> meta_rows = db->getColRowsAsSqlRecord( meta_table_name, "*", refColName, refName );
            PyObject *meta_list = PyList_New( 0 );
            for( Py_ssize_t j = 0 ; j < meta_rows.length() ; j++ ) {
                QSqlRecord meta_row = meta_rows.at( j );
                PyObject *meta_row_dict = pi->makeDictFromSqlRecord( meta_row );
                PyList_Append( meta_list, meta_row_dict );
            }
            PyDict_SetItemString( row_dict, meta_table_name.toStdString().data(), meta_list );
        }
        PyDict_SetItemString( row_dict, "TableName", PyString_FromString( table_name ) );
//        PyDict_SetItemString( row_dict, "DisplayCol", pi_global->getColList( pi_global->getDisplayCol( table_name ) ) );
        PyList_Append( row_list_obj, row_dict );
    }

    return row_list_obj;
}

PyObject *dbQuery_insertRow( PyObject *self, PyObject *args ) {
    char *table_name;
    PyObject *row;
    if ( !PyArg_ParseTuple( args, "sO", &table_name, &row ) ) {
        PyErr_Print();
        return NULL;
    }

    DatabaseHandler *db = db_global;
    PythonInterpreter *pi = pi_global;

    QList<QVariant> *data_row = pi->getDataRow( row );
    int row_id = db->insertRow( table_name, data_row );

    return Py_BuildValue( "i", row_id );
}

PyObject *dbQuery_updateRow( PyObject *self, PyObject *args ) {
    char *table_name;
    char *where_col;
    char *where;
    PyObject *row;

    if ( !PyArg_ParseTuple( args, "sssO", &table_name, &where_col, &where, &row ) ) {
        PyErr_Print();
        return NULL;
    }

    DatabaseHandler *db = db_global;
    PythonInterpreter *pi = pi_global;

    QList<QVariant> *data_row = pi->getDataRow( row );
    bool success = db->updateRow( table_name, pi->getColList( table_name ), where_col, where, data_row );
//    qInfo("Success: %s", success ? "true" : "false");
    return ( success ? Py_True : Py_False );
}

PyObject *dbQuery_deleteRow( PyObject *self, PyObject *args ) {
    char *table_name;
    char *where_col;
    char *where;

    if ( !PyArg_ParseTuple( args, "sss", &table_name, &where_col, &where ) ) {
        PyErr_Print();
        return NULL;
    }

    DatabaseHandler *db = db_global;
    PythonInterpreter *pi = pi_global;

    bool success = db->deleteRow( table_name, where_col, where );
    return ( success ? Py_True : Py_False );
}

PyObject *dbQuery_begin( PyObject *self, PyObject *args ) {
    DatabaseHandler *db = db_global;
    db->begin();
    return Py_None;
}

PyObject *dbQuery_commit( PyObject *self, PyObject *args ) {
    DatabaseHandler *db = db_global;

    bool commited = db->commit();
//    qInfo( "Commited: %s", commited ? "True" : "False" );
    return PyBool_FromLong( commited );
}
