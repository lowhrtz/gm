#include "PyToolButton.h"

PyToolButton::PyToolButton(QWidget *parent)
    :QToolButton(parent){

}

void PyToolButton::setPyObject(PyObject *py_object, QString arg_template) {
    connect(this, SIGNAL(clicked()), this, SLOT(reemitClicked()));
    this->argTemplate = arg_template;
    this->pyObject = py_object;
}

void PyToolButton::reemitClicked() {
    emit clicked(pyObject, argTemplate);
}
