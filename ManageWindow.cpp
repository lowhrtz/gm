#include "CustomWidgets.h"
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
    PyObject *class_name_obj, *widget_matrix, *matrix_row, *widget_obj, *field_name_obj, *widget_type_obj, *widget_edit_enabled_obj,
             *col_span_obj, *row_span_obj, *widget_align_obj, *widget_data_obj, *list_item_obj, *connect_list_obj, *connect_tuple,
             *trigger_widget_obj, *triggered_widget_obj, *callback_obj, *trigger_field_name_obj, *triggered_field_name_obj;
    Py_ssize_t number_of_rows, number_of_cols, col_span, row_span;
    QString class_name, field_name, widget_align, widget_data, widget_type, trigger_field_name, trigger_widget_type, triggered_field_name, triggered_widget_type;
    QWidget *widget, *trigger_widget, *triggered_widget;
    bool hide_field_name, is_edit_enabled;
    Qt::Alignment align_flag;

    QGroupBox *grid_groupbox = new QGroupBox;
    QGridLayout *grid_layout = new QGridLayout;
    QLayout *widget_layout;
//    qInfo( "Class Name: %s", PyString_AsString( PyObject_GetAttrString( PyObject_Type( manage_window_instance ), (char *) "__name__" ) ) );

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
            hide_field_name = false;
            widget_obj = PyList_GetItem( matrix_row, j );
            field_name_obj = PyObject_CallMethod( widget_obj, (char *) "get_field_name", NULL);
            PyErr_Print();
            widget_type_obj = PyObject_CallMethod( widget_obj, (char *) "get_widget_type", NULL );
            PyErr_Print();
            widget_edit_enabled_obj = PyObject_CallMethod( widget_obj, (char *) "is_edit_enabled", NULL );
            PyErr_Print();
            col_span_obj = PyObject_CallMethod( widget_obj, (char *) "get_col_span", NULL );
            PyErr_Print();
            row_span_obj = PyObject_CallMethod( widget_obj, (char *) "get_row_span", NULL );
            PyErr_Print();
            widget_align_obj = PyObject_CallMethod( widget_obj, (char *) "get_align", NULL );
            PyErr_Print();
            widget_data_obj = PyObject_CallMethod( widget_obj, (char *) "get_data", NULL );
            PyErr_Print();
            field_name = PyString_AsString( field_name_obj );
            if ( field_name.endsWith( "_" ) ) {
                hide_field_name = true;
                field_name.chop( 1 );
            }
            widget_type = PyString_AsString( widget_type_obj );
            is_edit_enabled =  widget_edit_enabled_obj == Py_True;
            col_span = PyInt_AsSsize_t( col_span_obj );
            row_span = PyInt_AsSsize_t( row_span_obj );
            align_flag = 0;
            if ( widget_align_obj != Py_None ) {
                widget_align = PyString_AsString( widget_align_obj );
                if ( widget_align.toLower() == "center" ) {
                    align_flag = Qt::AlignCenter;
                } else if ( widget_align.toLower() == "left" ) {
                    align_flag = Qt::AlignLeft;
                } else if ( widget_align.toLower() == "right" ) {
                    align_flag = Qt::AlignRight;
                }
            }
            //qInfo( "widget_type: %s", widget_type.toStdString().data() );

            if ( widget_type.toLower() == "checkbox" ) {
                widget_layout = new QHBoxLayout;
                if ( widget_data_obj != Py_None ) {
                    widget_data = PyString_AsString( widget_data_obj );
                } else if ( hide_field_name ) {
                    widget_data = "";
                } else {
                    widget_data = field_name;
                }
                widget = new QCheckBox( widget_data );
                widget->setEnabled( is_edit_enabled );
                widget_layout->addWidget( widget );

            } else if ( widget_type.toLower() == "textedit" ) {
                widget_layout = new QVBoxLayout;
                widget = new QTextEdit( this );
                widget->setEnabled( is_edit_enabled );
                if ( !hide_field_name ) {
                    widget_layout->addWidget( new QLabel( field_name, this ) );
                }
                widget_layout->addWidget( widget );

            } else if ( widget_type.toLower() == "lineedit" ) {
                widget_layout = new QHBoxLayout;
                if ( widget_data_obj != Py_None ) {
                    widget_data = PyString_AsString( widget_data_obj );
                } else {
                    widget_data = "";
                }
                widget = new QLineEdit( widget_data, this );
                widget->setEnabled( is_edit_enabled );
                if ( !hide_field_name ) {
                    widget_layout->addWidget( new QLabel( field_name, this ) );
                }
                widget_layout->addWidget( widget );

            } else if ( widget_type.toLower() == "spinbox" ) {
                widget_layout = new QHBoxLayout;
                widget = new QSpinBox( this );
                widget->setEnabled( is_edit_enabled );
                ( (QSpinBox *) widget )->setRange( -1000000000, 1000000000 );
                if ( !hide_field_name ) {
                    widget_layout->addWidget( new QLabel( field_name, this ) );
                }
                widget_layout->addWidget( widget );

            } else if ( widget_type.toLower() == "pushbutton" ) {
                widget_layout = new QHBoxLayout;
                widget = new QPushButton( field_name, this );
                widget->setEnabled( is_edit_enabled );
                widget_layout->addWidget( widget );

            } else if (  widget_type.toLower() == "combobox" ) {
                widget_layout = new QHBoxLayout;
                widget = new QComboBox( this );
                widget->setEnabled( is_edit_enabled );
                QStringList cb_contents;
                if ( PyList_Check( widget_data_obj ) ) {
                    for( int i = 0 ; i < PyList_Size( widget_data_obj ) ; i++ ) {
                        list_item_obj = PyList_GetItem( widget_data_obj, i );
                        if ( !PyString_Check( list_item_obj )  ) {
                            qInfo( "List item not a string! ComboBox widgets must contain a list of strings." );
                            continue;
                        }
                        cb_contents << PyString_AsString( list_item_obj );
                    }
                } else {
                    qInfo( "The data attribute of this ComboBox widget doesn't contain a list." );
                }
                ( (QComboBox *) widget )->addItems( cb_contents );
                if ( !hide_field_name ) {
                    widget_layout->addWidget(  new QLabel( field_name ) );
                }
                widget_layout->addWidget( widget );

            } else if ( widget_type.toLower() == "listbox" ) {
                widget_layout = new QVBoxLayout;
                widget = new QListWidget( this );
                widget->setEnabled( is_edit_enabled );
                if ( PyList_Check( widget_data_obj ) ) {
                    ManageWindow::fillListWidget( (QListWidget *) widget, widget_data_obj );
                }
                if ( !hide_field_name ) {
                    widget_layout->addWidget( new QLabel( field_name ) );
                }
                widget_layout->addWidget( widget );

            } else if ( widget_type.toLower() == "textlabel" ) {
                widget_layout = new QHBoxLayout;
                widget_data = PyString_AsString( widget_data_obj );
                widget = new QLabel( widget_data, this );
                widget->setEnabled( is_edit_enabled );
                widget_layout->addWidget( widget );

            } else if ( widget_type.toLower() == "image" ) {
                widget_layout = new QVBoxLayout;
                widget_data = "";
                if ( widget_data_obj != Py_None ) {
                    widget_data = PyString_AsString( widget_data_obj );
//                    ManageWindow::setImageWithBase64( (QLabel *) widget, widget_data);
                }
                widget = new ImageWidget( widget_data );
                widget->setEnabled( is_edit_enabled );
                widget_layout->addWidget( widget );

            } else if ( widget_type.toLower() == "hr" ) {
                widget_layout = new QHBoxLayout;
                QFrame *hr = new QFrame;
                hr->setFrameShape( QFrame::HLine );
                widget_layout->addWidget( hr );
            } else {
                continue;
            }

            registerWidget( field_name, widget_type, widget );
            grid_layout->addLayout( widget_layout, i, j, row_span, col_span, align_flag );
        }
    }
    grid_groupbox->setLayout( grid_layout );
    setCentralWidget( grid_groupbox );

    PyObject *action_list_obj = PyObject_CallMethod( manage_window_instance, (char *) "get_action_list", NULL  );
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
                    QLineEdit *widget1 = (QLineEdit *) widget_registry[widget1_field_name].first;
                    widget1->setText( PyString_AsString( callback_return_obj ) );

                } else if ( widget1_type.toLower() == "listbox" ) {
                    QListWidget *widget1 = (QListWidget *) widget_registry[widget1_field_name].first;
                    ManageWindow::fillListWidget( widget1, callback_return_obj );

                }
            });

        } else if ( widget1_type.toLower() == "pushbutton" ) {
            QPushButton *widget1 = (QPushButton *) widget_registry[widget1_field_name].first;
            connect( widget1, &QPushButton::clicked, [=] ( bool checked ) {
                processAction( action_obj );
            });

        } else if ( widget1_type.toLower() == "listbox" ) {
            QListWidget *widget1 = (QListWidget *) widget_registry[widget1_field_name].first;
            connect( widget1, &QListWidget::currentItemChanged, [=]  ( QListWidgetItem *current, QListWidgetItem *previous ) {
                processAction( action_obj );
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
            connect( menu_action, &QAction::triggered, [=] (bool checked ) {
                processAction( menu_action_obj );
            });
        }
    }

    QString title( class_name );
    title.prepend( "Manage " );
    setWindowTitle( title.toUtf8() );
    setWindowModality( Qt::WindowModal );
}

