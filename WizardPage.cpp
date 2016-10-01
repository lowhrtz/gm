#include "WizardPage.h"
#include "Dice.h"
#include <QDrag>
#include <QDragMoveEvent>
#include <QGridLayout>
#include <QLineEdit>
#include <QMimeData>
#include <QToolButton>
#include <QIntValidator>

#include <QList>

WizardPage::WizardPage(PyObject *pyWizardPageInstance, QWidget *parent) :
    QWizardPage(parent) {
    this->pyWizardPageInstance = pyWizardPageInstance;
    this->wizard = (QWizard *) parent;
}

void WizardPage::publicRegisterField(const QString &name, QWidget *widget, const char *property, const char *changedSignal) {
    this->registerField(name, widget, property, changedSignal);
}

void WizardPage::cleanupPage() {
}

void WizardPage::initializePage() {
}

int WizardPage::nextId() const {
//    char *format;
//    char *args;
    int return_value;
//    qInfo("Type: %s", nextIdArgs.typeName());
    if (nextIdArgs.isNull()) {
        return_value = PyInt_AsSsize_t(PyObject_CallMethod(this->pyWizardPageInstance, (char *) "get_next_page_id", NULL));
    }
    else if (strcmp(nextIdArgs.typeName(), "int") == 0) {
        return_value = PyInt_AsSsize_t(PyObject_CallMethod(this->pyWizardPageInstance, (char *) "get_next_page_id", (char *) "i", nextIdArgs.toInt()));
    }
    else if (strcmp(nextIdArgs.typeName(), "bool") == 0) {
        int argInt = 0;
        if (nextIdArgs.toBool()) {
            argInt = 1;
        }
        return_value = PyInt_AsSsize_t(PyObject_CallMethod(this->pyWizardPageInstance, (char *) "get_next_page_id", (char *) "i", argInt));
    }
    else {
        return_value = PyInt_AsSsize_t(PyObject_CallMethod(this->pyWizardPageInstance, (char *) "get_next_page_id", NULL));

    }

    if (return_value == -2) {
        bool foundCurrentPage = false;
        foreach (int id, wizard->pageIds()) {
            if (id == this->pageId) {
                foundCurrentPage = true;
            } else if (foundCurrentPage) {
                return id;
            }
        }
        return -1;
    }

    return return_value;
}

