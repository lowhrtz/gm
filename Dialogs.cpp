#include "Dialogs.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

YesNoDialog::YesNoDialog(QString title, QString message, QWidget *parent) :
    QDialog(parent) {
    setWindowTitle(title);
    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *messageLabel = new QLabel(message, this);
    QPushButton *yes_button = new QPushButton("Yes", this);
    QPushButton *no_button = new QPushButton("No", this);
    QDialogButtonBox *button_frame = new QDialogButtonBox(this);
    layout->addWidget(messageLabel);
    yes_button->setDefault(true);
    button_frame->addButton(yes_button, QDialogButtonBox::AcceptRole);
    button_frame->addButton(no_button, QDialogButtonBox::RejectRole);
    connect(button_frame, &QDialogButtonBox::accepted, this, &YesNoDialog::accept);
    connect(button_frame, &QDialogButtonBox::rejected, this, &YesNoDialog::reject);
    layout->addWidget(button_frame);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(layout);
}

PopupDialog::PopupDialog(QString title, QString message, QWidget *parent) :
    QDialog(parent) {
    setWindowTitle(title);
    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *messageLabel = new QLabel(message, this);
    QPushButton *ok_button = new QPushButton("OK", this);
    QDialogButtonBox *button_box = new QDialogButtonBox(this);
    layout->addWidget(messageLabel);
    ok_button->setDefault(true);
    button_box->addButton(ok_button, QDialogButtonBox::AcceptRole);
    connect(button_box, &QDialogButtonBox::accepted, this, &PopupDialog::accept);
    layout->addWidget(button_box);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(layout);
}