void ManageWindow::registerWidget(QString field_name, QString widget_type , QWidget *widget) {
    if ( widget_type.toLower() == "hr" ) return;
    std::pair<QWidget *, QString> widget_and_type( widget, widget_type );
    widget_registry[field_name] = widget_and_type;
}

void ManageWindow::processAction( PyObject *action_obj ) {
    PyObject *fields_dict_obj = getFieldsDict();
    PyObject *action_type_obj = PyObject_GetAttrString( action_obj, (char *) "action_type" );
    PyObject *widget1_obj = PyObject_GetAttrString( action_obj, (char *) "widget1" );
    PyObject *widget2_obj = PyObject_GetAttrString( action_obj, (char *) "widget2" );
    PyObject *callback_obj = PyObject_GetAttrString( action_obj, (char *) "callback" );
    PyObject *widget1_field_name_obj = PyObject_CallMethod( widget1_obj, (char *) "get_field_name", NULL );
    PyErr_Print();
    PyObject *widget1_widget_type_obj = PyObject_CallMethod( widget1_obj, (char *) "get_widget_type", NULL );
    PyErr_Print();

    QString action_type = PyString_AsString( action_type_obj );
    QString widget1_field_name = PyString_AsString( widget1_field_name_obj );
    if ( widget1_field_name.endsWith( "_" ) ) {
        widget1_field_name.chop( 1 );
    }
    QString widget1_widget_type = PyString_AsString( widget1_widget_type_obj );

    PyObject *widget2_field_name_obj, *widget2_widget_type_obj;
    QString widget2_field_name, widget2_widget_type;
    if ( widget2_obj != Py_None ) {
        widget2_field_name_obj = PyObject_CallMethod( widget2_obj, (char *) "get_field_name", NULL );
        PyErr_Print();
        widget2_widget_type_obj = PyObject_CallMethod( widget2_obj, (char *) "get_widget_type", NULL );
        PyErr_Print();
        widget2_field_name = PyString_AsString( widget2_field_name_obj );
        if ( widget2_field_name.endsWith( "_" ) ) {
            widget2_field_name.chop( 1 );
        }
        widget2_widget_type = PyString_AsString( widget2_widget_type_obj );
    }

    if ( action_type.toLower() == "fillfields" ) {
        if ( callback_obj != Py_None ) {
            PyObject *callback_return_obj = PyObject_CallObject( callback_obj, Py_BuildValue( "(O)", fields_dict_obj ) );
            PyErr_Print();
            fillFields( callback_return_obj );
//            Py_ssize_t pos = 0;
//            PyObject *key, *value;
//            while( PyDict_Next( callback_return_obj, &pos, &key, &value ) ) {
//                QString field_name = PyString_AsString( key );
//                QString widget_type = widget_registry[field_name].second;

//                if ( widget_type.toLower() == "lineedit" ) {
//                    QLineEdit *line_edit_widget = (QLineEdit *) widget_registry[field_name].first;
//                    line_edit_widget->setText( PyString_AsString( value ) );

//                } else if ( widget_type.toLower() == "textedit" ) {
//                    QTextEdit *text_edit_widget = (QTextEdit *) widget_registry[field_name].first;
//                    text_edit_widget->setText( PyString_AsString( value ) );

//                } else if ( widget_type.toLower() == "listbox" ) {
//                    QListWidget *list_widget = (QListWidget *) widget_registry[field_name].first;
//                    ManageWindow::fillListWidget( list_widget, value );

//                } else if ( widget_type.toLower() == "spinbox" ) {
//                    QSpinBox *spin_box_widget = (QSpinBox *) widget_registry[field_name].first;
//                    spin_box_widget->setValue( PyInt_AsSsize_t( value ) );

//                } else if ( widget_type.toLower() == "combobox" ) {
//                    QComboBox *combo_box_widget = (QComboBox *) widget_registry[field_name].first;
//                    int new_index = combo_box_widget->findText( PyString_AsString( value ) );
//                    combo_box_widget->setCurrentIndex( new_index );

//                } else if ( widget_type.toLower() == "image" ) {
//                    ImageWidget *image_widget = (ImageWidget *) widget_registry[field_name].first;
//                    image_widget->setData( PyString_AsString( value ) );

//                }
//            }
        }

    } else if ( action_type.toLower() == "savepdf" || action_type.toLower() == "printpreview" ) {
        if ( callback_obj != Py_None ) {
            PyObject *callback_return_obj = PyObject_CallObject( callback_obj, Py_BuildValue( "(O)", fields_dict_obj ) );
            PyErr_Print();
            PyObject *default_filename_obj = PyTuple_GetItem( callback_return_obj, 0 );
            PyObject *pdf_markup_obj = PyTuple_GetItem( callback_return_obj, 1 );

            QString default_filename = PyString_AsString( default_filename_obj );
            QString pdf_markup = PyString_AsString( pdf_markup_obj );
            PDFCreator pdf_creator( pdf_markup, default_filename );

            if ( action_type.toLower() == "savepdf" ) {
                pdf_creator.save();

            } else {
                pdf_creator.preview();
            }
        }

    } else if ( action_type.toLower().startsWith( "entrydialog" ) ) {
        QVariant *value = new QVariant;
        EntryDialog *dialog;
        bool accepted;

        QString title = widget1_field_name;
        if ( title.startsWith( "&" ) ) {
            title.remove( 0, 1 );
        }

        if ( widget2_widget_type.toLower() == "lineedit" ) {
            dialog = new EntryDialog( title, EntryDialog::LINE_EDIT, value, this );

        } else if ( widget2_widget_type.toLower() == "textedit" ) {
            dialog = new EntryDialog( title, EntryDialog::TEXT_EDIT, value, this );

        } else if ( widget2_widget_type.toLower() == "spinbox" ) {
            dialog = new EntryDialog( title, EntryDialog::SPIN_BOX, value, this );

        } else if ( widget2_widget_type.toLower() == "image" ) {
            ImageWidget *image_widget = (ImageWidget *) widget_registry[widget2_field_name].first;
            QString image_data = image_widget->getData();
            dialog = new EntryDialog( title, EntryDialog::IMAGE, value, this, image_data );
        }
        accepted = dialog->exec();
//        qInfo( "value: %s", value->toString().toStdString().data() );
        if ( accepted ) {
            if ( callback_obj != Py_None ) {
                PyObject *value_obj = PyString_FromString( value->toString().toStdString().data() );
                PyObject *callback_return_obj = PyObject_CallObject( callback_obj, Py_BuildValue( "(O, O)", value_obj, fields_dict_obj ) );
                PyErr_Print();
                fillFields( callback_return_obj );
            }
        }
    }
}

