#include "CharacterCreationWizard.h"
#include "PyToolButton.h"
#include "Dice.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

CharacterCreationWizard::CharacterCreationWizard(QWidget *parent, DatabaseHandler *db, PythonInterpreter *interpreter)
    : QWizard(parent) {
    QList<QWizardPage *> wizardPages;

    this->db = db;
    this->interpreter = interpreter;
    this->setWizardStyle(QWizard::ClassicStyle);
    wizardPages = getWizardPages();
    for(int i = 0;i < wizardPages.size();i++) {
//        printf("Page Title: %s\n", wizardPages.at(i)->title().toAscii().data());
        this->addPage(wizardPages.at(i));
    }
    this->setWindowTitle("Character Creation Wizard");
//    printf("ccWizard instantiated.\n");
}

void CharacterCreationWizard::accept() {
//    printf("Wizard Accepted\n");
    QDialog::accept();
}

QList<QWizardPage *> CharacterCreationWizard::getWizardPages() {
    QList<QWizardPage *> pageList;
    QList<PyObject *> pyWizPageList;

    interpreter->initPython();

    pyWizPageList = interpreter->getWizardPages();

    for (int i = 0; i < pyWizPageList.size(); i++) {
        pageList.append(getWizardPage(pyWizPageList.at(i)));
    }

    Py_Finalize();
    return pageList;
}

QWizardPage *CharacterCreationWizard::getWizardPage(PyObject *pyWizardPage) {
    QWizardPage *wizardPage;
    PyObject *pyWizardPageInstance;
    QString pageTemplate, pageTitle, pageSubtitle, layoutString, content;
    QLabel *contentLabel;

    if(!PyType_Check(pyWizardPage)) {
        printf("Error: Not a Type Object!\n");
        wizardPage = new QWizardPage;
        return wizardPage;
    }

    pyWizardPageInstance = PyObject_CallObject(pyWizardPage, NULL);
    pageTemplate = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_template", NULL));
    if(pageTemplate.toLower() == "infopage") {
        return getInfoPage(pyWizardPageInstance);
    } else if(pageTemplate.toLower() == "rollmethodspage") {
//        return getRollMethodsPage(pyWizardPageInstance);
        return new RollMethodsPage(pyWizardPageInstance, this);
    }

    pageTitle = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_title", NULL));
//    printf("pyPageTitle: %s\n", pageTitle.toAscii().data());
    pageSubtitle = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_subtitle", NULL));
    layoutString = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_layout", NULL));
    content = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_content", NULL));
//    PyErr_Print();
//    PyErr_Clear();

    wizardPage = new QWizardPage;
    wizardPage->setTitle(pageTitle);
    wizardPage->setSubTitle(pageSubtitle);

    contentLabel = new QLabel(content);
    if(layoutString.toLower() == "vertical") {
        QVBoxLayout *vlayout = new QVBoxLayout;
        vlayout->addWidget(contentLabel);
        wizardPage->setLayout(vlayout);

        return wizardPage;
    } else {
        QHBoxLayout *hlayout = new QHBoxLayout;
        hlayout->addWidget(contentLabel);
        wizardPage->setLayout(hlayout);

        return wizardPage;
    }

}

