#include "CustomWidgets.h"
#include "Dialogs.h"
#include "PDFCreator.h"

#include <QPushButton>
#include <QLayout>
#include <QCheckBox>
#include <QTextEdit>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>

ImageWidget::ImageWidget( QString base64_data, QWidget *parent )
    : QLabel( parent ) {
    setData( base64_data );
}

QString ImageWidget::getData() {
    return data;
}

void ImageWidget::setData( QString base64_data ) {
    ImageWidget::setImageWithBase64( this, base64_data );
    this->data = base64_data;
}

void ImageWidget::setImageWithBase64( QLabel *image_widget, QString base64_string ) {
    QByteArray ba = QByteArray::fromBase64( base64_string.toStdString().data() );
    QImage image = QImage::fromData( ba );
    QPixmap pixmap = QPixmap( QPixmap::fromImage( image ) );
    if( pixmap.height() > 200 ) {
        pixmap = pixmap.scaledToHeight( 200 );
    }
    image_widget->setPixmap( pixmap );
}

PyDataListWidgetItem::PyDataListWidgetItem(QString display_text, PyObject *data, QListWidget *parent , QListWidget *original_list)
    : QListWidgetItem( display_text, parent, QListWidgetItem::UserType ) {
    this->data = data;
    this->originalList = original_list;
}

PyObject *PyDataListWidgetItem::getData() {
    return data;
}

QListWidget *PyDataListWidgetItem::getOriginalList() {
    return originalList;
}

void PyDataListWidgetItem::fillListWidget(QListWidget *list_widget, PyObject *list_obj, PyObject *tool_tip_callback, PyObject *fields_obj ) {
    list_widget->clear();
    for ( Py_ssize_t i = 0 ; i < PyList_Size( list_obj ) ; i++ ) {
        PyObject *list_item_obj = PyList_GetItem( list_obj, i );
        PyDataListWidgetItem::addItemToWidget( list_widget, list_item_obj, tool_tip_callback, fields_obj );
    }
}

void PyDataListWidgetItem::addItemToWidget( QListWidget *list_widget, PyObject *list_item_obj, PyObject *tool_tip_callback, PyObject *fields_obj ) {
    PyDataListWidgetItem *list_item;
    PyObject *list_item_dict_obj;
    if ( PyString_Check( list_item_obj ) ) {
        list_item = new PyDataListWidgetItem( PyString_AsString( list_item_obj ), list_item_obj, list_widget );

    } else if ( PyDict_Check( list_item_obj ) ) {
        list_item_dict_obj = list_item_obj;
        PyObject *table_name_obj = PyDict_GetItemString( list_item_obj, "TableName" );
        QString table_name = PyString_AsString( table_name_obj );
        QString display = pi_global->getColList( table_name )[ pi_global->getDisplayCol( table_name ) ];
        PyObject *display_name_obj = PyDict_GetItemString( list_item_obj, display.toStdString().data() );
        QString display_name = PyString_AsString( display_name_obj );
        list_item = new PyDataListWidgetItem( display_name, list_item_obj, list_widget );

    } else if ( PyTuple_Check( list_item_obj ) ) {
        PyObject *display_obj = PyTuple_GetItem( list_item_obj, 0 );
        list_item_dict_obj = PyTuple_GetItem( list_item_obj, 1 );
        QString display_name = PyString_AsString( display_obj );
        list_item = new PyDataListWidgetItem( display_name, list_item_dict_obj, list_widget );
    } else {
        return;
    }

    if ( tool_tip_callback != NULL && tool_tip_callback != Py_None && fields_obj != NULL && list_item_dict_obj != NULL ) {
        PyObject *tool_tip_return = PyObject_CallObject( tool_tip_callback, Py_BuildValue( "(O,O)", list_item_dict_obj, fields_obj ) );
        PyErr_Print();
        QString tool_tip = PyString_AsString( tool_tip_return );
        list_item->setToolTip( tool_tip );
    }

    list_widget->addItem( list_item );
}

