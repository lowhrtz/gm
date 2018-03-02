#ifndef GUIWIZARD_H
#define GUIWIZARD_H

#include "PythonInterpreter.h"
#include "CustomWidgets.h"

#include <QWizard>


class GuiWizard : public QWizard {
public:
    GuiWizard( PyObject *wizard_page_list_obj, PyObject *external_data, QWidget *parent = 0 );
    WidgetRegistry getWidgetRegistry();

private:
    WidgetRegistry widgetRegistry;
};

#endif // GUIWIZARD_H
