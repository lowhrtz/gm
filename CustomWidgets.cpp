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

PyDataListWidgetItem::PyDataListWidgetItem( QString display_text, PyObject *data, QListWidget *parent )
    : QListWidgetItem( display_text, parent, QListWidgetItem::UserType ) {
    this->data = data;
}

PyObject *PyDataListWidgetItem::getData() {
    return data;
}

void PyDataListWidgetItem::fillListWidget(QListWidget *list_widget, PyObject *list_obj) {
    list_widget->clear();
    for ( Py_ssize_t i = 0 ; i < PyList_Size( list_obj ) ; i++ ) {
        PyObject *list_item_obj = PyList_GetItem( list_obj, i );

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
    QListWidget *avail_list = new QListWidget( this );
    chosenList = new QListWidget( this );

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

    PyDataListWidgetItem::fillListWidget( avail_list, avail_item_list_obj );
    if( avail_list->count() > 0 ) {
        avail_list->setCurrentRow( 0 );
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

    list_layout->addWidget( avail_list );
    list_layout->addLayout( button_layout );
    list_layout->addWidget( chosenList );
    layout->addLayout( list_layout );

//    layout->setSizeConstraint( QLayout::SetFixedSize );
    setLayout( layout );

    connect( add_button, &QPushButton::clicked, [=] () {
        //qInfo( "Current Index: %i", avail_list->currentRow() );
        PyDataListWidgetItem *current_item = (PyDataListWidgetItem *) avail_list->currentItem();
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
            avail_list->takeItem( avail_list->currentRow() );
        }

        QString new_display_string;
        if( new_display_obj == NULL ) {
            new_display_string = current_item->text();
        } else if( PyString_Check( new_display_obj ) ) {
            new_display_string = PyString_AsString( new_display_obj );
        } else {
            new_display_string = current_item->text();
        }

        chosenList->addItem( new PyDataListWidgetItem( new_display_string, current_item->getData(), chosenList ) );

    });

    connect( del_button, &QPushButton::clicked, [=] () {
        PyDataListWidgetItem *current_item = (PyDataListWidgetItem *) chosenList->currentItem();
        PyObject *remove_callback_return = PyObject_CallObject( remove_callback, Py_BuildValue( "(O,O)", current_item->getData(), fields_obj ) );
    });
}

QListWidget *DualListWidget::getChosenList() {
    return chosenList;
}
