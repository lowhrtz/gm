#ifndef PYTOOLBUTTON_H
#define PYTOOLBUTTON_H

#include <Python.h>
#include <QToolButton>

class PyToolButton : public QToolButton {

    Q_OBJECT

public:
    PyToolButton(QWidget *parent);
    void setPyAction(PyObject *pyAction);

signals:
    void clicked(PyObject *pyAction);

private slots:
    void reemitClicked();

public:
    PyObject *currentAction;
};

#endif // PYTOOLBUTTON_H
