#include "ManageWindow.h"

#include <QGridLayout>
#include <QGroupBox>

ManageWindow::ManageWindow( PyObject *manage_window_instance, QWidget *parent )
    : QMainWindow( parent ) {
    PyObject *class_name_obj, *widget_matrix, *matrix_row, *widget_obj;
    Py_ssize_t number_of_rows, number_of_cols;
    QString class_name;
    QGroupBox *grid_groupbox = new QGroupBox;
    QGridLayout *grid_layout = new QGridLayout;

    class_name_obj = PyObject_GetAttrString( PyObject_Type( manage_window_instance ), "__name__" );
    class_name = PyString_AsString( class_name_obj );
    widget_matrix = PyObject_CallMethod( manage_window_instance, (char *) "get_widget_matrix", NULL );
    PyErr_Print();
    number_of_rows = PyList_Size( widget_matrix );
    //qInfo( "number_of_rows: %i", number_of_rows );
    for( Py_ssize_t i = 0 ; i < number_of_rows ; i++ ) {
        matrix_row = PyList_GetItem( widget_matrix, i );
        number_of_cols = PyList_Size( matrix_row );
        for( Py_ssize_t j = 0 ; j < number_of_cols ; j++ ) {
            widget_obj = PyList_GetItem( matrix_row, j );
            GuiWidget *gui_widget = new GuiWidget( widget_obj, this );
            widget_registry.registerWidget( gui_widget );
            if ( gui_widget->getWidgetLayout() != NULL ) {
                grid_layout->addLayout( gui_widget->getWidgetLayout(), i, j, gui_widget->getRowSpan(), gui_widget->getColSpan(), gui_widget->getAlignFlag() );
            }
        }
    }
    grid_groupbox->setLayout( grid_layout );
    setCentralWidget( grid_groupbox );

    PyObject *action_list_obj = PyObject_CallMethod( manage_window_instance, (char *) "get_action_list", NULL );
    PyErr_Print();
    for( Py_ssize_t i = 0 ; i < PyList_Size( action_list_obj ) ; i++ ) {
        PyObject *action_obj = PyList_GetItem( action_list_obj, i );
        GuiAction gui_action( action_obj );
        QString action_type = gui_action.getActionType();
        PyObject *callback_obj = gui_action.getCallback();

        if ( action_type.toLower() == "onshow" ) {
            if ( callback_obj != Py_None ) {
                PyObject *callback_return_obj = Py_None;
                callback_return_obj = PyObject_CallObject( callback_obj, Py_BuildValue( "(O)", widget_registry.getFieldsDict() ) );
                PyErr_Print();
                connect( this, &ManageWindow::onShow, [=] () {
                    if ( !PyDict_Check( callback_return_obj ) ) {
                        qInfo( "The OnShow action expects the callback function to return a dictionary of all the fields to be updated." );
                        return;
                    }
                    widget_registry.fillFields( callback_return_obj );
                });
            }

        } else {
            widget_registry.setDefaultActions( gui_action, this );
        }
    }

    PyObject *menu_list_obj = PyObject_CallMethod( manage_window_instance, (char *) "get_menu_list", NULL );
    PyErr_Print();
    widget_registry.fillMenuBar( menuBar(), menu_list_obj, this );

    QString title( class_name );
    title.prepend( "Manage " );
    setWindowTitle( title.toUtf8() );
    setWindowModality( Qt::WindowModal );
}
