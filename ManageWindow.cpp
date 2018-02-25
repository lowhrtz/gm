//#include "CustomWidgets.h"
#include "Dialogs.h"
#include "ManageWindow.h"
#include "PDFCreator.h"
#include "PythonInterpreter.h"

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>

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
        PyObject *action_type_obj = PyObject_GetAttrString( action_obj, (char *) "action_type" );
        PyObject *widget1_obj = PyObject_GetAttrString( action_obj, (char *) "widget1" );
        PyObject *widget2_obj = PyObject_GetAttrString( action_obj, (char *) "widget2" );
        PyObject *callback_obj = PyObject_GetAttrString( action_obj, (char *) "callback" );

        PyObject *widget1_field_name_obj = PyObject_CallMethod( widget1_obj, (char *) "get_field_name", NULL );
        PyErr_Print();
        PyObject *widget1_type_obj = PyObject_CallMethod( widget1_obj, (char *) "get_widget_type", NULL );
        PyErr_Print();

        QString action_type = PyString_AsString( action_type_obj );
        QString widget1_type = PyString_AsString( widget1_type_obj );
        QString widget1_field_name = PyString_AsString( widget1_field_name_obj );
        if ( widget1_field_name.endsWith( "_" ) ) {
            widget1_field_name.chop( 1 );
        }

        QString widget2_type;
        QString widget2_field_name;
        if ( widget2_obj != Py_None ) {
            PyObject *widget2_field_name_obj = PyObject_CallMethod( widget2_obj, (char *) "get_field_name", NULL );
            PyErr_Print();
            PyObject *widget2_type_obj = PyObject_CallMethod( widget2_obj, (char *) "get_widget_type", NULL );
            PyErr_Print();
            widget2_type = PyString_AsString( widget2_type_obj );
            widget2_field_name = PyString_AsString( widget2_field_name_obj );
            if ( widget2_field_name.endsWith( "_" ) ) {
                widget2_field_name.chop( 1 );
            }
        }

        if ( action_type.toLower() == "onshow" ) {
            PyObject *callback_return_obj = Py_None;
            if ( callback_obj != Py_None ) {
                callback_return_obj = PyObject_CallObject( callback_obj, NULL );
                PyErr_Print();
            }
            connect( this, &ManageWindow::onShow, [=] () {
                if ( widget1_type.toLower() == "lineedit" ) {
                    QLineEdit *widget1 = (QLineEdit *) widget_registry.getGuiWidget( widget1_field_name )->getWidget();
                    widget1->setText( PyString_AsString( callback_return_obj ) );

                } else if ( widget1_type.toLower() == "listbox" ) {
                    QListWidget *widget1 = (QListWidget *) widget_registry.getGuiWidget(widget1_field_name)->getWidget();
                    PyDataListWidgetItem::fillListWidget( widget1, callback_return_obj );

                }
            });

        } else if ( widget1_type.toLower() == "pushbutton" ) {
            QPushButton *widget1 = (QPushButton *) widget_registry.getGuiWidget(widget1_field_name)->getWidget();
            connect( widget1, &QPushButton::clicked, [=] ( /*bool checked*/ ) {
                widget_registry.processAction( action_obj, this );
            });

        } else if ( widget1_type.toLower() == "listbox" ) {
            QListWidget *widget1 = (QListWidget *) widget_registry.getGuiWidget(widget1_field_name)->getWidget();
            connect( widget1, &QListWidget::currentItemChanged, [=]  ( /*QListWidgetItem *current, QListWidgetItem *previous*/ ) {
                widget_registry.processAction( action_obj, this );
            });

        }

    }

    PyObject *menu_list_obj = PyObject_CallMethod( manage_window_instance, (char *) "get_menu_list", NULL );
    PyErr_Print();

    for( Py_ssize_t i = 0 ; i < PyList_Size( menu_list_obj ) ; i++ ) {
        PyObject *menu_obj = PyList_GetItem( menu_list_obj, i );
        PyObject *menu_name_obj = PyObject_CallMethod( menu_obj, (char *) "get_menu_name", NULL );
        PyErr_Print();
        QString menu_name = PyString_AsString( menu_name_obj );
        QMenu *menu = menuBar()->addMenu( menu_name );
        PyObject *menu_action_list_obj = PyObject_CallMethod( menu_obj, (char *) "get_action_list", NULL );
        PyErr_Print();
        for( Py_ssize_t j = 0 ; j < PyList_Size( menu_action_list_obj ) ; j++ ) {
            PyObject *menu_action_obj = PyList_GetItem( menu_action_list_obj, j );
            PyObject *menu_widget1_obj = PyObject_GetAttrString( menu_action_obj, (char *) "widget1" );
            PyObject *menu_widget1_field_name_obj = PyObject_CallMethod( menu_widget1_obj, (char *) "get_field_name", NULL );
            PyErr_Print();
            QString menu_widget1_field_name = PyString_AsString( menu_widget1_field_name_obj );

            QAction *menu_action = new QAction( menu_widget1_field_name, this );
            menu->addAction( menu_action );
            connect( menu_action, &QAction::triggered, [=] ( /*bool checked*/ ) {
                widget_registry.processAction( menu_action_obj, this );
            });
        }
    }

    QString title( class_name );
    title.prepend( "Manage " );
    setWindowTitle( title.toUtf8() );
    setWindowModality( Qt::WindowModal );
}