PyObject *PyDataListWidgetItem::getDataList( QListWidget *listbox ) {
    PyObject *list_obj = PyList_New( 0 );
    for( int i = 0 ; i < listbox->count() ; i++) {
        PyDataListWidgetItem *item = (PyDataListWidgetItem *) listbox->item( i );
        PyList_Append( list_obj, item->getData() );
    }

    return list_obj;
}

DualListWidget::DualListWidget(PyObject *owned_item_list_obj, PyObject *action_data_obj, PyObject *fields_obj, QWidget *parent)
    : QWidget( parent ){
    QHash<QString, QListWidget *> category_hash;
    tabbedAvailLists = new QTabWidget( this );
    tabbedAvailLists->tabBar()->setStyleSheet( "font: bold 8pt;" );

    chosenList = new QListWidget( this );

    QTabWidget *tabbed_chosen_list = new QTabWidget( this );
    tabbed_chosen_list->addTab( chosenList, "" );
    tabbed_chosen_list->tabBar()->hide();

    if ( !PyDict_Check( action_data_obj ) ) {
        qInfo( "The data parameter for a ListDialog Action must be a dictionary." );
        return;
    }

    PyObject *fill_avail_callback = PyDict_GetItemString( action_data_obj, (char *) "fill_avail" );
    PyErr_Print();
    PyObject *slots_callback = PyDict_GetItemString( action_data_obj, (char *) "slots" );
    PyErr_Print();
    PyObject *slots_name_obj = PyDict_GetItemString( action_data_obj, (char *) "slots_name");
    PyErr_Print();
    PyObject *category_field_obj = PyDict_GetItemString( action_data_obj, (char *) "category_field");
    PyErr_Print();
    PyObject *tool_tip_callback = PyDict_GetItemString( action_data_obj, (char *) "tool_tip" );
    PyErr_Print();
    PyObject *add_callback = PyDict_GetItemString( action_data_obj, (char *) "add" );
    PyErr_Print();
    PyObject *remove_callback = PyDict_GetItemString( action_data_obj, (char *) "remove" );
    PyErr_Print();

    QString slots_name = PyString_AsString( slots_name_obj );
    PyObject *avail_item_list_obj = PyObject_CallObject( fill_avail_callback, Py_BuildValue( "(O,O)", owned_item_list_obj, fields_obj ) );
    PyErr_Print();


    if ( category_field_obj != NULL && PyString_Check( category_field_obj ) ) {
        for ( Py_ssize_t i = 0; i < PyList_Size( avail_item_list_obj ); i++ ) {
            PyObject *item_obj = PyList_GetItem( avail_item_list_obj, i );
            PyObject *item_dict_obj;

            if ( PyDict_Check( item_obj ) ) {
                item_dict_obj = item_obj;
            } else if ( PyTuple_Check( item_obj ) && PyTuple_Size( item_obj ) > 1 ) {
                item_dict_obj = PyTuple_GetItem( item_obj, 1 );
            } else {
                continue;
            }

            PyObject *category_obj = PyDict_GetItem( item_dict_obj, category_field_obj );
            QString category = PyString_AsString( category_obj );
            if ( !category_hash.contains( category ) ) {
//                qInfo( "%s", category.toStdString().data() );
                QListWidget *cat_list = new QListWidget( this );
                category_hash[category] = cat_list;

                tabbedAvailLists->addTab( cat_list, category );
                PyDataListWidgetItem::addItemToWidget( cat_list, item_obj, tool_tip_callback, fields_obj );

            } else {
                QListWidget *cat_list = category_hash[category];
                PyDataListWidgetItem::addItemToWidget( cat_list, item_obj, tool_tip_callback, fields_obj );
            }
        }
    } else {
        QListWidget *avail_list = new QListWidget( this );
//        category_hash["Avail"] = avail_list;
        tabbedAvailLists->addTab( avail_list, "Avail" );
        tabbedAvailLists->tabBar()->hide();
        PyDataListWidgetItem::fillListWidget( avail_list, avail_item_list_obj, tool_tip_callback, fields_obj );
        if( avail_list->count() > 0 ) {
            avail_list->setCurrentRow( 0 );
        }

    }


    PyDataListWidgetItem::fillListWidget( chosenList, owned_item_list_obj, tool_tip_callback, fields_obj );
    if( chosenList->count() > 0 ) {
        chosenList->setCurrentRow( 0 );
    }

    PyObject *slots_callback_return_obj = PyObject_CallObject( slots_callback, Py_BuildValue( "(O)", fields_obj ) );
    PyErr_Print();
    
    QString slots_callback_return = PyString_AsString( slots_callback_return_obj );

    QPushButton *add_button = new QPushButton( "Add", this );
    QPushButton *del_button = new QPushButton( "Remove", this );

    QVBoxLayout *layout = new QVBoxLayout;

    QLabel *slots_label = new QLabel( "<b>" + slots_name + ":</b> " + slots_callback_return, this );
    layout->addWidget( slots_label, 1, Qt::AlignCenter );

    QHBoxLayout *list_layout = new QHBoxLayout;
    QVBoxLayout *button_layout = new QVBoxLayout;
    button_layout->addWidget( add_button );
    button_layout->addWidget( del_button );

    list_layout->addWidget( tabbedAvailLists );
    list_layout->addLayout( button_layout );
    list_layout->addWidget( tabbed_chosen_list );
    layout->addLayout( list_layout );

//    layout->setSizeConstraint( QLayout::SetFixedSize );
    setLayout( layout );

    connect( add_button, &QPushButton::clicked, [=] () {
        QListWidget *current_list = (QListWidget *) tabbedAvailLists->currentWidget();
//        qInfo( "Current Index: %i", current_list->currentRow() );
        if ( current_list->currentRow() == -1 ) return;
        PyDataListWidgetItem *current_item = (PyDataListWidgetItem *) current_list->currentItem();
        PyObject *add_callback_return = PyObject_CallObject( add_callback, Py_BuildValue( "(O,O)", current_item->getData(), fields_obj ) );
        PyErr_Print();
        if( !PyDict_Check( add_callback_return ) ) {
            qInfo( "The add callback should return a dictionary with the following keys: valid, slots_new_value, remove, new_display" );
            return;
        }

        PyObject *valid_obj = PyDict_GetItemString( add_callback_return, (char *) "valid" );
        PyErr_Print();
        PyObject *slots_new_value_obj = PyDict_GetItemString( add_callback_return, (char *) "slots_new_value" );
        PyErr_Print();
        PyObject *remove_obj = PyDict_GetItemString( add_callback_return, (char *) "remove" );
        PyErr_Print();
        PyObject *new_display_obj = PyDict_GetItemString( add_callback_return, (char *) "new_display" );
        PyErr_Print();

        if( valid_obj != Py_True ) {
            return;
        }

        if( slots_new_value_obj != NULL && PyString_Check( slots_new_value_obj ) ) {
            slots_label->setText( "<b>" + slots_name + ":</b> " + PyString_AsString( slots_new_value_obj ) );
        }

        if( remove_obj == Py_True ) {
            current_list->takeItem( current_list->currentRow() );
        }

        QString new_display_string;
        if( new_display_obj == NULL ) {
            new_display_string = current_item->text();
        } else if( PyString_Check( new_display_obj ) ) {
            new_display_string = PyString_AsString( new_display_obj );
        } else {
            new_display_string = current_item->text();
        }

        PyDataListWidgetItem::addItemToWidget( chosenList, current_item->getData(), tool_tip_callback, fields_obj );

    });

    connect( del_button, &QPushButton::clicked, [=] () {
        if ( chosenList->currentRow() == -1 ) return;
        PyDataListWidgetItem *current_item = (PyDataListWidgetItem *) chosenList->currentItem();
        PyObject *remove_callback_return = PyObject_CallObject( remove_callback, Py_BuildValue( "(O,O)", current_item->getData(), fields_obj ) );
        PyErr_Print();
        if( !PyDict_Check( remove_callback_return ) ) {
            qInfo( "The remove callback should return a dictionary with the following keys: valid, slots_new_value, replace, new_display" );
            return;
        }

        PyObject *valid_obj = PyDict_GetItemString( remove_callback_return, (char *) "valid" );
        PyErr_Print();
        PyObject *slots_new_value_obj = PyDict_GetItemString( remove_callback_return, (char *) "slots_new_value" );
        PyErr_Print();
        PyObject *replace_obj = PyDict_GetItemString( remove_callback_return, (char *) "replace" );
        PyErr_Print();
        PyObject *new_display_obj = PyDict_GetItemString( remove_callback_return, (char *) "new_display" );
        PyErr_Print();

        if( valid_obj != Py_True ) {
            return;
        }

        if( slots_new_value_obj != NULL && PyString_Check( slots_new_value_obj ) ) {
            slots_label->setText( "<b>" + slots_name + ":</b> " + PyString_AsString( slots_new_value_obj ) );
        }

        QString new_display_string;
        if( new_display_obj == NULL ) {
            new_display_string = current_item->text();
        } else if( PyString_Check( new_display_obj ) ) {
            new_display_string = PyString_AsString( new_display_obj );
        } else {
            new_display_string = current_item->text();
        }

        if( replace_obj == Py_True ) {
            QListWidget *original_list = current_item->getOriginalList();
            PyDataListWidgetItem::addItemToWidget( original_list, current_item->getData(), tool_tip_callback, fields_obj );
        }

        chosenList->takeItem( chosenList->currentRow() );

    });
}

