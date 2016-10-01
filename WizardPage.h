#ifndef WIZARDPAGE_H
#define WIZARDPAGE_H

#include "PythonInterpreter.h"
#include <QComboBox>
#include <QLabel>
//#include <QVariant>
#include <QWizard>
#include <QWizardPage>

class WizardPage : public QWizardPage {

    Q_OBJECT

public:
    WizardPage(PyObject *pyWizardPageInstance = Py_None, QWidget *parent = 0);
    void publicRegisterField(const QString & name, QWidget * widget, const char * property = 0, const char * changedSignal = 0 );
    void cleanupPage();
    void initializePage();
    int nextId() const;

public:
   int pageId;
   QWizard *wizard;
   PyObject *pyWizardPageInstance;
   QVariant nextIdArgs;

signals:
    
public slots:
//    void setNextIdArgs(int int_param) {
//        qInfo("int_param: %d", int_param);
//        nextIdArgs = QString::number(int_param);
//    }
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
    void fillAttributeFields();
    void publicSetField(const QString &fieldName, const QVariant &fieldValue);
    QVariant getField(QString fieldName);

private:
    QComboBox *rollMethodSelector;
    QList<QString> attributeList;
    QList<DragLabel *> diceLabelList;

public slots:
//    void rollMethodChanged(QString rollMethodString);
    void buttonClicked();
};

#endif // WIZARDPAGE_H
