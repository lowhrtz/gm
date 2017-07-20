#include "DBWindow.h"
//#include <QByteArray>
#include <QGridLayout>
#include <QGroupBox>
#include <QDir>
#include <QMainWindow>
#include <QLineEdit>
//#include <QObjectList>
#include <QSignalMapper>
#include <QTextEdit>
#include <QScrollArea>

#include <iostream>
using namespace std;

DBWindow::DBWindow(QWidget *parent, DatabaseHandler *db, PythonInterpreter *interpreter, QString tableName)
    : QMainWindow(parent)
{
    this->interpreter = interpreter;
    this->db = db;
    this->tableName = tableName;
    this->uniqueCol = 0;
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
        QString labelString = QString(colList.at(i));
        labelString.replace("_", " ");
        QLabel *colLabel = new QLabel(labelString, fieldWidget);
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
    QString metaTableName = interpreter->getMetaTableName(tableName);
    if(!metaTableName.isEmpty()) {
        MetaDBButton *metaTableButton = new MetaDBButton(metaTableName, fieldWidget);
        fieldLayout->addWidget(metaTableButton);
        connect(metaTableButton, SIGNAL(clicked(QString, QString)), this, SLOT(openMetaDBWindow(QString, QString)));
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

void DBWindow::selectRecord( QListWidgetItem *recordWidgetItem, QListWidgetItem *previousRecord ) {
    if( previousRecord == recordWidgetItem ) return;
    QList< QVariant > *row = ( (DBListWidgetItem*) recordWidgetItem )->getRow();
    QList< QString > colDefsList = interpreter->getColDefsList( tableName );
    for( int i = 0 ; i < row->size() ; i++ )     {
        if( colDefsList.at( i ).contains( "UNIQUE", Qt::CaseInsensitive ) ) uniqueCol = i;
        if( colDefsList.at( i ).compare( "TEXT" ) == 0 ) {
            QTextEdit *entryBox = (QTextEdit *) entryList.at( i );
            entryBox->setText( row->at( i ).toString() );
        } else {
            QLineEdit *entryBox = (QLineEdit *) entryList.at( i );
            entryBox->setText( row->at( i ).toString() );
            entryBox->setCursorPosition( 0 );
        }
    }
    std::pair<int, int> base_64_and_type = interpreter->getBase64ImageAndTypeCols( tableName );
    int base_64_col = base_64_and_type.first;
    int image_type_col = base_64_and_type.second;
    if ( base_64_col < 0 || image_type_col < 0 ) {
        iconLabel->setPixmap( getPortrait( row->at( uniqueCol ).toString() ) );
    } else {
        QString base_64_string = row->at( base_64_col ).toString();
        QString image_type = row->at( image_type_col ).toString();
        QByteArray ba = QByteArray::fromBase64( base_64_string.toStdString().data() );
        QImage portrait_image = QImage::fromData( ba, image_type.toStdString().data() );
        QPixmap portrait_pixmap = QPixmap( QPixmap::fromImage( portrait_image ) );
        if( portrait_pixmap.height() > 200 ) {
            portrait_pixmap = portrait_pixmap.scaledToHeight( 200 );
        }
        iconLabel->setPixmap( portrait_pixmap );
    }
}

void DBWindow::openMetaDBWindow(QString metaTableName, QString record_id) {
    /*cout << "Table: " << metaTableName.toStdString().data() << endl;
    cout << "Record ID: " << record_id.toStdString().data() << endl;*/
    MetaDBWindow *metaDBWindow = new MetaDBWindow(this, this->db, this->interpreter, metaTableName, record_id);
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


MetaDBButton::MetaDBButton(QString metaTableName, QWidget *parent)
 : QPushButton("More", parent) {
    this->metaTableName = metaTableName;
    //this->parent = parent;
    connect(this, SIGNAL(clicked()), this, SLOT(reemitClicked()));
}

void MetaDBButton::reemitClicked()
{
    //QLineEdit *unique_id = (QLineEdit *) this->parent()->children().at(1);
    QLineEdit *uniqueID;
    //QString unique_text;

    DBWindow *dbWin = (DBWindow *) this->parent()->parent()->parent()->parent()->parent();
    int uniqueChildIndex = (dbWin->uniqueCol * 2) + 1;
    //cout << dbWin->uniqueCol << " : " << uniqueChildIndex << endl;
    uniqueID = (QLineEdit *) this->parent()->children().at(uniqueChildIndex);
    /*QObjectList oList = this->parent()->children();
    for(int i = 0;i < this->parent()->children().size();i++) {
        QLineEdit *obj = (QLineEdit *) oList.at(i);
        if(!obj->isEnabled()) {
            unique_id = obj;
            unique_text = unique_id->text();
            break;
        }
    }*/
    emit clicked(metaTableName, uniqueID->text());
}


MetaDBWindow::MetaDBWindow(QWidget *parent, DatabaseHandler *db, PythonInterpreter *interpreter, QString tableName, QString recordID)
 : QMainWindow(parent){
    this->db = db;
    this->interpreter = interpreter;
    this->tableName = tableName;
    this->recordID = recordID;

    QList<QString> colDefsList = interpreter->getColDefsList(tableName);
    for(int i = 0;i < colDefsList.size();i++) {
        QString colDef = colDefsList.at(i);
        if(colDef.contains("REFERENCES", Qt::CaseInsensitive)) {
            this->referenceCol = i;
            break;
        }
    }
    QString referenceColName = interpreter->getColList(tableName).at(referenceCol);
    //cout << "Reference Column Name: " << referenceColName.toStdString().data() << endl;
    QList<QList<QVariant> *> metaRows = db->getRows(tableName, referenceColName, recordID);
    for(int i = 0;i < metaRows.size();i++) {
        QList<QVariant> *metaRow = metaRows.at(i);
        for(int j = 0;j < metaRow->size();j++) {
            cout << metaRow->at(j).toString().toStdString().data() << ", ";
        }
        cout << endl;
    }
}
