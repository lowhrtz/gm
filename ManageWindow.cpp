#include "ManageWindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>

ManageWindow::ManageWindow( PyObject *manage_window_instance, QWidget *parent )
    : QMainWindow( parent ) {
    PyObject *class_name_obj, *widget_matrix, *matrix_row, *widget_obj, *field_name_obj, *widget_type_obj,
             *col_span_obj, *row_span_obj, *widget_align_obj, *widget_data_obj, *list_item_obj, *connect_list_obj, *connect_tuple,
             *trigger_widget_obj, *triggered_widget_obj, *callback_obj, *trigger_field_name_obj, *triggered_field_name_obj;
    Py_ssize_t number_of_rows, number_of_cols, col_span, row_span;
    QString class_name, field_name, widget_align, widget_data, widget_type, trigger_field_name, trigger_widget_type, triggered_field_name, triggered_widget_type;
    QWidget *widget, *trigger_widget, *triggered_widget;
    bool hide_field_name;
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

//            QHBoxLayout *widget_layout = new QHBoxLayout;
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
                widget_layout->addWidget( widget );

            } else if ( widget_type.toLower() == "textedit" ) {
                widget_layout = new QVBoxLayout;
                widget = new QTextEdit( "QTextEdit" );
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
                if ( !hide_field_name ) {
                    widget_layout->addWidget( new QLabel( field_name, this ) );
                }
                widget_layout->addWidget( widget );

            } else if ( widget_type.toLower() == "spinbox" ) {
                widget_layout = new QHBoxLayout;
                widget = new QSpinBox( this );
                ( (QSpinBox *) widget )->setRange( -1000, 1000 );
                if ( !hide_field_name ) {
                    widget_layout->addWidget( new QLabel( field_name, this ) );
                }
                widget_layout->addWidget( widget );

            }else if ( widget_type.toLower() == "pushbutton" ) {
                widget_layout = new QHBoxLayout;
                widget = new QPushButton( field_name, this );
                widget_layout->addWidget( widget );

            } else if (  widget_type.toLower() == "combobox" ) {
                widget_layout = new QHBoxLayout;
                widget = new QComboBox( this );
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
                if ( widget_data_obj != Py_None ) {
                    for ( Py_ssize_t i = 0 ; i < PyList_Size( widget_data_obj ) ; i++ ) {
                        ( (QListWidget *) widget )->addItem( PyString_AsString( PyList_GetItem( widget_data_obj, i ) ) );
                    }
                }
                if ( !hide_field_name ) {
                    widget_layout->addWidget( new QLabel( field_name ) );
                }
                widget_layout->addWidget( widget );

            } else if ( widget_type.toLower() == "textlabel" ) {
                widget_layout = new QHBoxLayout;
                widget_data = PyString_AsString( widget_data_obj );
                widget = new QLabel( widget_data, this );
                widget_layout->addWidget( widget );

            } else if ( widget_type.toLower() == "image" ) {
                widget_layout = new QVBoxLayout;
                widget = new QLabel;
                if ( widget_data_obj != Py_None ) {
                    widget_data = PyString_AsString( widget_data_obj );
                    QByteArray ba = QByteArray::fromBase64( widget_data.toStdString().data() );
                    //QImage image = QImage::fromData( ba, image_type.toStdString().data() );
                    QImage image = QImage::fromData( ba );
                    QPixmap pixmap = QPixmap( QPixmap::fromImage( image ) );
                    if( pixmap.height() > 200 ) {
                        pixmap = pixmap.scaledToHeight( 200 );
                    }
                    ( (QLabel *)widget )->setPixmap( pixmap );
                }
                if ( !hide_field_name ) {
                    widget_layout->addWidget( new QLabel( field_name, this ) );
                }
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

//    this->layout()->update();
//    this->layout()->activate();
//    QRect parent_rect = parent->geometry();
//    qInfo( "parent_rect.x(): %i", parent_rect.x() );
//    qInfo( "parent_rect.y(): %i", parent_rect.y() );
//    qInfo( "parent_rect.width(): %i", parent_rect.width() );
//    qInfo( "parent_rect.height(): %i", parent_rect.height() );
//    qInfo( "rect.width(): %i", this->width() );
//    qInfo( "rect.height(): %i", geometry().height() );
//    move( parent_rect.x() + ( parent_rect.width() - geometry().width() ) / 2, parent_rect.y() + ( parent_rect.height() - geometry().height() ) / 2 );

    connect_list_obj = PyObject_CallMethod( manage_window_instance, (char *) "get_connect_list", NULL );
    PyErr_Print();
    for( Py_ssize_t i = 0 ; i < PyList_Size( connect_list_obj ) ; i++ ) {
        connect_tuple = PyList_GetItem( connect_list_obj, i );
        trigger_widget_obj = PyTuple_GetItem( connect_tuple, 0 );
        triggered_widget_obj = PyTuple_GetItem( connect_tuple, 1 );
        callback_obj = PyTuple_GetItem( connect_tuple, 2 );

        trigger_field_name_obj = PyObject_CallMethod( trigger_widget_obj, (char *) "get_field_name", NULL);
        PyErr_Print();
        triggered_field_name_obj = PyObject_CallMethod( triggered_widget_obj, (char *) "get_field_name", NULL);
        PyErr_Print();
        trigger_field_name = PyString_AsString( trigger_field_name_obj );
        triggered_field_name = PyString_AsString( triggered_field_name_obj );

        trigger_widget = widget_registry[trigger_field_name].first;
        trigger_widget_type = widget_registry[trigger_field_name].second;
        triggered_widget = widget_registry[triggered_field_name].first;
        triggered_widget_type = widget_registry[triggered_field_name].second;

        if ( trigger_widget_type.toLower() == "pushbutton" ) {
            connect( (QPushButton *) trigger_widget, &QPushButton::clicked, [=] ( bool checked ) {
                if ( callback_obj != Py_None ) {
                    PyObject *callback_return_obj = PyObject_CallObject( callback_obj, NULL );
                    PyErr_Print();

                    if ( triggered_widget_type.toLower() == "lineedit" ) {
                        ( (QLineEdit *) triggered_widget )->setText( PyString_AsString( callback_return_obj ) );
                    }
                }
            } );
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


ManageListWidgetItem::ManageListWidgetItem(QString display_text, PyObject *data, QListWidget *parent)
    : QListWidgetItem( display_text, parent, QListWidgetItem::UserType) {
    this->data = data;
}
