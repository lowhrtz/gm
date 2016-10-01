#ifndef YESNODIALOG_H
#define YESNODIALOG_H

#include <QDialog>

class YesNoDialog : public QDialog {

    Q_OBJECT

public:
    YesNoDialog(QString title, QString message, QWidget *parent = 0);

};

class PopupDialog : public QDialog {

    Q_OBJECT

public:
    PopupDialog(QString title, QString message, QWidget *parent = 0);

};

#endif // YESNODIALOG_H

