#include "DBWindow.h"
#include <QGridLayout>
#include <QGroupBox>
#include <QDir>
#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalMapper>
#include <QTextEdit>
#include <QScrollArea>

#include <iostream>
using namespace std;

DBWindow::DBWindow(QWidget *parent, DatabaseHandler *db, PythonInterpreter *interpreter, QString tableName)
    : QMainWindow(parent)
{
    this->interpreter = interpreter;
    this->tableName = tableName;
//    setWindowFlags(Qt::SubWindow);

    QGroupBox *gridGroupBox = new QGroupBox(/*tr(tableName.toAscii()).append(" Database")*/);
    QGridLayout *layout = new QGridLayout;
    QListWidget *recordList = new QListWidget(this);
    QGridLayout *fieldLayout = new QGridLayout;
    QWidget *fieldWidget = new QWidget(this);
    QScrollArea *fieldScroll = new QScrollArea;
    fieldScroll->setBackgroundRole(QPalette::AlternateBase);
//    QLabel *testLabel = new QLabel("Name: ", this);
    layout->addWidget(recordList, 1, 0);
//    QLineEdit *nameField = new QLineEdit(this);
    layout->addWidget(fieldScroll, 0, 1, 2, 1);
//    QPixmap icon = QPixmap(":/images/noImage.jpg");
    QPixmap icon = getPortrait("");
    iconLabel = new QLabel;
    iconLabel->setPixmap(icon);
//    iconLabel->setStyleSheet("border-style: outset; border-width: 2px;");
    iconLabel->setStyleSheet("border: 3px inset #999999;");
//    QPushButton *testButton = new QPushButton("Test");
//    testButton->setStyleSheet("background-color: #FFFFFF; border-style: outset; border-width: 2px;");
//    layout->addWidget(iconLabel, 0, 0, 1, 1, Qt::AlignCenter);
    layout->addWidget(iconLabel, 0, 0, 1, 1, Qt::AlignCenter);
    gridGroupBox->setLayout(layout);
    setCentralWidget(gridGroupBox);

    QString title(tableName);
    title.append(" Database");
    setWindowTitle(tr(title.toUtf8()));
    setWindowModality(Qt::WindowModal);

    QList<QString> colList = interpreter->getColList(tableName);
    QList<QString> colDefsList = interpreter->getColDefsList(tableName);
    int displayCol = interpreter->getDisplayCol(tableName);
//    cout << "colDefsList size: " << colDefsList.size() << endl;
    for(int i = 0 ; i < colDefsList.size() ; i++)
    {
        QLabel *colLabel = new QLabel(colList.at(i).toStdString().data(), fieldWidget);
        QString colDefString = colDefsList.at(i);
        fieldLayout->addWidget(colLabel, i, 0);
        if(colDefString.compare("TEXT") == 0) {
            colLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            QTextEdit *fieldEntry = new QTextEdit(fieldWidget);
            fieldLayout->addWidget(fieldEntry, i, 1);
            entryList.append(fieldEntry);
        } else {
            QLineEdit *fieldEntry = new QLineEdit(fieldWidget);
            if(colDefString.contains("UNIQUE", Qt::CaseInsensitive)) fieldEntry->setEnabled(false);
            fieldLayout->addWidget(fieldEntry, i, 1);
            fieldEntry->setFixedWidth(250);
            entryList.append(fieldEntry);
        }
//        fieldEntry->setMaximumWidth(500);
//        cout << "Column Def: " << colDefsList.at(i).toStdString() << endl;
    }
    fieldWidget->setLayout(fieldLayout);
    fieldScroll->setWidget(fieldWidget);

//    connect(recordList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectRecord(QListWidgetItem*)));
//    connect(recordList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(selectRecord(QListWidgetItem*)));
//    connect(recordList, SIGNAL(itemPressed(QListWidgetItem*)), this, SLOT(selectRecord(QListWidgetItem*)));
    connect(recordList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(selectRecord(QListWidgetItem*, QListWidgetItem*)));
    QList<QList<QVariant> *> rows = db->getRows(tableName);
    for(int i = 0 ; i < rows.size() ; i++)
    {
        QList<QVariant> *row = rows.at(i);
//        QString ident = row->at(0).toString();
        QListWidgetItem *recordItem = new DBListWidgetItem(recordList, row);
        recordItem->setData(Qt::DisplayRole, row->at(displayCol));
        recordList->addItem(recordItem);
    }
}

