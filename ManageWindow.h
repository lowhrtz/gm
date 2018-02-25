#ifndef MANAGEWINDOW_H
#define MANAGEWINDOW_H

#include "CustomWidgets.h"
#include "PythonInterpreter.h"

#include <QLabel>
#include <QListWidget>
#include <QMainWindow>

class ManageWindow : public QMainWindow {

    Q_OBJECT

public:
    ManageWindow( PyObject *manage_window_obj, QWidget *parent = 0 );

private:

private:
    WidgetRegistry widget_registry;

signals:
    void onShow();

public slots:


};

#endif // MANAGEWINDOW_H
