#ifndef MANAGEWINDOW_H
#define MANAGEWINDOW_H

#include "PythonInterpreter.h"

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

private:
    QHash<QString, std::pair<QWidget *, QString> > widget_registry;

signals:
    void onShow();

public slots:
};

#endif // MANAGEWINDOW_H
