#include "GuiWizard.h"
#include "GuiWizardPage.h"

GuiWizard::GuiWizard(PyObject *wizard_obj, PyObject *external_data, QWidget *parent )
    : QWizard( parent ) {
    PyObject *gui_defs_mod = PyImport_ImportModule( "GuiDefs" );
    PyObject *wizard_type = PyObject_GetAttrString( gui_defs_mod, (char *) "Wizard" );
    PyObject *wizard_page_list_obj = PyObject_CallMethod( wizard_obj, (char *) "get_wizard_pages", NULL );
    PyErr_Print();
    PyObject *title_obj = PyObject_CallMethod( wizard_obj, (char *) "get_title", NULL );
    PyErr_Print();

    if ( !wizard_type ) {
        PyErr_Print();
        qInfo( "Wizard class is missing!" );
        return;
    }
    bool is_wizard = PyObject_IsInstance( wizard_obj, wizard_type );
    if ( !is_wizard ) {
        qInfo( "An object that is not a WizardPage has been added to the wizard list!" );
        return;
    }
    if ( !PyList_Check( wizard_page_list_obj ) ) {
        qInfo( "Wizard does not return a list for get_wizard_pages!" );
        return;
    }
    if ( !PyString_Check( title_obj ) ) {
        qInfo( "The title of the Wizard is not a string!" );
    }

//    setWizardStyle( QWizard::ModernStyle );
    setWizardStyle( QWizard::ClassicStyle );

    setWindowTitle( PyString_AsString( title_obj ) );

    PyObject *wizard_page_dict_obj = PyDict_New();
    for ( int i = 0; i < PyList_Size( wizard_page_list_obj ); i++ ) {
        PyObject *wizard_page_obj = PyList_GetItem( wizard_page_list_obj, i );
        PyDict_SetItem( wizard_page_dict_obj, PyObject_CallMethod( wizard_page_obj, (char *) "get_title", NULL ), wizard_page_obj );
        GuiWizardPage *gui_wizard_page = new GuiWizardPage( wizard_page_obj, wizard_page_dict_obj, external_data, &widgetRegistry, this );
        setPage( gui_wizard_page->getPageId(), gui_wizard_page );
    }

    if( !PyList_Check( wizard_page_list_obj ) ) {
        qInfo( "The wizard must be a list of WizardPages" );
        return;
    }
}

WidgetRegistry GuiWizard::getWidgetRegistry() {
    return widgetRegistry;
}

