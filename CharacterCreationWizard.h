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
//    PyObject *getField(PyObject *self, PyObject *args);
    PyObject *fieldsDict = PyDict_New();
    PyObject *pyWizardPageList = PyDict_New();

private:
    QList<WizardPage *> getWizardPages();
    WizardPage *getWizardPage(PyObject *pyWizardPage);
    WizardPage *getInfoPage(PyObject *pyWizardPageInstance);
//    WizardPage *getRollMethodsPage(PyObject *pyWizardPageInstance);
    QString getMandatoryString(QString fillString, PyObject *pyContentItem);

public:
    QHash<QString, QString> attributes;
    QStringList attributeList;
    QHash<QString, QWidget *> field_name_to_widget_hash;
    QStringList pyOnlyFields;

private:
    PythonInterpreter *interpreter;
    DatabaseHandler *db;

public slots:
    void buttonPushed(PyObject *currentActionTuple);
};
//extern "C" PyObject *CharacterCreationWizard::getField(PyObject *self, PyObject *args);

#endif // CHARACTERCREATIONWIZARD_H
