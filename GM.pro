######################################################################
# Automatically generated by qmake (2.01a) Tue Sep 10 02:03:52 2013
######################################################################

TEMPLATE = app
TARGET = GM
DEPENDPATH += .
INCLUDEPATH += .

win32 {
    INCLUDEPATH += /Python27/include
    LIBS += -L/Python27/libs -lpython27
} else {
    INCLUDEPATH += /usr/include/python2.7
    LIBS += -lpython2.7
}
QT += sql
QT += printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): CONFIG += c++11

# Input
HEADERS += MainWindow.h \
    PythonInterpreter.h \
    ListObject.h \
    DatabaseHandler.h \
    DBWindow.h \
    ChooseSystemDialog.h \
    Dice.h \
    CharacterCreationWizard.h \
    PyToolButton.h \
    WizardPage.h \
    Dialogs.h \
    PDFCreator.h
SOURCES += main.cpp MainWindow.cpp \
    PythonInterpreter.cpp \
    ListObject.cpp \
    DatabaseHandler.cpp \
    DBWindow.cpp \
    ChooseSystemDialog.cpp \
    Dice.cpp \
    CharacterCreationWizard.cpp \
    PyToolButton.cpp \
    WizardPage.cpp \
    Dialogs.cpp \
    PDFCreator.cpp

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    qml.main
