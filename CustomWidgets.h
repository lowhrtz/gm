#ifndef CUSTOMWIDGETS_H
#define CUSTOMWIDGETS_H

#include "PythonInterpreter.h"

#include <QLabel>
#include <QListWidgetItem>
#include <QWidget>
#include <QString>

class ImageWidget : public QLabel {

public:
    ImageWidget( QString base64_data, QWidget *parent = 0 );
    QString getData();
    void setData( QString base64_data );

/*Static Methods*/
public:
    static void setImageWithBase64( QLabel *image_widget, QString base64_string );

private:
    QString data;

};

class PyDataListWidgetItem : public QListWidgetItem {

public:
    PyDataListWidgetItem( QString display_text, PyObject *data, QListWidget *parent = 0 );
    PyObject *getData();

/*Static Methods*/
public:
    static void fillListWidget( QListWidget *list_widget, PyObject *list_obj );
    static PyObject *getDataList(QListWidget *list_widget );

private:
    PyObject *data;

};

class DualListWidget : public QWidget {

public:
    DualListWidget(PyObject *owned_item_list_obj, PyObject *action_data_obj, PyObject *fields_obj, QWidget *parent = 0 );
    QListWidget *getChosenList();

private:
    QListWidget *chosenList;

};

#endif // CUSTOMWIDGETS_H

