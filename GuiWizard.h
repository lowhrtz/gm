#ifndef GUIWIZARD_H
#define GUIWIZARD_H

#include "PythonInterpreter.h"
#include "CustomWidgets.h"

#include <QWizard>


class GuiWizard : public QWizard {

    Q_OBJECT

public:
    GuiWizard( PyObject *wizard_page_list_obj, PyObject *external_data, QWidget *parent = 0 );
    WidgetRegistry getWidgetRegistry();
    PyObject *getAcceptReturn();
    void accept();

private:
    WidgetRegistry widgetRegistry;
    PyObject *wizardPageDictObj;
    PyObject *externalData;
    PyObject *acceptMethod;
    PyObject *acceptReturn = Py_None;
};

#endif // GUIWIZARD_H
