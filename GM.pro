######################################################################
# Automatically generated by qmake (2.01a) Tue Sep 10 02:03:52 2013
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .

INCLUDEPATH += /usr/include/python2.7
LIBS += -lpython2.7
QT += sql

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
    WizardPage.h
SOURCES += main.cpp MainWindow.cpp \
    PythonInterpreter.cpp \
    ListObject.cpp \
    DatabaseHandler.cpp \
    DBWindow.cpp \
    ChooseSystemDialog.cpp \
    Dice.cpp \
    CharacterCreationWizard.cpp \
    PyToolButton.cpp \
    WizardPage.cpp

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    qml.main
