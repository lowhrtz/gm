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

class EntryDialog : public QDialog {

    Q_OBJECT
    Q_ENUMS(EntryWidgetType)

public:
    enum EntryWidgetType { LINE_EDIT, TEXT_EDIT, SPIN_BOX };
    EntryDialog(QString title, EntryWidgetType type, QVariant *value, QWidget *parent = 0 );

public:

private:
    QWidget *entryWidget;
    QVariant *value;
};

#endif // YESNODIALOG_H

