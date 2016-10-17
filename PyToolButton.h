#ifndef PYTOOLBUTTON_H
#define PYTOOLBUTTON_H

#ifdef _WIN32
    #include <QToolButton>
    #include <Python.h>
#else
    #include <Python.h>
    #include <QToolButton>
#endif

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
