#include "MainWindow.h"
#include "CharacterCreationWizard.h"
#include "DBWindow.h"
#include <QToolBar>
#include <QIcon>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QSignalMapper>
#include <QTextEdit>
#include <QDesktopWidget>

#include <iostream>
using namespace std;

MainWindow::MainWindow(QString systemPath, QWidget *parent)
    : QMainWindow(parent)
{
    qRegisterMetaType< QList<PyObject *> >( "QList<PyObject *>" );
    interpreter = new PythonInterpreter(systemPath);
    //interpreter->runModule(new QString("Db"));
    db = new DatabaseHandler(systemPath);
    interpreter->loadSettings(db);

    QPixmap newpix(":/images/new.png");
    QPixmap openpix(":/images/open.png");
    QPixmap quitpix(":/images/quit.png");
    QPixmap icon(":/images/icon.png");

    setWindowIcon(icon);

    QAction *open = new QAction("&Open", this);
    QAction *quit = new QAction("&Quit", this);

    QMenu *file;
    file = menuBar()->addMenu("&File");
    file->addAction(open);
    file->addSeparator();
    file->addAction(quit);

    connect(this, SIGNAL(clickedDB(QString)), this, SLOT(openDBWindow(QString)));
    QSignalMapper *signalMapper = new QSignalMapper(this);
    connect(signalMapper, SIGNAL(mapped(QString)), this, SIGNAL(clickedDB(QString)));
    QMenu *database;
    database = menuBar()->addMenu("&Database");
//    cout << "Before TableNames" << endl;
    QList<QString> tableNames = interpreter->getMenuOrderedTableNames();
    for(int i = 0 ; i < tableNames.size() ; i++)
    {
        QString tableName = tableNames.at(i);
        QAction *dbAction = new QAction(tableName, this);
        signalMapper->setMapping(dbAction, tableName);
        connect(dbAction, SIGNAL(triggered()), signalMapper, SLOT(map()));
        database->addAction(dbAction);
    }
    database->addSeparator();
    QAction *dbReset = new QAction("Reset Databases", this);
    database->addAction(dbReset);

    QToolBar *toolbar = addToolBar("main toolbar");
    toolbar->addAction(QIcon(newpix), "New File");
    QAction *open2 = toolbar->addAction(QIcon(openpix), "Open File");
    toolbar->addSeparator();
    QAction *charCreateWizard = toolbar->addAction("Character Creation Wizard");
    toolbar->addSeparator();
    QAction *quit2 = toolbar->addAction(QIcon(quitpix),
                                        "Quit Application");

    QTextEdit *edit = new QTextEdit(this);

    setCentralWidget(edit);

    statusBar()->showMessage("Ready");

    connect(open, SIGNAL(triggered()), this, SLOT(clickOpen()));
    connect(open2, SIGNAL(triggered()), this, SLOT(clickOpen()));
    connect(dbReset, SIGNAL(triggered()), this, SLOT(resetDB()));
    connect(quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(quit2, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(charCreateWizard, SIGNAL(triggered()), this, SLOT(openCharacterCreationWizard()));

//    QString systemName = db->getInMemoryValue("Settings", "systemName");
//    QString subSysName = db->getInMemoryValue("Settings", "subSystemName");
//    QString attributes = db->getInMemoryValue("Settings", "attributes");
//    QString alignments = db->getInMemoryValue("Settings", "alignments");
//    cout << "systemName = " << systemName.toStdString() << endl;
//    cout << "subSystemName = " << subSysName.toStdString() << endl;
//    cout << "attributes = " << attributes.toStdString() << endl;
//    cout << "alignments = " << alignments.toStdString() << endl;
}

void MainWindow::center()
{
  int x, y;
  int screenWidth;
  int screenHeight;

  int WIDTH = this->width();
  int HEIGHT = this->height();
  

  QDesktopWidget *desktop = QApplication::desktop();

  screenWidth = desktop->width();
  screenHeight = desktop->height();
 
  x = (screenWidth - WIDTH) / 2;
  y = (screenHeight - HEIGHT) / 2;

  this->setGeometry(x, y, WIDTH, HEIGHT);
  //this->setFixedSize(WIDTH, HEIGHT);
}

void MainWindow::clickOpen()
{
    /*interpreter->importTables(db);
    DBWindow *testWindow = new DBWindow(this, db, "items");

    testWindow->show();*/
}

void MainWindow::resetDB()
{
    QMessageBox confirmAction;
    confirmAction.setWindowIcon(this->windowIcon());
    confirmAction.setText("Are you sure you want to reset the databases to the defaults?");
    confirmAction.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmAction.setDefaultButton(QMessageBox::No);
    int answer = confirmAction.exec();
    if(answer == QMessageBox::Yes)
    {
        interpreter->importTables(db);
    }
}

void MainWindow::openDBWindow(QString tableName)
{
    DBWindow *dbWindow = new DBWindow(this, db, interpreter, tableName);
    QSize size;
    QRect screenRec = QApplication::desktop()->screenGeometry();
    int height = screenRec.height() / 2;
    int width = height * 1.75;
    size.setHeight(height);
    size.setWidth(width);
    //size.setHeight(dbWindow->height());
    //size.setWidth(850);
    dbWindow->resize(size);
    dbWindow->move((screenRec.width() - width) / 2, (screenRec.height() - height) / 2);
    dbWindow->show();
}

void MainWindow::openCharacterCreationWizard() {
//    printf("You clicked the Character Creation Wizard button.\n");
    CharacterCreationWizard ccWizard(this, this->db, this->interpreter);
    ccWizard.exec();
}
