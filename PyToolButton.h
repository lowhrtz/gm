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
    void setPyObject(PyObject *py_object, QString arg_template);

signals:
    void clicked(PyObject *py_object, QString arg_template);

private slots:
    void reemitClicked();

private:
    PyObject *pyObject;
    QString argTemplate;
};

#endif // PYTOOLBUTTON_H
