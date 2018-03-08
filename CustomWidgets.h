#ifndef CUSTOMWIDGETS_H
#define CUSTOMWIDGETS_H

#include "PythonInterpreter.h"

#include <QPushButton>
#include <QLabel>
#include <QListWidgetItem>
#include <QWidget>
#include <QString>
#include <QMenuBar>

class WidgetRegistry;

class ImageWidget : public QLabel {

    Q_OBJECT

public:
    ImageWidget( QString base64_data, QWidget *parent = 0 );
    QString getData();
    void setData( QString base64_data );

/*Static Methods*/
public:
    static void setImageWithBase64( QLabel *image_widget, QString base64_string );

private:
    QString data;

signals:
    void imageChanged( QString base64_string );

};

class PyDataListWidgetItem : public QListWidgetItem {

public:
    PyDataListWidgetItem( QString display_text, PyObject *data, QListWidget *parent = 0, QListWidget *original_list = 0 );
    PyObject *getData();
    QListWidget *getOriginalList();

/*Static Methods*/
public:
    static void fillListWidget( QListWidget *list_widget, PyObject *list_obj, PyObject *tool_tip_callback = NULL, PyObject *fields_obj = NULL, PyObject *wizard_pages_obj = NULL, PyObject *wizard_data_obj = NULL, QListWidget *original_list = 0 );
    static void addItemToWidget( QListWidget *list_widget, PyObject *list_item_obj, PyObject *tool_tip_callback = NULL, PyObject *fields_obj = NULL, PyObject *wizard_pages_obj = NULL, PyObject *wizard_data_obj = NULL, QListWidget *original_list = 0 );
    static PyObject *getDataList( QListWidget *list_widget );

private:
    PyObject *data;
    QListWidget *originalList;

};

class DualListData {

public:
    DualListData( PyObject *data_obj );


public:
    PyObject *getFillAvailCallback();
    PyObject *getSlotsCallback();
    PyObject *getSlotsNameObj();
    PyObject *getCategoryFieldObj();
    PyObject *getToolTipCallback();
    PyObject *getAddCallback();
    PyObject *getRemoveCallback();

private:
    PyObject *fillAvailCallback;
    PyObject *slotsCallback;
    PyObject *slotsNameObj;
    PyObject *categoryFieldObj;
    PyObject *toolTipCallback;
    PyObject *addCallback;
    PyObject *removeCallback;
};

class DualListWidget : public QWidget {

public:
    DualListWidget( PyObject *owned_item_list_obj, PyObject *action_data_obj, PyObject *fields_obj, QWidget *parent = 0 );
    DualListWidget(  PyObject *data_obj, PyObject *fields_obj, PyObject *wizard_pages_obj, PyObject *wizard_data_obj, QWidget *parent = 0 );
    void setData( PyObject *data_obj );
    DualListData *getData();
    void fillAvail( PyObject *owned_item_list_obj, PyObject *fields_obj, PyObject *wizard_pages_obj = NULL, PyObject *wizard_data_obj = NULL );
    QListWidget *getChosenList();


private:
    QHash<QString, QListWidget *> categoryHash;
    QTabWidget *tabbedAvailLists;
    QListWidget *chosenList;
    QLabel *slotsLabel;
    QPushButton *addButton;
    QPushButton *delButton;
    DualListData *data;
    PyObject *availItemListObj;

};

class GuiWidget {

public:
    GuiWidget( PyObject *widget_obj, QWidget *parent = 0, bool create_qwidget = true, PyObject *wizard_fields_obj = NULL, PyObject *wizard_pages_obj = NULL, PyObject *wizard_data_obj = NULL );
    QString getFieldName();
    int getColSpan();
    int getRowSpan();
    Qt::Alignment getAlignFlag();
    QWidget *getWidget();
    QString getWidgetType();
    QLayout *getWidgetLayout();
    void setWidgetData( PyObject *data );
    PyObject *getWidgetData();
    void setWizardPages( PyObject *wizard_pages_obj );
    PyObject *getWizardPages();
    void setWizardData( PyObject *wiazard_data_obj );
    PyObject *getWizardData();

private:
    QString fieldName;
    int colSpan;
    int rowSpan;
    Qt::Alignment alignFlag = 0;
    QWidget *widget;
    QString widgetType;
    QLayout *widgetLayout = NULL;
    PyObject *data = Py_None;
    PyObject *wizardPagesObj = NULL;
    PyObject *wizardDataObj = NULL;
};

class GuiAction {

public:
    GuiAction( PyObject *action_obj );
    QString getActionType();
    GuiWidget *getWidget1();
    GuiWidget *getWidget2();
    PyObject *getCallback();
    PyObject *getData();

private:
    QString actionType;
    GuiWidget *widget1;
    GuiWidget *widget2 = NULL;
    PyObject *callback;
    PyObject *data;
};

class WidgetRegistry {

public:
    void registerWidget( GuiWidget *gui_widget );
    GuiWidget *getGuiWidget( QString field_name );
    QHash<QString, GuiWidget *>::iterator begin();
    QHash<QString, GuiWidget *>::iterator end();
    void processAction( PyObject *action_obj, QWidget *parent = 0 );
    void processAction( GuiAction gui_action, QWidget *parent = 0 );
    void fillFields( PyObject *fill_dict_obj );
    PyObject *getFieldsDict();
    void setDefaultActions( GuiAction gui_action, QWidget *parent = 0 );
    void fillMenuBar( QMenuBar *menu_bar, PyObject *menu_list_obj, QWidget *parent = 0 );

private:
    QHash<QString, GuiWidget *> hash;
};

#endif // CUSTOMWIDGETS_H

