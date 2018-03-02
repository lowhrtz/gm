#ifndef GUIWIZARDPAGE_H
#define GUIWIZARDPAGE_H

#include "PythonInterpreter.h"
#include "CustomWidgets.h"

#include <QWizardPage>

class GuiWizardPage : public QWizardPage {
public:
    GuiWizardPage( PyObject *wizard_page_obj, PyObject *wizard_page_dict_obj, PyObject *external_data, WidgetRegistry *widget_registry, QWidget *parent = 0 );
    int getPageId();
    void initializePage();
    bool isComplete() const;
    int nextId() const;

private:
    void connectGuiWidgetToCompleteChanged( GuiWidget *gui_widget );

private:
    PyObject *wizardPageObj;
    PyObject *wizardPageDictObj;
    PyObject *externalData;
    int pageId;
    WidgetRegistry *widgetRegistry;
};

#endif // GUIWIZARDPAGE_H
