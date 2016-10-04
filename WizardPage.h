#ifndef WIZARDPAGE_H
#define WIZARDPAGE_H

#include "CharacterCreationWizard.h"
#include "PythonInterpreter.h"
#include <QComboBox>
#include <QLabel>
#include <QStackedWidget>
#include <QWizard>
#include <QWizardPage>

// http://stackoverflow.com/questions/2133250/does-not-name-a-type-error
class CharacterCreationWizard;

class WizardPage : public QWizardPage {

    Q_OBJECT

public:
    WizardPage(PyObject *pyWizardPageInstance = Py_None, QWidget *parent = 0);
    void publicRegisterField(const QString & name, QWidget * widget, const char * property = 0, const char * changedSignal = 0 );
    void cleanupPage();
    void initializePage();
    int nextId() const;
    PyObject *parseArgTemplateString(QString templateString);

public:
   int pageId;
   CharacterCreationWizard *wizard;
   PyObject *pyWizardPageInstance;
   QVariant nextIdArgs;

private:

signals:
    
public slots:

};

class DragLabel : public QLabel {

    Q_OBJECT

public:
    DragLabel(QWidget *parent = 0);
    void setAttrFieldName(QString attrFieldName);
    QString getAttrFieldName();

protected:
    void dropEvent(QDropEvent *dropEvent);
    void dragMoveEvent(QDragMoveEvent *dragEvent);
    void dragEnterEvent(QDragEnterEvent *dragEvent);
    void mousePressEvent(QMouseEvent *ev);

public:
    QString attrFieldName;
};

class RollMethodsPage : public WizardPage {

    Q_OBJECT

    int RollMethodRole = 33;

public:
    RollMethodsPage(PyObject *pyWizardPageInstance, QWidget *parent = 0);
    void initializePage();
    bool validatePage();
    void cleanupPage();
    void fillAttributeFields();
    void publicSetField(const QString &fieldName, const QVariant &fieldValue);
    QVariant getField(QString fieldName);

private:
    QComboBox *rollMethodSelector;
    QList<QString> attributeList;
    QList<DragLabel *> diceLabelList;
    QLineEdit *pool;

public slots:
    void buttonClicked();
};

class WidgetWithCallableAndArgs {

public:
    WidgetWithCallableAndArgs(QWidget *widget, PyObject *callable, QString args);
    QWidget *getWidget();
    PyObject *getCallable();
    QString getArgs();

private:
    void setWidget(QWidget *widget);
    void setCallable(PyObject *callable);
    void setArgs(QString args);

private:
    QWidget *widget;
    PyObject *callable;
    QString args;
};

class InfoPage : public WizardPage {

    Q_OBJECT

public:
    InfoPage(PyObject *pyWizardPageInstance, QWidget *parent = 0);
    void initializePage();
    void cleanupPage();

private:
    QString getMandatoryString(QString fillString, PyObject *pyContentItem);
    void addCallable(QWidget *widget, PyObject *callable, QString argsTemplate);

private:
    QList<WidgetWithCallableAndArgs> page_init_callable_list;

};

class StackedWidget : public QStackedWidget {

    Q_OBJECT

public:
    StackedWidget(QWidget *parent, DatabaseHandler *db, PythonInterpreter *interpreter);
    PyObject *getData(int index);
    void addRowItem(QList<QVariant> *row, int displayColumn);
    void fillComboRow(QString tableName);
    void addItem(QString displayString, PyObject *data = Py_None);
    void addItems(QList<QString> displayStringList);
    void clear();

private:
    QList<PyObject *> dataList;
    PythonInterpreter *interpreter;
    DatabaseHandler *db;

};

#endif // WIZARDPAGE_H
