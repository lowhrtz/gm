#ifndef CHARACTERCREATIONWIZARD_H
#define CHARACTERCREATIONWIZARD_H

#include "PythonInterpreter.h"
#include "DatabaseHandler.h"
#include "WizardPage.h"
#include <QComboBox>
#include <QWizard>

class WizardPage;

class ComboRow : public QComboBox {

    Q_OBJECT

public:
    ComboRow(QWidget *parent, DatabaseHandler *db, PythonInterpreter *interpreter);
    void addRowItem(QList<QVariant> *row, int displayColumn);
    void fillComboRow(QString tableName);

private:
    PythonInterpreter *interpreter;
    DatabaseHandler *db;
};

class CharacterCreationWizard : public QWizard {

    Q_OBJECT

public:
    CharacterCreationWizard(QWidget *parent, DatabaseHandler *db, PythonInterpreter *interpreter);
    void accept();
    DatabaseHandler *getDb();
    PythonInterpreter *getPythonInterpreter();

private:
    QList<WizardPage *> getWizardPages();
    WizardPage *getWizardPage(PyObject *pyWizardPage);
    WizardPage *getInfoPage(PyObject *pyWizardPageInstance);
//    WizardPage *getRollMethodsPage(PyObject *pyWizardPageInstance);
    QString getMandatoryString(QString fillString, PyObject *pyContentItem);

public:
    QHash<QString, QString> attributes;
    QHash<QString, QWidget *> field_name_to_widget_hash;

private:
    PythonInterpreter *interpreter;
    DatabaseHandler *db;

public slots:
    void buttonPushed(PyObject *currentActionTuple);
};

#endif // CHARACTERCREATIONWIZARD_H
