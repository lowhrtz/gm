#include "CustomWidgets.h"

#include <QPushButton>
#include <QLayout>

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

void PyDataListWidgetItem::fillListWidget(  QListWidget *list_widget, PyObject *list_obj  ) {
    list_widget->clear();
    for ( Py_ssize_t i = 0 ; i < PyList_Size( list_obj ) ; i++ ) {
        PyObject *list_item_obj = PyList_GetItem( list_obj, i );

//        if ( PyString_Check( list_item_obj ) ) {
//            list_widget->addItem( new PyDataListWidgetItem( PyString_AsString( list_item_obj ), list_item_obj, list_widget ) );

//        } else if ( PyDict_Check( list_item_obj ) ) {
//            PyObject *table_name_obj = PyDict_GetItemString( list_item_obj, "TableName" );
//            QString table_name = PyString_AsString( table_name_obj );
//            QString display = pi_global->getColList( table_name )[ pi_global->getDisplayCol( table_name ) ];
//            PyObject *display_name_obj = PyDict_GetItemString( list_item_obj, display.toStdString().data() );
//            QString display_name = PyString_AsString( display_name_obj );
//            list_widget->addItem( new PyDataListWidgetItem( display_name, list_item_obj, list_widget ));

//        } else if ( PyTuple_Check( list_item_obj ) ) {
//            PyObject *display_obj = PyTuple_GetItem( list_item_obj, 0 );
//            PyObject *dict_obj = PyTuple_GetItem( list_item_obj, 1 );
//            QString display_name = PyString_AsString( display_obj );
//            list_widget->addItem( new PyDataListWidgetItem( display_name, dict_obj, list_widget ) );
//        }
        PyDataListWidgetItem::addItemToWidget( list_widget, list_item_obj );
    }
}

void PyDataListWidgetItem::addItemToWidget(  QListWidget *list_widget, PyObject *list_item_obj  ) {
    if ( PyString_Check( list_item_obj ) ) {
        list_widget->addItem( new PyDataListWidgetItem( PyString_AsString( list_item_obj ), list_item_obj, list_widget ) );

    } else if ( PyDict_Check( list_item_obj ) ) {
        PyObject *table_name_obj = PyDict_GetItemString( list_item_obj, "TableName" );
        QString table_name = PyString_AsString( table_name_obj );
        QString display = pi_global->getColList( table_name )[ pi_global->getDisplayCol( table_name ) ];
        PyObject *display_name_obj = PyDict_GetItemString( list_item_obj, display.toStdString().data() );
        QString display_name = PyString_AsString( display_name_obj );
        list_widget->addItem( new PyDataListWidgetItem( display_name, list_item_obj, list_widget ));

    } else if ( PyTuple_Check( list_item_obj ) ) {
        PyObject *display_obj = PyTuple_GetItem( list_item_obj, 0 );
        PyObject *dict_obj = PyTuple_GetItem( list_item_obj, 1 );
        QString display_name = PyString_AsString( display_obj );
        list_widget->addItem( new PyDataListWidgetItem( display_name, dict_obj, list_widget ) );
    }
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
    QString slots_name = PyString_AsString( slots_name_obj );
    PyObject *add_callback = PyDict_GetItemString( action_data_obj, (char *) "add" );
    PyErr_Print();
    PyObject *remove_callback = PyDict_GetItemString( action_data_obj, (char *) "remove" );
    PyErr_Print();

    PyObject *avail_item_list_obj = PyObject_CallObject( fill_avail_callback, Py_BuildValue( "(O,O)", owned_item_list_obj, fields_obj ) );
    PyErr_Print();


    if ( category_field_obj != NULL && PyString_Check( category_field_obj ) ) {
        for ( Py_ssize_t i = 0; i < PyList_Size( avail_item_list_obj ); i++ ) {
            PyObject *item_obj = PyList_GetItem( avail_item_list_obj, i );
            PyObject *item_dict_obj;

            if ( PyDict_Check( item_obj ) ) {
                item_dict_obj = item_obj;
            } else if ( PyTuple_Check( item_obj ) ) {
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
                PyDataListWidgetItem::addItemToWidget( cat_list, item_obj );

            } else {
                QListWidget *cat_list = category_hash[category];
                PyDataListWidgetItem::addItemToWidget( cat_list, item_obj );
            }
        }
    } else {
        QListWidget *avail_list = new QListWidget( this );
//        category_hash["Avail"] = avail_list;
        tabbedAvailLists->addTab( avail_list, "Avail" );
        tabbedAvailLists->tabBar()->hide();
        PyDataListWidgetItem::fillListWidget( avail_list, avail_item_list_obj );
        if( avail_list->count() > 0 ) {
            avail_list->setCurrentRow( 0 );
        }

    }


    PyDataListWidgetItem::fillListWidget( chosenList, owned_item_list_obj );
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
//    list_layout->addWidget( avail_list );
    list_layout->addLayout( button_layout );
//    list_layout->addWidget( chosenList );
    list_layout->addWidget( tabbed_chosen_list );
    layout->addLayout( list_layout );

//    layout->setSizeConstraint( QLayout::SetFixedSize );
    setLayout( layout );

    connect( add_button, &QPushButton::clicked, [=] () {
        QListWidget *current_list = (QListWidget *) tabbedAvailLists->currentWidget();
        //qInfo( "Current Index: %i", avail_list->currentRow() );
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

        chosenList->addItem( new PyDataListWidgetItem( new_display_string, current_item->getData(), chosenList, current_list ) );

    });

    connect( del_button, &QPushButton::clicked, [=] () {
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
            original_list->addItem( new PyDataListWidgetItem( new_display_string, current_item->getData(), original_list ) );
        }

        chosenList->takeItem( chosenList->currentRow() );

    });
}

QListWidget *DualListWidget::getChosenList() {
    return chosenList;
}