WizardPage *CharacterCreationWizard::getInfoPage(PyObject *pyWizardPageInstance) {
//    QWizardPage *wizardPage = new QWizardPage;
    WizardPage *wizardPage = new WizardPage(this);
    PyObject *pyContent, *pyContentItem,
            *firstTupleItem, *secondTupleItem;
    Py_ssize_t pyContentSize, tupleSize;
    QGridLayout *layout;
    QString pageTitle, pageSubtitle, itemType, itemName;

    pyContent = PyObject_CallMethod(pyWizardPageInstance, (char *) "get_content", NULL);
    if(!PyList_Check(pyContent)) {
        printf("Content not a Python list!\n");
        return wizardPage;
    }

    pageTitle = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_title", NULL));
    pageSubtitle = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_subtitle", NULL));
    wizardPage->setTitle(pageTitle);
    wizardPage->setSubTitle(pageSubtitle);

    layout = new QGridLayout;
//    layout->setVerticalSpacing(10);
    pyContentSize = PyList_Size(pyContent);
    for(int i = 0;i < pyContentSize;i++) {
        pyContentItem = PyList_GetItem(pyContent, i);
        if(!PyTuple_Check(pyContentItem) ||
                (tupleSize = PyTuple_Size(pyContentItem)) < 2) {
            printf("Content item not a Python tupel of at least size 2!\n");
            continue;
        }
        firstTupleItem = PyTuple_GetItem(pyContentItem, 0);
        secondTupleItem = PyTuple_GetItem(pyContentItem, 1);
        itemType = PyString_AsString(firstTupleItem);
        itemName = PyString_AsString(secondTupleItem);

        if(itemType.toLower() == "text") {
            QLabel *textLabel = new QLabel(itemName, wizardPage);
            layout->addWidget(textLabel, i, 0, 1, 2, Qt::AlignCenter);
        } else if(itemType.toLower() == "field") {
            QLabel *fieldLabel = new QLabel(itemName);
            QLineEdit *field = new QLineEdit(wizardPage);
            wizardPage->publicRegisterField(getMandatoryString(itemName, pyContentItem), field);
            layout->addWidget(fieldLabel, i, 0, Qt::AlignLeft);
            layout->addWidget(field, i, 1);
        } else if(itemType.toLower() == "choose") {
            if(tupleSize < 4) {
                printf("Choose tuple has a size of less than 4!\n");
                continue;
            }
            QLabel *chooseLabel = new QLabel(itemName);
            ComboRow *choose = new ComboRow(wizardPage, db, interpreter);
            QString source(PyString_AsString(PyTuple_GetItem(pyContentItem, 2)));
            PyObject *fourthTupleItem = PyTuple_GetItem(pyContentItem, 3);
            if(source.toLower() == "settings") {
                QString propName(PyString_AsString(fourthTupleItem));
                choose->addItems(interpreter->settingAsStringList(propName));
            } else if(source.toLower() == "method") {
            } else if(source.toLower() == "function") {
                PyObject *fifthTupleItem;
                if(tupleSize < 5 ||
                        (fifthTupleItem = PyTuple_GetItem(pyContentItem, 4)) == Py_None) {
                    fifthTupleItem = NULL;
                }
                PyObject *functionReturn = PyObject_CallObject(fourthTupleItem, fifthTupleItem);
                if(!functionReturn ||
                        !PyList_Check(functionReturn)) {
                    printf("Error calling function!\n");
                    PyErr_Print();
                    PyErr_Clear();
                    continue;
                }
                Py_ssize_t listSize = PyList_Size(functionReturn);
                for(int j = 0;j < listSize;j++) {
                    PyObject *pyListItem = PyList_GetItem(functionReturn, j);
                    if(!pyListItem ||
                            !PyString_Check(pyListItem)) {
                        printf("Function returned a list that contains at least 1 item that is not a string.\n");
                        PyErr_Print();
                        PyErr_Clear();
                        continue;
                    }
                    QString listItem = PyString_AsString(pyListItem);
                    choose->addItem(listItem);
                }
            } else {
                QString propName(PyString_AsString(fourthTupleItem));
//                fillCombo(choose, propName);
                choose->fillComboRow(propName);
            }
            wizardPage->publicRegisterField(getMandatoryString(itemName, pyContentItem), choose);
            layout->addWidget(chooseLabel, i, 0, Qt::AlignLeft);
            layout->addWidget(choose, i, 1);
        } else if(itemType.toLower() == "checkbox") {
            QCheckBox *checkbox = new QCheckBox(itemName, wizardPage);
            wizardPage->publicRegisterField(getMandatoryString(itemName, pyContentItem), checkbox);
            layout->addWidget(checkbox, i, 0);

        } else if(itemType.toLower() == "button" || itemType.toLower() == "buttoncenter") {
            PyToolButton *button = new PyToolButton(wizardPage);
            button->setText(itemName);
            button->setToolButtonStyle(Qt::ToolButtonTextOnly);
            layout->addWidget(button, i, 0, 1, 2, Qt::AlignCenter);
            if(tupleSize > 2) {
                PyObject *actionName = PyTuple_GetItem(pyContentItem, 2);
                PyObject *pyAction = PyObject_CallMethodObjArgs(pyWizardPageInstance, actionName, NULL);
                if(!pyAction ||
                        !PyTuple_Check(pyAction)) {
                    printf("Error getting action tuple!\n");
                    PyErr_Print();
                    PyErr_Clear();
                    continue;
                }
//                currentActionTuple = pyAction;
                button->setPyAction(pyAction);
                connect(button, SIGNAL(clicked(PyObject*)), this, SLOT(buttonPushed(PyObject*)));
            }
        } else if(itemType.toLower() == "buttonleft") {
            QToolButton *button = new QToolButton(wizardPage);
            button->setText(itemName);
            button->setToolButtonStyle(Qt::ToolButtonTextOnly);
            layout->addWidget(button, i, 0, Qt::AlignCenter);
        } else if(itemType.toLower() == "buttonright") {
            QToolButton *button = new QToolButton(wizardPage);
            button->setText(itemName);
            button->setToolButtonStyle(Qt::ToolButtonTextOnly);
            layout->addWidget(button, i, 1, Qt::AlignCenter);
        }  else if(itemType.toLower() == "fillhook") {
            if(tupleSize < 3) {
                printf("fillHook tuple has a size of less than 3!\n");
                continue;
            }
            QWidget *fillArea = new QWidget(this);
            QGridLayout *fillAreaLayout = new QGridLayout;
            QButtonGroup *radioButtonGroup = new QButtonGroup(this);
            PyObject *pyFillHookArgs = PyTuple_GetItem(pyContentItem, 2);
            if(pyFillHookArgs == Py_None) pyFillHookArgs = NULL;
//            printf("FillHook fires!\n");
            PyObject *pyFillHookList = PyObject_CallMethodObjArgs(pyWizardPageInstance, secondTupleItem, pyFillHookArgs);
            if(!pyFillHookList ||
                    !PyList_Check(pyFillHookList)) {
                printf("FillHook, %s, did not return a list!\n", itemName.toStdString().data());
                continue;
            }
            Py_ssize_t pyFillHookListSize = PyList_Size(pyFillHookList);
            for(int j = 0;j < pyFillHookListSize;j++) {
                PyObject *pyFillString = PyList_GetItem(pyFillHookList, j);
                if(!pyFillString ||
                        !PyString_Check(pyFillString)) {
                    printf("FillHook, %s, returns a list containg objects other than strings!\n", itemName.toStdString().data());
                    continue;
                }
//                printf("FillHook pass #: %d\n", j+1);
                if(tupleSize > 3) {
                    PyObject *pyFillHookType = PyTuple_GetItem(pyContentItem, 3);
                    if(!pyFillHookType ||
                            !PyString_Check(pyFillHookType)) {
                        printf("The fourth item of FillHook tuple, %s, is not a string!\n", itemName.toStdString().data());
                        PyErr_Print();
                        PyErr_Clear();
                        continue;
                    }
                    QString fillHookType = PyString_AsString(pyFillHookType);
                    if(fillHookType.toLower() == "checkbox") {
                        QString fillString(PyString_AsString(pyFillString));
                        QCheckBox *fillCheckbox = new QCheckBox(fillString, wizardPage);
                        wizardPage->publicRegisterField(getMandatoryString(fillString, pyContentItem), fillCheckbox);
                        fillAreaLayout->addWidget(fillCheckbox, j, 1);
                        continue;
                    } else if(fillHookType.toLower() == "radiobutton") {
                        QString fillString(PyString_AsString(pyFillString));
                        QRadioButton *radioButton = new QRadioButton(fillString, wizardPage);
                        radioButtonGroup->addButton(radioButton);
                        if(j == 0) radioButton->setChecked(true);
                        wizardPage->publicRegisterField(fillString, radioButton);
                        fillAreaLayout->addWidget(radioButton, j, 1);
                        continue;
                    } else if(fillHookType.toLower() == "spinbox") {
                        QString fillString(PyString_AsString(pyFillString));
//                        QString fieldName(fillString);
//                        fieldName.append("-spinbox");
                        QSpinBox *spinBox = new QSpinBox(wizardPage);
//                        printf("field name: %s\n", fillString.toStdString().data());
                        wizardPage->publicRegisterField(getMandatoryString(fillString, pyContentItem), spinBox);
                        fillAreaLayout->addWidget(new QLabel(fillString), j, 0);
                        fillAreaLayout->addWidget(spinBox, j, 1);
                        continue;
                    }
                }
                QString fillString(PyString_AsString(pyFillString));
                QLabel *fillStringLabel = new QLabel(fillString, wizardPage);
                QLineEdit *fillField = new QLineEdit(wizardPage);
                wizardPage->publicRegisterField(getMandatoryString(fillString, pyContentItem), fillField);
                fillAreaLayout->addWidget(fillStringLabel, j, 0, Qt::AlignLeft);
                fillAreaLayout->addWidget(fillField, j, 1);
            }
            fillArea->setLayout(fillAreaLayout);
            layout->addWidget(fillArea, i, 0, 1, 2, Qt::AlignCenter);
        }

    }

    wizardPage->setLayout(layout);

    return wizardPage;
}