void ManageWindow::fillFields(PyObject *fill_dict_obj) {
    Py_ssize_t pos = 0;
    PyObject *key, *value;
    while( PyDict_Next( fill_dict_obj, &pos, &key, &value ) ) {
        QString field_name = PyString_AsString( key );
        QString widget_type = widget_registry[field_name].second;

        if ( widget_type.toLower() == "lineedit" ) {
            QLineEdit *line_edit_widget = (QLineEdit *) widget_registry[field_name].first;
            line_edit_widget->setText( PyString_AsString( value ) );

        } else if ( widget_type.toLower() == "textedit" ) {
            QTextEdit *text_edit_widget = (QTextEdit *) widget_registry[field_name].first;
            text_edit_widget->setText( PyString_AsString( value ) );

        } else if ( widget_type.toLower() == "listbox" ) {
            QListWidget *list_widget = (QListWidget *) widget_registry[field_name].first;
            ManageWindow::fillListWidget( list_widget, value );

        } else if ( widget_type.toLower() == "spinbox" ) {
            QSpinBox *spin_box_widget = (QSpinBox *) widget_registry[field_name].first;
            spin_box_widget->setValue( PyInt_AsSsize_t( value ) );

        } else if ( widget_type.toLower() == "combobox" ) {
            QComboBox *combo_box_widget = (QComboBox *) widget_registry[field_name].first;
            int new_index = combo_box_widget->findText( PyString_AsString( value ) );
            combo_box_widget->setCurrentIndex( new_index );

        } else if ( widget_type.toLower() == "image" ) {
            ImageWidget *image_widget = (ImageWidget *) widget_registry[field_name].first;
            image_widget->setData( PyString_AsString( value ) );

        }
    }
}