QListWidget *DualListWidget::getChosenList() {
    return chosenList;
}

GuiWidget::GuiWidget( PyObject *widget_obj, QWidget *parent, bool create_qwidget ) {
    bool hide_field_name = false;
    PyObject *field_name_obj = PyObject_CallMethod( widget_obj, (char *) "get_field_name", NULL);
    PyErr_Print();
    PyObject *widget_type_obj = PyObject_CallMethod( widget_obj, (char *) "get_widget_type", NULL );
    PyErr_Print();
    PyObject *widget_edit_enabled_obj = PyObject_CallMethod( widget_obj, (char *) "is_edit_enabled", NULL );
    PyErr_Print();
    PyObject *col_span_obj = PyObject_CallMethod( widget_obj, (char *) "get_col_span", NULL );
    PyErr_Print();
    PyObject *row_span_obj = PyObject_CallMethod( widget_obj, (char *) "get_row_span", NULL );
    PyErr_Print();
    PyObject *widget_align_obj = PyObject_CallMethod( widget_obj, (char *) "get_align", NULL );
    PyErr_Print();
    PyObject *widget_data_obj = PyObject_CallMethod( widget_obj, (char *) "get_data", NULL );
    PyErr_Print();
    fieldName = PyString_AsString( field_name_obj );
    if ( fieldName.endsWith( "_" ) ) {
        hide_field_name = true;
        fieldName.chop( 1 );
    }
    widgetType = PyString_AsString( widget_type_obj );
    bool is_edit_enabled = widget_edit_enabled_obj == Py_True;
    colSpan = PyInt_AsSsize_t( col_span_obj );
    rowSpan = PyInt_AsSsize_t( row_span_obj );
    if ( widget_align_obj != Py_None ) {
        QString widget_align = PyString_AsString( widget_align_obj );
        if ( widget_align.toLower() == "center" ) {
            alignFlag = Qt::AlignCenter;
        } else if ( widget_align.toLower() == "left" ) {
            alignFlag = Qt::AlignLeft;
        } else if ( widget_align.toLower() == "right" ) {
            alignFlag = Qt::AlignRight;
        }
    }
    //qInfo( "widget_type: %s", widget_type.toStdString().data() );

    if ( create_qwidget ) {
        QString widget_data;
        if ( widgetType.toLower() == "checkbox" ) {
            widgetLayout = new QHBoxLayout;
            if ( widget_data_obj != Py_None ) {
                widget_data = PyString_AsString( widget_data_obj );
            } else if ( hide_field_name ) {
                widget_data = "";
            } else {
                widget_data = fieldName;
            }
            widget = new QCheckBox( widget_data );
            widget->setEnabled( is_edit_enabled );
            widgetLayout->addWidget( widget );

        } else if ( widgetType.toLower() == "textedit" ) {
            widgetLayout = new QVBoxLayout;
            widget = new QTextEdit( parent );
            widget->setEnabled( is_edit_enabled );
            if ( !hide_field_name ) {
                widgetLayout->addWidget( new QLabel( fieldName, parent ) );
            }
            widgetLayout->addWidget( widget );

        } else if ( widgetType.toLower() == "lineedit" ) {
            widgetLayout = new QHBoxLayout;
            if ( widget_data_obj != Py_None ) {
                widget_data = PyString_AsString( widget_data_obj );
            } else {
                widget_data = "";
            }
            widget = new QLineEdit( widget_data, parent );
            widget->setEnabled( is_edit_enabled );
            if ( !hide_field_name ) {
                widgetLayout->addWidget( new QLabel( fieldName, parent ) );
            }
            widgetLayout->addWidget( widget );

        } else if ( widgetType.toLower() == "spinbox" ) {
            widgetLayout = new QHBoxLayout;
            widget = new QSpinBox( parent );
            widget->setEnabled( is_edit_enabled );
            ( (QSpinBox *) widget )->setRange( -1000000000, 1000000000 );
            if ( !hide_field_name ) {
                widgetLayout->addWidget( new QLabel( fieldName, parent ) );
            }
            widgetLayout->addWidget( widget );

        } else if ( widgetType.toLower() == "pushbutton" ) {
            widgetLayout = new QHBoxLayout;
            widget = new QPushButton( fieldName, parent );
            widget->setEnabled( is_edit_enabled );
            widgetLayout->addWidget( widget );

        } else if (  widgetType.toLower() == "combobox" ) {
            widgetLayout = new QHBoxLayout;
            widget = new QComboBox( parent );
            widget->setEnabled( is_edit_enabled );
            QStringList cb_contents;
            if ( PyList_Check( widget_data_obj ) ) {
                for( int i = 0 ; i < PyList_Size( widget_data_obj ) ; i++ ) {
                    PyObject *list_item_obj = PyList_GetItem( widget_data_obj, i );
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
                widgetLayout->addWidget(  new QLabel( fieldName ) );
            }
            widgetLayout->addWidget( widget );

        } else if ( widgetType.toLower() == "listbox" ) {
            widgetLayout = new QVBoxLayout;
            widget = new QListWidget( parent );
            widget->setEnabled( is_edit_enabled );
            if ( PyList_Check( widget_data_obj ) ) {
                PyDataListWidgetItem::fillListWidget( (QListWidget *) widget, widget_data_obj );
            }
            if ( !hide_field_name ) {
                widgetLayout->addWidget( new QLabel( fieldName ) );
            }
            widgetLayout->addWidget( widget );

        } else if ( widgetType.toLower() == "textlabel" ) {
            widgetLayout = new QHBoxLayout;
            widget_data = PyString_AsString( widget_data_obj );
            widget = new QLabel( widget_data, parent );
            widget->setEnabled( is_edit_enabled );
            widgetLayout->addWidget( widget );

        } else if ( widgetType.toLower() == "image" ) {
            widgetLayout = new QVBoxLayout;
            widget_data = "";
            if ( widget_data_obj != Py_None ) {
                widget_data = PyString_AsString( widget_data_obj );
            }
            widget = new ImageWidget( widget_data );
            widget->setEnabled( is_edit_enabled );
            widgetLayout->addWidget( widget );

        } else if ( widgetType.toLower() == "hr" ) {
            widgetLayout = new QHBoxLayout;
            QFrame *hr = new QFrame;
            hr->setFrameShape( QFrame::HLine );
            widgetLayout->addWidget( hr );
        } else {
            if ( widgetType.toLower() != "empty" ) {
                qInfo( "This widget contains an unknown type: %s", widgetType.toStdString().data() );
            }
        }
    }
}

QString GuiWidget::getFieldName() {
    return fieldName;
}

int GuiWidget::getColSpan() {
    return colSpan;
}

int GuiWidget::getRowSpan() {
    return rowSpan;
}

Qt::Alignment GuiWidget::getAlignFlag() {
    return alignFlag;
}

QWidget *GuiWidget::getWidget() {
    return widget;
}

QString GuiWidget::getWidgetType() {
    return widgetType;
}

QLayout *GuiWidget::getWidgetLayout() {
    return widgetLayout;
}

GuiAction::GuiAction( PyObject *action_obj ) {
    PyObject *action_type_obj = PyObject_CallMethod( action_obj, (char *) "get_action_type", NULL );
    PyErr_Print();
    PyObject *widget1_obj = PyObject_CallMethod( action_obj, (char *) "get_widget1", NULL );
    PyErr_Print();
    PyObject *widget2_obj = PyObject_CallMethod( action_obj, (char *) "get_widget2", NULL );
    PyErr_Print();
    callback = PyObject_CallMethod( action_obj, (char *) "get_callback", NULL );
    PyErr_Print();
    data = PyObject_CallMethod( action_obj, (char *) "get_data", NULL );
    PyErr_Print();

    actionType = PyString_AsString( action_type_obj );
    widget1 = new GuiWidget( widget1_obj, 0, false );
    if ( widget2_obj != Py_None ) {
        widget2 = new GuiWidget( widget2_obj, 0, false );
    }
}

QString GuiAction::getActionType() {
    return actionType;
}

GuiWidget *GuiAction::getWidget1() {
    return widget1;
}

GuiWidget *GuiAction::getWidget2() {
    return widget2;
}

PyObject *GuiAction::getCallback() {
    return callback;
}

PyObject *GuiAction::getData() {
    return data;
}

void WidgetRegistry::registerWidget( GuiWidget *gui_widget ) {
    QString widget_type = gui_widget->getWidgetType();
    if ( widget_type.toLower() == "hr" ) return;
    hash[gui_widget->getFieldName()] = gui_widget;
}

GuiWidget *WidgetRegistry::getGuiWidget( QString field_name ) {
    return hash[field_name];
}

QHash<QString, GuiWidget *>::iterator WidgetRegistry::begin() {
    return hash.begin();
}

QHash<QString, GuiWidget *>::iterator WidgetRegistry::end() {
    return hash.end();
}

void WidgetRegistry::processAction( PyObject *action_obj, QWidget *parent ) {
    GuiAction gui_action( action_obj );
    processAction( gui_action, parent );
}

void WidgetRegistry::processAction( GuiAction gui_action, QWidget *parent ) {
    PyObject *fields_dict_obj = getFieldsDict();
    QString action_type = gui_action.getActionType();
    PyObject *callback_obj = gui_action.getCallback();
    QString widget1_field_name = gui_action.getWidget1()->getFieldName();
    QString widget2_field_name;
    QString widget2_widget_type;
    GuiWidget *widget2 = gui_action.getWidget2();
    if ( widget2 != NULL ) {
        widget2_field_name = gui_action.getWidget2()->getFieldName();
        widget2_widget_type = gui_action.getWidget2()->getWidgetType();
    }

    if ( action_type.toLower() == "fillfields" ) {
        if ( callback_obj != Py_None ) {
            PyObject *callback_return_obj = PyObject_CallObject( callback_obj, Py_BuildValue( "(O)", fields_dict_obj ) );
            PyErr_Print();
            fillFields( callback_return_obj );
        }

    } else if ( action_type.toLower() == "savepdf" || action_type.toLower() == "printpreview" ) {
        if ( callback_obj != Py_None ) {
            PyObject *callback_return_obj = PyObject_CallObject( callback_obj, Py_BuildValue( "(O)", fields_dict_obj ) );
            PyErr_Print();
            if ( !PyTuple_Check( callback_return_obj ) || PyTuple_Size( callback_return_obj ) == 0 ) {
                return;
            }
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

    } else if ( action_type.toLower() == "entrydialog" ) {
        QVariant *value = new QVariant;
        EntryDialog *dialog;
        bool accepted;

//        QString title = widget1_field_name;
        QString title = widget1_field_name;
        if ( title.startsWith( "&" ) ) {
            title.remove( 0, 1 );
        }

        if ( widget2_widget_type.toLower() == "lineedit" ) {
            dialog = new EntryDialog( title, EntryDialog::LINE_EDIT, value, parent );

        } else if ( widget2_widget_type.toLower() == "textedit" ) {
            dialog = new EntryDialog( title, EntryDialog::TEXT_EDIT, value, parent );

        } else if ( widget2_widget_type.toLower() == "spinbox" ) {
            dialog = new EntryDialog( title, EntryDialog::SPIN_BOX, value, parent );

        } else if ( widget2_widget_type.toLower() == "image" ) {
            ImageWidget *image_widget = (ImageWidget *) getGuiWidget(widget2_field_name)->getWidget();
            QString image_data = image_widget->getData();
            dialog = new EntryDialog( title, EntryDialog::IMAGE, value, parent, image_data );
        }
        accepted = dialog->exec();
//        qInfo( "value: %s", value->toString().toStdString().data() );
        if ( accepted ) {
            if ( callback_obj != Py_None ) {
                PyObject *value_obj = PyString_FromString( value->toString().toStdString().data() );
                PyObject *callback_return_obj = PyObject_CallObject( callback_obj, Py_BuildValue( "(O, O)", value_obj, fields_dict_obj ) );
                PyErr_Print();
//                fillFields( callback_return_obj );
                fillFields( callback_return_obj );
            }
        }

    } else if ( action_type.toLower() == "listdialog" ) {
        bool accepted;
        DualListDialog *dialog;

        QString title = widget1_field_name;
        if ( title.startsWith( "&" ) ) {
            title.remove( 0, 1 );
        }

        if ( widget2_widget_type.toLower() == "listbox" ) {
            QListWidget *owned_item_list = (QListWidget *) getGuiWidget(widget2_field_name)->getWidget();
            PyObject *owned_item_list_obj = PyDataListWidgetItem::getDataList( owned_item_list );
//            PyObject *action_data_obj = PyObject_GetAttrString( action_obj, (char *) "data" );
//            PyErr_Print();
//            dialog = new DualListDialog( title, owned_item_list_obj, action_data_obj, fields_dict_obj, parent );
            dialog = new DualListDialog( title, owned_item_list_obj, gui_action.getData(), fields_dict_obj, parent );
        }
        accepted = dialog->exec();
        if ( accepted ) {
            if ( callback_obj != Py_None ) {
                PyObject *callback_return_obj = PyObject_CallObject( callback_obj, Py_BuildValue( "(O, O)", dialog->getItemList(), fields_dict_obj ) );
                PyErr_Print();
//                fillFields( callback_return_obj );
                fillFields( callback_return_obj );
            }
        }

    } else if ( action_type.toLower() == "callbackonly" ) {
        if ( callback_obj != Py_None ) {
            PyObject_CallObject( callback_obj, Py_BuildValue( "(O)", fields_dict_obj ) );
            PyErr_Print();
        }
    }
}

void WidgetRegistry::fillFields( PyObject *fill_dict_obj ) {
    Py_ssize_t pos = 0;
    PyObject *key, *value;
    while( PyDict_Next( fill_dict_obj, &pos, &key, &value ) ) {
        QString field_name = PyString_AsString( key );
        GuiWidget *gui_widget = this->getGuiWidget( field_name );
        QString widget_type = gui_widget->getWidgetType();
        QWidget *widget = gui_widget->getWidget();

        if ( widget_type.toLower() == "lineedit" ) {
            QLineEdit *line_edit_widget = (QLineEdit *) widget;
            line_edit_widget->setText( PyString_AsString( value ) );

        } else if ( widget_type.toLower() == "textedit" ) {
            QTextEdit *text_edit_widget = (QTextEdit *) widget;
            text_edit_widget->setText( PyString_AsString( value ) );

        } else if ( widget_type.toLower() == "listbox" ) {
            QListWidget *list_widget = (QListWidget *) widget;
            PyDataListWidgetItem::fillListWidget( list_widget, value );

        } else if ( widget_type.toLower() == "spinbox" ) {
            QSpinBox *spin_box_widget = (QSpinBox *) widget;
            spin_box_widget->setValue( PyInt_AsSsize_t( value ) );

        } else if ( widget_type.toLower() == "combobox" ) {
            QComboBox *combo_box_widget = (QComboBox *) widget;
            int new_index = combo_box_widget->findText( PyString_AsString( value ) );
            combo_box_widget->setCurrentIndex( new_index );

        } else if ( widget_type.toLower() == "image" ) {
            ImageWidget *image_widget = (ImageWidget *) widget;
            image_widget->setData( PyString_AsString( value ) );

        }
    }
}

PyObject *WidgetRegistry::getFieldsDict() {
    PyObject *fields_dict_obj = PyDict_New();
    QHash<QString, GuiWidget *>::iterator iter;
    for( iter = this->begin() ; iter != this->end() ; iter ++ ) {
        QString field_name = iter.key();
        QWidget *widget = iter.value()->getWidget();
        QString widget_type = iter.value()->getWidgetType();

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
                     PyDataListWidgetItem *item = (PyDataListWidgetItem *) ( (QListWidget *) widget )->currentItem();
                     item_data = item->getData();
            }
            PyDict_SetItemString( fields_dict_obj, current_field_name.toStdString().data(), item_data );

            PyObject *item_list_obj = PyList_New( ( (QListWidget *) widget )->count() );
            for ( Py_ssize_t i = 0 ; i < ( (QListWidget *) widget )->count() ; i++ ) {
                PyDataListWidgetItem *item = (PyDataListWidgetItem *) ( (QListWidget *) widget )->item( i );
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

void WidgetRegistry::setDefaultActions( GuiAction gui_action, QWidget *parent ) {
    GuiWidget *widget1 = gui_action.getWidget1();
    QString widget1_field_name = widget1->getFieldName();
    QString widget1_type = widget1->getWidgetType();

    if ( widget1_type.toLower() == "pushbutton" ) {
        QPushButton *widget1 = (QPushButton *) getGuiWidget( widget1_field_name )->getWidget();
        parent->connect( widget1, &QPushButton::clicked, [=] ( /*bool checked*/ ) {
            processAction( gui_action, parent );
        });

    } else if ( widget1_type.toLower() == "listbox" ) {
        QListWidget *widget1 = (QListWidget *) getGuiWidget( widget1_field_name )->getWidget();
        parent->connect( widget1, &QListWidget::currentItemChanged, [=]  ( /*QListWidgetItem *current, QListWidgetItem *previous*/ ) {
            processAction( gui_action, parent );
        });
    }
}

void WidgetRegistry::fillMenuBar( QMenuBar *menu_bar, PyObject *menu_list_obj, QWidget *parent ) {
    for( Py_ssize_t i = 0 ; i < PyList_Size( menu_list_obj ) ; i++ ) {
        PyObject *menu_obj = PyList_GetItem( menu_list_obj, i );
        PyObject *menu_name_obj = PyObject_CallMethod( menu_obj, (char *) "get_menu_name", NULL );
        PyErr_Print();
        QString menu_name = PyString_AsString( menu_name_obj );
        QMenu *menu = menu_bar->addMenu( menu_name );
        PyObject *menu_action_list_obj = PyObject_CallMethod( menu_obj, (char *) "get_action_list", NULL );
        PyErr_Print();
        for( Py_ssize_t j = 0 ; j < PyList_Size( menu_action_list_obj ) ; j++ ) {
            PyObject *menu_action_obj = PyList_GetItem( menu_action_list_obj, j );
            PyObject *menu_widget1_obj = PyObject_GetAttrString( menu_action_obj, (char *) "widget1" );
            PyObject *menu_widget1_field_name_obj = PyObject_CallMethod( menu_widget1_obj, (char *) "get_field_name", NULL );
            PyErr_Print();
            QString menu_widget1_field_name = PyString_AsString( menu_widget1_field_name_obj );

            QAction *menu_action = new QAction( menu_widget1_field_name, parent );
            menu->addAction( menu_action );
            parent->connect( menu_action, &QAction::triggered, [=] ( /*bool checked*/ ) {
                processAction( menu_action_obj, parent );
            });
        }
    }
}
