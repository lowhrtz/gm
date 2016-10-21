#ifndef WIZARDPAGE_H
#define WIZARDPAGE_H

#include "CharacterCreationWizard.h"
#include "PythonInterpreter.h"
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
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
    PyObject *parseArgTemplateString(QString templateString) const;
    QString parseTextTemplateString(QString templateString);

protected:
    void setBanner(QString bannerFilePath);

public:
   int pageId;
   CharacterCreationWizard *wizard;
   PyObject *pyWizardPageInstance;
   QVariant nextIdArgs;

protected:
   QString pageTitle;
   QString pageSubtitle;
   PyObject *content;

protected:
//   QHash<QString, QWidget *> field_name_to_widget_hash;

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

class BindCallable : public WidgetWithCallableAndArgs {

public:
    BindCallable(QWidget *targetWidget, PyObject *callable, QString bindWidgetName, QString context);
    QString getContext();

private:
    void setContext(QString context);

private:
    QString context;
};

class QLabelWithTextTemplate {

public:
    QLabelWithTextTemplate(QLabel *label, QString templateString);
    QLabel *getLabel();
    QString getTemplateString();

private:
    void setLabel(QLabel *label);
    void setTemplateString(QString templateString);

private:
    QLabel *label;
    QString templateString;
};

class InfoPage : public WizardPage {

    Q_OBJECT

public:
    InfoPage(PyObject *pyWizardPageInstance, QWidget *parent = 0);
    void initializePage();
    void cleanupPage();

private:
    QString getMandatoryString(QString fillString, PyObject *pyContentItem);
    void addInitCallable(QWidget *widget, PyObject *callable, QString argsTemplate);
    void addBindCallable(QWidget *widget, PyObject *callable, QString bindWidget, QString context);
    void addParsableString(QLabel *label, QString templateString);

private:
    QList<WidgetWithCallableAndArgs> page_init_callable_list;
    QList<BindCallable> bind_callable_list;
    QList<QLabelWithTextTemplate> page_init_string_parse;

};

class StackedWidget : public QStackedWidget {

    Q_OBJECT
    Q_PROPERTY(int currentItemIndex READ getCurrentItemIndex NOTIFY currentItemChanged)
    Q_PROPERTY(QList<PyObject *> dataList READ getDataList NOTIFY dataListChanged)

public:
    StackedWidget(QWidget *parent, QString displayType, DatabaseHandler *db, PythonInterpreter *interpreter);
    PyObject *getData(int index);
    QList<PyObject *> getDataList();
    void addRowItem(QList<QVariant> *row, int displayColumn);
    void fillComboRow(QString tableName);
    void addItem(QString displayString, PyObject *data = Py_None, QString toolTip = NULL);
    void addItems(QList<QString> displayStringList);
    int getCurrentItemIndex();
    void setCurrentItemIndex(int itemIndex);
    QString getCurrentItemText();
    void clear();
    QString getCurrentToolTip();
    QListWidgetItem *takeItem(int index);
    void insertItem(int index, QString displayString, PyObject *data = Py_None, QString toolTip = NULL);

public:

private:
    QList<PyObject *> m_dataList;
    PythonInterpreter *interpreter;
    DatabaseHandler *db;
    int m_currentItemIndex;

signals:
    void currentItemChanged(int itemIndex);
    void dataListChanged(QList<PyObject *> m_dataList);

public slots:
    void currentItemChangedSlot(int itemIndex);

};

class DualListCallableAndArgs : public WidgetWithCallableAndArgs {

public:
    DualListCallableAndArgs(QString type, PyObject *callable, QString args);
    QString getType();

private:
    void setType(QString type);

private:
    QString type;
};

class DualListSelection : public WizardPage {

    Q_OBJECT

    int OriginalIndexRole = 33;

public:
    DualListSelection(PyObject *pyWizardPageInstance, QWidget *parent = 0);
    void initializePage();
    void cleanupPage();
    bool isComplete() const;

public:
    Py_ssize_t slotsTotal;
    double slotsTotalFloat;

private:
    void addInitCallable(QString type, PyObject *callable, QString argsTemplate);

private:
    StackedWidget *firstList, *secondList;
    QPushButton *addButton, *removeButton;
    QLabel *slotsTextLabel, *slotsTotalLabel;
    QString slotsType, toolTipField;
    QList<DualListCallableAndArgs> page_init_callable_list;
    QList<int> firstListIndices;
    QList<int> secondListIndices;

};

//template <class T> class VPtr
//{
//public:
//    static T* asPtr(QVariant v)
//    {
//    return  (T *) v.value<void *>();
//    }

//    static QVariant asQVariant(T* ptr)
//    {
//    return qVariantFromValue((void *) ptr);
//    }
//};

#endif // WIZARDPAGE_H