QPixmap DBWindow::getPortrait(QString name)
{
    QString systemPath = interpreter->getSystemPath();
    QString portraitPathString(systemPath);
    portraitPathString.append("/portraits/").append(tableName);
//    cout << "Portrait Path String: " << portraitPathString.toStdString() << endl;
    QDir portraitPath(portraitPathString);
    portraitPath.setFilter(QDir::Files);
    QStringList nameFilters;
    QString jpgName(name);
    jpgName.append(".jpg");
    QString jpegName(name);
    jpegName.append(".jpeg");
    QString pngName(name);
    pngName.append(".png");
    QString gifName(name);
    gifName.append(".gif");
    nameFilters << jpgName << jpegName << pngName << gifName;
    QStringList fileList = portraitPath.entryList(nameFilters);
    if(fileList.size() < 1)
    {
//        cout << "No Portraits Found! " << fileList.size() << endl;
        QString defaultPortraitPathString(systemPath);
        defaultPortraitPathString.append("/portraits/");
        QDir defaultPortraitPath(defaultPortraitPathString);
        QStringList defaultFilters;
        defaultFilters << "default.jpg" << "default.jpeg" << "default.png" << "default.gif";
        QStringList defaultList = defaultPortraitPath.entryList(defaultFilters);
        if(defaultList.size() < 1)
        {
            return QPixmap(":/images/noImage.jpg");
        }
        QString defaultFilePathString(defaultPortraitPathString);
        defaultFilePathString.append(defaultList.at(0));
//        cout << "Default Path: " << defaultFilePathString.toStdString() << endl;
        return QPixmap(defaultFilePathString);
    }

    QString filePathString(portraitPathString);
    filePathString.append("/").append(fileList.at(0));
    QPixmap imagePixmap = QPixmap(filePathString);
    if(imagePixmap.height() > 200) {
        imagePixmap = imagePixmap.scaledToHeight(200);
    }
    return imagePixmap;
}

void DBWindow::selectRecord(QListWidgetItem *recordWidgetItem, QListWidgetItem *previousRecord)
{
    if(previousRecord==recordWidgetItem) return;
    QList<QVariant> *row = ((DBListWidgetItem*)recordWidgetItem)->getRow();
//    cout << "Item: " << row->at(0).toString().toStdString() << endl;
//    cout << "Len Item: " << row->size() << " Len entryList: " << entryList.size() << endl;
    int uniqueCol = 0;
    QList<QString> colDefsList = interpreter->getColDefsList(tableName);
    for(int i = 0 ; i < row->size() ; i++)
    {
        if(colDefsList.at(i).contains("UNIQUE", Qt::CaseInsensitive)) uniqueCol = i;
        if(colDefsList.at(i).compare("TEXT") == 0) {
            QTextEdit *entryBox = (QTextEdit *) entryList.at(i);
            entryBox->setText(row->at(i).toString());
            //entryBox->setCursorPosition(0);
        } else {
            QLineEdit *entryBox = (QLineEdit *) entryList.at(i);
            entryBox->setText(row->at(i).toString());
            entryBox->setCursorPosition(0);
        }
    }
//    cout << row->at(0).toString().toStdString() << endl;
    iconLabel->setPixmap(getPortrait(row->at(uniqueCol).toString()));
}

DBListWidgetItem::DBListWidgetItem(QListWidget *parent, QList<QVariant> *row)
    : QListWidgetItem(parent, QListWidgetItem::UserType)
{
    this->row = row;
}

QList<QVariant> *DBListWidgetItem::getRow()
{
    return this->row;
}
