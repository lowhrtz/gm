#ifndef MANAGEWINDOW_H
#define MANAGEWINDOW_H

#include "PythonInterpreter.h"

#include <QLabel>
#include <QListWidget>
#include <QMainWindow>

class ManageListWidgetItem : public QListWidgetItem {

public:
    ManageListWidgetItem( QString display_text, PyObject *data, QListWidget *parent = 0 );
    PyObject *getData();

private:
    PyObject *data;

};


class ManageWindow : public QMainWindow {

    Q_OBJECT

public:
    ManageWindow( PyObject *manage_window_obj, QWidget *parent = 0 );

private:
    void registerWidget( QString field_name, QString widget_type, QWidget *widget );
    void processAction( PyObject *action_obj );
    void fillFields( PyObject *fill_dict_obj );
    PyObject *getFieldsDict();

private:
    QHash<QString, std::pair<QWidget *, QString> > widget_registry;
//    PyObject *fields_dict_obj = PyDict_New();

signals:
    void onShow();

public slots:

public:
    static void fillListWidget( QListWidget *list_widget, PyObject *list_obj );
//    static void setImageWithBase64( QLabel *image_widget, QString base64_string );

};

#endif // MANAGEWINDOW_H