PyObject *ManageWindow::getFieldsDict() {
    PyObject *fields_dict_obj = PyDict_New();
    QHash<QString, std::pair<QWidget *, QString> >::iterator iter;
    for( iter = widget_registry.begin() ; iter != widget_registry.end() ; iter ++ ) {
        QString field_name = iter.key();
        QWidget *widget = iter.value().first;
        QString widget_type = iter.value().second;

        if ( widget_type.toLower() == "checkbox" ) {
            bool checked = ( (QCheckBox *) widget )->isChecked();
            PyDict_SetItemString( fields_dict_obj, field_name.toStdString().data(), checked ? Py_True : Py_False );

        } else if ( widget_type.toLower() == "textedit" ) {
            QString text = ( (QTextEdit *) widget )->toPlainText();
            PyDict_SetItemString( fields_dict_obj, field_name.toStdString().data(), Py_BuildValue( "s", text.toStdString().data() ) );

        } else if ( widget_type.toLower() == "lineedit" ) {
            QString text = ( (QLineEdit *) widget )->text();
            PyDict_SetItemString( fields_dict_obj, field_name.toStdString().data(), Py_BuildValue( "s", text.toStdString().data() ) );

        } else if ( widget_type.toLower() == "spinbox" ) {
            Py_ssize_t value = ( (QSpinBox *) widget )->value();
            PyDict_SetItemString( fields_dict_obj, field_name.toStdString().data(), Py_BuildValue( "i", value ) );

        } else if ( widget_type.toLower() == "combobox" ) {
            QString current_text = ( (QComboBox *) widget )->currentText();
            PyDict_SetItemString( fields_dict_obj, field_name.toStdString().data(), Py_BuildValue( "s", current_text.toStdString().data() ) );

        } else if ( widget_type.toLower() == "listbox" ) {
            QString current_field_name( field_name );
            current_field_name.append( " Current" );
            PyObject *item_data = Py_None;
            if ( ( (QListWidget *) widget )->currentRow() >= 0 ) {
                     ManageListWidgetItem *item = (ManageListWidgetItem *) ( (QListWidget *) widget )->currentItem();
                     item_data = item->getData();
            }
            PyDict_SetItemString( fields_dict_obj, current_field_name.toStdString().data(), item_data );

            PyObject *item_list_obj = PyList_New( ( (QListWidget *) widget )->count() );
            for ( Py_ssize_t i = 0 ; i < ( (QListWidget *) widget )->count() ; i++ ) {
                ManageListWidgetItem *item = (ManageListWidgetItem *) ( (QListWidget *) widget )->item( i );
                PyObject *data = item->getData();
                PyList_SetItem( item_list_obj, i, data );
            }
            PyDict_SetItemString( fields_dict_obj, field_name.toStdString().data(), item_list_obj );

        } else if ( widget_type.toLower() == "image" ) {
            QString image_data = ( (ImageWidget *) widget )->getData();
            PyDict_SetItemString( fields_dict_obj, field_name.toStdString().data(), Py_BuildValue( "s", image_data.toStdString().data() ) );

        }
    }
    return fields_dict_obj;
}