RollMethodsPage::RollMethodsPage(PyObject *pyWizardPageInstance, QWidget *parent) :
    WizardPage(pyWizardPageInstance, parent) {

    QPixmap dicePix(":/images/dice.png"), rollBanner(":/images/rollBanner.jpg");
    QLabel *diceLabel;
    PyObject *pyContent, *pyAttributeList, *pyAttribute,
            *pyContentItem, *firstTupleItem, *secondTupleItem, *thirdTupleItem;
    Py_ssize_t pyAttributeListSize, pyContentSize, tupleSize;
    int nextRow = 0;
    QGridLayout *layout;
    QString pageTitle, pageSubtitle, rollMethod, rollMethodDisplayString, diceString;

    setStyleSheet("QLineEdit { background: #ffffff; color: #000000; }");

    pyAttributeList = PyObject_GetAttrString(pyWizardPageInstance, (char *) "attribute_list");
    if(!pyAttributeList ||
            !PyList_Check(pyAttributeList)) {
        qInfo("Error retrieving attribute_list from rollMethodsPage template. Make sure the variable has been added and that it contains a list of strings.\n");
        PyErr_Print();
        PyErr_Clear();
        return;
    }
    pyAttributeListSize = PyList_Size(pyAttributeList);
    for(int i = 0;i < pyAttributeListSize;i++) {
        pyAttribute = PyList_GetItem(pyAttributeList, i);
        if(!pyAttribute ||
                !PyString_Check(pyAttribute)) {
            qInfo("attribute_list item is not a Python string.\n");
            PyErr_Print();
            PyErr_Clear();
            continue;
        }
        attributeList.append(PyString_AsString(pyAttribute));
    }

//    rollBanner
    setPixmap(QWizard::WatermarkPixmap, rollBanner);
    setPixmap(QWizard::BackgroundPixmap, rollBanner);
//    wizardPage->setStyleSheet("QPixmap { padding: 5px; background: yellow }");

    pyContent = PyObject_CallMethod(pyWizardPageInstance, (char *) "get_content", NULL);
    if(!PyList_Check(pyContent)) {
        qInfo("get_content doesn't return a Python list!\n");
        return;
    }

    pageTitle = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_title", NULL));
    pageSubtitle = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_subtitle", NULL));
    pageId = PyInt_AsSsize_t(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_id", NULL));
    setTitle(pageTitle);
    setSubTitle(pageSubtitle);

    layout = new QGridLayout;

    QLineEdit *pool = new QLineEdit(this);
    pool->setFixedWidth(35);
    pool->setEnabled(false);

    rollMethodSelector = new QComboBox(this);
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
        tupleSize = PyTuple_Size(pyContentItem);
        firstTupleItem = PyTuple_GetItem(pyContentItem, 0);
        if(tupleSize > 1) {
            secondTupleItem = PyTuple_GetItem(pyContentItem, 1);
        } else {
            secondTupleItem = firstTupleItem;
        }

        if(!firstTupleItem ||
                !PyString_Check(firstTupleItem) ||
                !PyString_Check(secondTupleItem)) {
            qInfo("Error getting roll method from content item.\n");
            PyErr_Print();
            PyErr_Clear();
            continue;
        }

        rollMethod = PyString_AsString(firstTupleItem);
        rollMethodDisplayString = PyString_AsString(secondTupleItem);
        diceString = "3d6";
        if(tupleSize > 2) {
            thirdTupleItem = PyTuple_GetItem(pyContentItem, 2);
            if(!thirdTupleItem ||
                    !PyString_Check(thirdTupleItem)) {
                PyErr_Clear();
            } else {
                diceString = PyString_AsString(thirdTupleItem);
            }

//            if(rollMethod.startsWith("pool")) {
//                QString pool_total_string = PyString_AsString(thirdTupleItem);
//                pool->setText(pool_total_string);
//                pool_total = pool_total_string.toInt();
//            }
        }

        rollMethodSelector->addItem(rollMethodDisplayString, diceString);
        rollMethodSelector->setItemData(i, rollMethod, RollMethodRole);
    }

    diceLabel = new QLabel();
    diceLabel->setPixmap(dicePix);
    if (pyContentSize > 1) {
        layout->addWidget(rollMethodSelector, 0, 0, 1, 5);
    } else {
        rollMethodSelector->hide();
    }

    QList<QLineEdit *> *attrEditList = new QList<QLineEdit *>;
    for(int i = 0;i < attributeList.size();i++) {
        QString attrName = attributeList.at(i);
        QLineEdit *attrEdit = new QLineEdit(this);
        DragLabel *diceDragLabel = new DragLabel(this);
        diceDragLabel->setEnabled(false);
        diceDragLabel->setPixmap(QPixmap(":/images/diceSmall.png"));
        diceDragLabel->setAttrFieldName(attrName);
        attrEdit->setFixedWidth(35);
        attrEdit->setEnabled(false);
        attrEdit->setValidator(new QIntValidator(this));
        layout->addWidget(new QLabel(attrName), i + 1, 1);
        layout->addWidget(attrEdit, i + 1, 2);
        layout->addWidget(diceDragLabel, i + 1, 3);
        publicRegisterField(attrName + "*", attrEdit);
        diceLabelList.append(diceDragLabel);
        nextRow = i + 2;
        attrEditList->append(attrEdit);

//        connect(attrEdit, &QLineEdit::cursorPositionChanged, [=](int old_pos, int new_pos) {
//            old_pos += 1;
//            int place = attrEdit->text().length();
//            if(new_pos != place) {
//                attrEdit->setCursorPosition(place);
//            }
//        });

        connect(attrEdit, &QLineEdit::textEdited, [=](QString newText) {
            newText.toInt();
            int pool_int = pool->text().toInt();
            int score_tally = 0;
            foreach(QLineEdit *atrEdt, *attrEditList) {
                int attr_score = atrEdt->text().toInt();
                score_tally += attr_score;
            }

            int pool_total = rollMethodSelector->currentData().toInt();
            pool_int = pool_total - score_tally;
            if(pool_int < 0) {
                if(attrEdit->isUndoAvailable()) {
                    attrEdit->undo();
                } else {
                    attrEdit->clear();
                }
            } else {
                pool->setText(QString::number(pool_int));
            }

            if(attrEdit->text().toInt() == 0) {
                attrEdit->clear();
            }
        });
    }

    QToolButton *rollButton = new QToolButton(this);
    QLabel *poolLabel = new QLabel("Points Left");
    connect(rollButton, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    rollButton->setText("Roll");
    layout->addWidget(rollButton, nextRow, 2);
    layout->addWidget(pool, nextRow, 2);
    layout->addWidget(poolLabel, nextRow, 3);

//    connect(rollMethodSelector, SIGNAL(currentIndexChanged(QString)), this, SLOT(rollMethodChanged(QString)));
    // http://stackoverflow.com/questions/31164574/qt5-signal-slot-syntax-w-overloaded-signal-lambda
    connect(rollMethodSelector, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int newIndex) {
        rollMethodSelector->itemText(newIndex);
        QString rollMethodString = rollMethodSelector->currentData(RollMethodRole).toString();
        bool enabled = false;
        if(rollMethodString.toLower().endsWith("arrange")) {
            enabled = true;
        }
        for(int i = 0;i < diceLabelList.size();i++) {
            DragLabel *diceLabel = diceLabelList.at(i);
            diceLabel->setAcceptDrops(enabled);
            diceLabel->setEnabled(enabled);
        }

        if(rollMethodString.toLower().startsWith("pool")) {
            rollButton->hide();
            pool->show();
            poolLabel->show();
            foreach(QLineEdit *attrEdit, *attrEditList) {
                attrEdit->setEnabled(true);
            }
            pool->setText(rollMethodSelector->currentData().toString());
            QLineEdit *first_field = attrEditList->at(0);
            emit first_field->textEdited("1");

        } else {
            pool->hide();
            poolLabel->hide();
            rollButton->show();
            foreach(QLineEdit *attrEdit, *attrEditList) {
                attrEdit->setEnabled(false);
            }
        }
    });

    this->setLayout(layout);

    return;

}

