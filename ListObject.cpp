#include "ListObject.h"

ListObject::ListObject()
{
}

ListObject::ListObject(PyObject *pyListOb)
{
    list = QList<PyObject>();
    int listSize = PyList_Size(pyListOb);
    for(int i = 0 ; i < listSize ; i++)
    {
        PyObject *item = PyList_GetItem(pyListOb, i);
        list.append(*item);
    }
}

QList<PyObject> ListObject::getList()
{
    return list;
}
