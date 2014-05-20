#ifndef LISTOBJECT_H
#define LISTOBJECT_H

#include "PythonInterpreter.h"
#include <QList>

class ListObject
{
public:
    ListObject();
    ListObject(PyObject *pyListOb);
    QList<PyObject> getList();

private:
    QList<PyObject> list;
};

#endif // LISTOBJECT_H
