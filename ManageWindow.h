#ifndef MANAGEWINDOW_H
#define MANAGEWINDOW_H

#include "PythonInterpreter.h"

#include <QLabel>
#include <QListWidget>
#include <QMainWindow>

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

signals:
    void onShow();

public slots:


};

#endif // MANAGEWINDOW_H