WizardPage *CharacterCreationWizard::getRollMethodsPage(PyObject *pyWizardPageInstance) {
    WizardPage *wizardPage = new WizardPage(this);
    QPixmap dicePix(":/images/dice.png"), rollBanner(":/images/rollBanner.jpg");
    QLabel *diceLabel;
    PyObject *pyContent, *pyAttributeList, *pyAttribute,
            *pyContentItem, *firstTupleItem, *secondTupleItem;
    Py_ssize_t pyAttributeListSize, pyContentSize, tupleSize;
    int nextRow = 0;
    QGridLayout *layout;
    QComboBox *rollMethodSelector;
    QString pageTitle, pageSubtitle, rollMethod, diceString;
    QList<QString> attributeList;
    Dice dice;

    pyAttributeList = PyObject_GetAttrString(pyWizardPageInstance, (char *) "attribute_list");
    if(!pyAttributeList ||
            !PyList_Check(pyAttributeList)) {
        printf("Error retrieving attribute_list from rollMethodsPage template. Make sure the variable has been added and that it contains a list of strings.\n");
        PyErr_Print();
        PyErr_Clear();
        return wizardPage;
    }
    pyAttributeListSize = PyList_Size(pyAttributeList);
    for(int i = 0;i < pyAttributeListSize;i++) {
        pyAttribute = PyList_GetItem(pyAttributeList, i);
        if(!pyAttribute ||
                !PyString_Check(pyAttribute)) {
            printf("attribute_list item is not a Python string.\n");
            PyErr_Print();
            PyErr_Clear();
            continue;
        }
        attributeList.append(PyString_AsString(pyAttribute));
    }

//    rollBanner
    wizardPage->setPixmap(QWizard::WatermarkPixmap, rollBanner);
    wizardPage->setPixmap(QWizard::BackgroundPixmap, rollBanner);
//    wizardPage->setStyleSheet("QPixmap { padding: 5px; background: yellow }");

    pyContent = PyObject_CallMethod(pyWizardPageInstance, (char *) "get_content", NULL);
    if(!PyList_Check(pyContent)) {
        printf("Content not a Python list!\n");
        return wizardPage;
    }

    pageTitle = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_title", NULL));
    pageSubtitle = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_subtitle", NULL));
    wizardPage->setTitle(pageTitle);
    wizardPage->setSubTitle(pageSubtitle);

    layout = new QGridLayout;

    rollMethodSelector = new QComboBox(wizardPage);
    pyContentSize = PyList_Size(pyContent);
    for(int i = 0;i < pyContentSize;i++) {
        pyContentItem = PyList_GetItem(pyContent, i);
        if(!pyContentItem ||
                !PyTuple_Check(pyContentItem)) {
            printf("content item is not a tuple.\n");
            PyErr_Print();
            PyErr_Clear();
            continue;
        }
        firstTupleItem = PyTuple_GetItem(pyContentItem, 0);
        if(!firstTupleItem ||
                !PyString_Check(firstTupleItem)) {
            printf("Error getting roll method from content item.\n");
            PyErr_Print();
            PyErr_Clear();
            continue;
        }
        rollMethod = PyString_AsString(firstTupleItem);
        diceString = "3d6";
        tupleSize = PyTuple_Size(pyContentItem);
        if(tupleSize > 1) {
            secondTupleItem = PyTuple_GetItem(pyContentItem, 1);
            if(!secondTupleItem ||
                    !PyString_Check(secondTupleItem)) {
                PyErr_Clear();
            } else {
                diceString = PyString_AsString(secondTupleItem);
            }
        }

        rollMethodSelector->addItem(rollMethod);
    }

    diceLabel = new QLabel();
    diceLabel->setPixmap(dicePix);
    layout->addWidget(rollMethodSelector, 0, 0, 1, 3);

    for(int i = 0;i < attributeList.size();i++) {
        QString attrName = attributeList.at(i);
        QLineEdit *attrEdit = new QLineEdit(wizardPage);
        attrEdit->setFixedWidth(30);
        int rollResult = dice.rollDice(diceString);
        printf("diceString: %s | rollResult: %d\n", diceString.toStdString().data(), rollResult);
        attrEdit->setText((QString) rollResult);
        layout->addWidget(new QLabel(attrName), i + 1, 0);
        layout->addWidget(attrEdit, i + 1, 1);
        wizardPage->publicRegisterField(attrName, attrEdit);
        nextRow = i + 2;
    }

    QToolButton *rollButton = new QToolButton(wizardPage);
    rollButton->setText("Roll");
    layout->addWidget(rollButton, nextRow, 1);

    wizardPage->setLayout(layout);

//    wizardPage->initializePage();

    return wizardPage;
}

