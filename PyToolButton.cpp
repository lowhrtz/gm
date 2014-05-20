#include "PyToolButton.h"

PyToolButton::PyToolButton(QWidget *parent)
    :QToolButton(parent){

}

void PyToolButton::setPyAction(PyObject *pyAction) {
    connect(this, SIGNAL(clicked()), this, SLOT(reemitClicked()));
    this->currentAction = pyAction;
}

void PyToolButton::reemitClicked() {
    emit clicked(currentAction);
}
