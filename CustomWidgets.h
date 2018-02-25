#ifndef CUSTOMWIDGETS_H
#define CUSTOMWIDGETS_H

#include "PythonInterpreter.h"

#include <QLabel>
#include <QListWidgetItem>
#include <QWidget>
#include <QString>

class ImageWidget : public QLabel {

public:
    ImageWidget( QString base64_data, QWidget *parent = 0 );
    QString getData();
    void setData( QString base64_data );

/*Static Methods*/
public:
    static void setImageWithBase64( QLabel *image_widget, QString base64_string );

private:
    QString data;

};

class PyDataListWidgetItem : public QListWidgetItem {

public:
    PyDataListWidgetItem( QString display_text, PyObject *data, QListWidget *parent = 0, QListWidget *original_list = 0 );
    PyObject *getData();
    QListWidget *getOriginalList();

/*Static Methods*/
public:
    static void fillListWidget( QListWidget *list_widget, PyObject *list_obj, PyObject *tool_tip_callback = NULL, PyObject *fields_obj = NULL );
    static void addItemToWidget( QListWidget *list_widget, PyObject *list_item_obj, PyObject *tool_tip_callback = NULL, PyObject *fields_obj = NULL );
    static PyObject *getDataList( QListWidget *list_widget );

private:
    PyObject *data;
    QListWidget *originalList;

};

class DualListWidget : public QWidget {

public:
    DualListWidget( PyObject *owned_item_list_obj, PyObject *action_data_obj, PyObject *fields_obj, QWidget *parent = 0 );
    QListWidget *getChosenList();

private:
    QTabWidget *tabbedAvailLists;
    QListWidget *chosenList;

};

class GuiWidget {

public:
    GuiWidget( PyObject *widget_obj, QWidget *parent = 0 );
    QString getFieldName();
    int getColSpan();
    int getRowSpan();
    Qt::Alignment getAlignFlag();
    QWidget *getWidget();
    QString getWidgetType();
    QLayout *getWidgetLayout();

private:
    QString fieldName;
    int colSpan;
    int rowSpan;
    Qt::Alignment alignFlag = 0;
    QWidget *widget;
    QString widgetType;
    QLayout *widgetLayout = NULL;
};

class WidgetRegistry {

public:
    void registerWidget( GuiWidget *gui_widget );
    GuiWidget *getGuiWidget( QString field_name );
    QHash<QString, GuiWidget *>::iterator begin();
    QHash<QString, GuiWidget *>::iterator end();
    void processAction( PyObject *action_obj, QWidget *parent = 0 );
    void fillFields( PyObject *fill_dict_obj );
    PyObject *getFieldsDict();

private:
    QHash<QString, GuiWidget *> hash;
};

#endif // CUSTOMWIDGETS_H

