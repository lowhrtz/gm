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

DualListWidget::DualListWidget(PyObject *owned_item_list_obj, PyObject *action_data_obj, PyObject *fields_obj, QWidget *parent)
    : QWidget( parent ){
    QListWidget *avail_list = new QListWidget( this );
    chosenList = new QListWidget( this );

    if ( !PyDict_Check( action_data_obj ) ) {
        qInfo( "The data parameter for a ListDialog Action must be a dictionary of callables." );
        return;
    }

    PyObject *fill_avail_callback = PyDict_GetItemString( action_data_obj, (char *) "fill_avail" );
    PyErr_Print();
    PyObject *slots_callback = PyDict_GetItemString( action_data_obj, (char *) "slots" );
    PyErr_Print();
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
    QVariant slots_callback_return;
    if( PyInt_Check( slots_callback_return_obj ) ) {
        slots_callback_return.setValue( PyInt_AsSsize_t( slots_callback_return_obj ) );
    } else if( PyFloat_Check( slots_callback_return_obj ) ) {
        slots_callback_return = PyFloat_AsDouble( slots_callback_return_obj );
    } else {
        slots_callback_return = 0;
    }

    QPushButton *add_button = new QPushButton( "Add", this );
    QPushButton *del_button = new QPushButton( "Remove", this );

    connect( add_button, &QPushButton::clicked, [=] ( bool checked ) {
        qInfo( "Current Index: %i", avail_list->currentRow() );
        PyDataListWidgetItem *current_item = (PyDataListWidgetItem *) avail_list->currentItem();
        PyObject *add_callback_return = PyObject_CallObject( add_callback, Py_BuildValue( "(O,O)", current_item->getData(), fields_obj ) );
        PyErr_Print();
    });

    QVBoxLayout *layout = new QVBoxLayout;

    QLabel *slots_label = new QLabel( slots_callback_return.toString(), this );
    layout->addWidget( slots_label );

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
}

QListWidget *DualListWidget::getChosenList() {
    return chosenList;
}