QString CharacterCreationWizard::getMandatoryString(QString fillString, PyObject *pyContentItem) {
    PyObject *mandatory;
    Py_ssize_t itemSize = PyTuple_Size(pyContentItem);
    QString contentType = PyString_AsString(PyTuple_GetItem(pyContentItem, 0));
    if((itemSize < 5 && (contentType.toLower() == "fillhook" || contentType.toLower() == "choose")) ||
            (itemSize < 3 && contentType.toLower() == "field")) {
        return fillString;
    }

    mandatory = PyTuple_GetItem(pyContentItem, 4);
    if(contentType.toLower() == "field") mandatory = PyTuple_GetItem(pyContentItem, 2);
    if(!mandatory ||
            mandatory != Py_True) {
        return fillString;
    }

    return fillString.append("*");
}

void CharacterCreationWizard::buttonPushed(PyObject *currentActionTuple) {
    if(!currentActionTuple) {
        return;
    }
    QString cAction = PyString_AsString(PyTuple_GetItem(currentActionTuple, 0));
    if(cAction.toLower() == "dice.rolldice" &&
            PyTuple_Size(currentActionTuple) > 1) {
        Dice dice;
        QString rollString = PyString_AsString(PyTuple_GetItem(currentActionTuple, 1));
//        int roll = dice.rollDice(rollString);
        PyObject *fields = PyTuple_GetItem(currentActionTuple, 2);
        if(!fields ||
                !PyList_Check(fields)) {
            printf("Third action tuple item is not a list!\n");
            PyErr_Print();
            PyErr_Clear();
            return;
        }
        Py_ssize_t fieldsSize = PyList_Size(fields);
        for(int i = 0;i < fieldsSize;i++) {
            setField(PyString_AsString(PyList_GetItem(fields, i)), dice.rollDice(rollString));
        }
//        printf("%s roll: %d\n", rollString.toStdString().data(), roll);
    }
}

ComboRow::ComboRow(QWidget *parent, DatabaseHandler *db, PythonInterpreter *interpreter)
    : QComboBox(parent) {
    this->db = db;
    this->interpreter = interpreter;
}

void ComboRow::addRowItem(QList<QVariant> *row, int displayColumn) {
    QString displayString = row->at(displayColumn).toString();
    this->addItem(displayString, *row);
}

void ComboRow::fillComboRow(QString tableName) {
    if(!db->checkPersistentTable(tableName)) {
        printf("Table, %s, missing!\n", tableName.toStdString().data());
        return;
    }

    QList<QList<QVariant> *> rows = db->getRows(tableName);
//    int displayCol = interpreter->getDisplayColWithoutInit(tableName);
    printf("fillComboRow\n");
    int displayCol = interpreter->getDisplayCol(tableName);

    for(int i = 0;i < rows.size();i++) {
        QList<QVariant> *row = rows.at(i);
//        QString displayString = row->at(displayCol).toString();
        this->addRowItem(row, displayCol);
    }
}