void RollMethodsPage::initializePage() {
    emit rollMethodSelector->currentIndexChanged(0);
//    rollMethodSelector->setCurrentIndex(-1);
//    rollMethodSelector->setCurrentIndex(0);
//    rollMethodChanged(rollMethodSelector->currentText());
}

void RollMethodsPage::fillAttributeFields() {
    Dice dice;
//    QString diceString = rollMethodSelector->itemData(rollMethodSelector->currentIndex()).toString();
    QString diceString = rollMethodSelector->currentData().toString();
//    QString rollMethodString = rollMethodSelector->currentText();
    QString rollMethodString = rollMethodSelector->currentData(RollMethodRole).toString();
    for(int i = 0;i < attributeList.size();i++) {
        QString attrName = attributeList.at(i);
        int rollResult;
        if(rollMethodString.toLower().startsWith("droplow")) {
            rollResult = dice.rollDiceDropLowest(diceString);
        } else {
            rollResult = dice.rollDice(diceString);
        }
//        printf("diceString: %s | rollResult: %d\n", diceString.toStdString().data(), rollResult);
        setField(attrName, QVariant(rollResult));
    }
}

void RollMethodsPage::publicSetField(const QString &fieldName, const QVariant &fieldValue) {
    setField(fieldName, fieldValue);
}

QVariant RollMethodsPage::getField(QString fieldName) {
    return field(fieldName);
}

//void RollMethodsPage::rollMethodChanged(QString rollMethodDisplayString) {
//    QString rollMethodString = rollMethodSelector->currentData(RollMethodRole).toString();
//    bool enabled = false;
//    if(rollMethodString.endsWith("arrange")) {
//        enabled = true;
//    }
//    for(int i = 0;i < diceLabelList.size();i++) {
//        DragLabel *diceLabel = diceLabelList.at(i);
//        diceLabel->setAcceptDrops(enabled);
//        diceLabel->setEnabled(enabled);
//    }
//}

void RollMethodsPage::buttonClicked() {
    fillAttributeFields();
}

DragLabel::DragLabel(QWidget *parent) :
    QLabel(parent) {

}

void DragLabel::setAttrFieldName(QString attrFieldName) {
    this->attrFieldName = attrFieldName;
}

QString DragLabel::getAttrFieldName() {
    return attrFieldName;
}

void DragLabel::dropEvent(QDropEvent *dropEvent) {
    RollMethodsPage *rollMethodsPage = (RollMethodsPage *) this->parent();
    QString sourceFieldName = dropEvent->mimeData()->text();
    QString sourceFieldValue = dropEvent->mimeData()->property("field value").toString();
//    printf("Source Field Name: %s\n", sourceFieldName.toLatin1().data());
//    printf("Source Field Value: %s\n", sourceFieldValue.toStdString().data());
    QString destFieldName = this->getAttrFieldName();
    QString destFieldValue = rollMethodsPage->getField(destFieldName).toString();
//    printf("Destination Field Name: %s\n", destFieldName.toLatin1().data());
//    printf("Destination Field Value: %s\n", destFieldValue.toStdString().data());
    rollMethodsPage->publicSetField(destFieldName, QVariant(sourceFieldValue));
    rollMethodsPage->publicSetField(sourceFieldName, QVariant(destFieldValue));
}

void DragLabel::dragMoveEvent(QDragMoveEvent *dragEvent) {
    dragEvent->accept();
}

void DragLabel::dragEnterEvent(QDragEnterEvent *dragEvent) {
    dragEvent->acceptProposedAction();
}

void DragLabel::mousePressEvent(QMouseEvent *ev) {
    if(ev->button() != Qt::LeftButton) qDebug("You didn't click the left button!");
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    RollMethodsPage *rollMethodsPage = (RollMethodsPage *) this->parent();
    QString fieldName = getAttrFieldName();
    QString fieldValue = rollMethodsPage->getField(fieldName).toString();
    if(fieldValue.isNull() || fieldValue.isEmpty()) return;
//    printf("attrfieldName: %s\n", fieldName.toStdString().data());
//    printf("Field Value: %s\n", fieldValue.toStdString().data());
    mimeData->setText(fieldName);
    mimeData->setProperty("field value", QVariant(fieldValue));
    drag->setMimeData(mimeData);
    drag->setPixmap(*this->pixmap());
    drag->start();
}