void ManageWindow::fillListWidget( QListWidget *list_widget, PyObject *list_obj ) {
    list_widget->clear();
    for ( Py_ssize_t i = 0 ; i < PyList_Size( list_obj ) ; i++ ) {
        PyObject *list_item_obj = PyList_GetItem( list_obj, i );

        if ( PyString_Check( list_item_obj ) ) {
            list_widget->addItem( new ManageListWidgetItem( PyString_AsString( list_item_obj ), list_item_obj, list_widget ) );

        } else if ( PyDict_Check( list_item_obj ) ) {
            PyObject *table_name_obj = PyDict_GetItemString( list_item_obj, "TableName" );
            QString table_name = PyString_AsString( table_name_obj );
            QString display = pi_global->getColList( table_name )[ pi_global->getDisplayCol( table_name ) ];
            PyObject *display_name_obj = PyDict_GetItemString( list_item_obj, display.toStdString().data() );
            QString display_name = PyString_AsString( display_name_obj );
            list_widget->addItem( new ManageListWidgetItem( display_name, list_item_obj, list_widget ));

        } else if ( PyTuple_Check( list_item_obj ) ) {
            PyObject *display_obj = PyTuple_GetItem( list_item_obj, 0 );
            PyObject *dict_obj = PyTuple_GetItem( list_item_obj, 1 );
            QString display_name = PyString_AsString( display_obj );
            list_widget->addItem( new ManageListWidgetItem( display_name, dict_obj, list_widget ) );
        }
    }
//    if ( list_widget->count() > 0 ) {
//        list_widget->setCurrentRow( 0 );
//    }
}

ManageListWidgetItem::ManageListWidgetItem(QString display_text, PyObject *data, QListWidget *parent)
    : QListWidgetItem( display_text, parent, QListWidgetItem::UserType ) {
    this->data = data;
}

PyObject *ManageListWidgetItem::getData() {
    return data;
}
