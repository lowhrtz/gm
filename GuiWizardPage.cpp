#include "GuiWizard.h"
#include "GuiWizardPage.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QTextEdit>

GuiWizardPage::GuiWizardPage(PyObject *wizard_page_obj, PyObject *wizard_page_dict_obj, PyObject *external_data, WidgetRegistry *widget_registry, QWidget *parent )
    : QWizardPage( parent ){

    PyObject *gui_defs_mod = PyImport_ImportModule( "GuiDefs" );
    PyObject *wizard_page_type = PyObject_GetAttrString( gui_defs_mod, (char *) "WizardPage" );
    if ( !wizard_page_type ) {
        PyErr_Print();
        qInfo( "WizardPage class is missing!" );
        return;
    }
    bool is_wizard_page = PyObject_IsInstance( wizard_page_obj, wizard_page_type );
    if ( !is_wizard_page ) {
        qInfo( "An object that is not a WizardPage has been added to the wizard list!" );
        return;
    }

    this->wizardPageObj = wizard_page_obj;
    this->wizardPageDictObj = wizard_page_dict_obj;
    this->externalData = external_data;
    this->widgetRegistry = widget_registry;

    PyObject *page_id_obj = PyObject_CallMethod( wizard_page_obj, (char *) "get_page_id", NULL );
    PyErr_Print();
    PyObject *title_obj = PyObject_CallMethod( wizard_page_obj, (char *) "get_title", NULL );
    PyErr_Print();
    PyObject *subtitle_obj = PyObject_CallMethod( wizard_page_obj, (char *) "get_subtitle", NULL );
    PyErr_Print();
    PyObject *widget_matrix_obj = PyObject_CallMethod( wizard_page_obj, (char *) "get_widget_matrix", NULL );
    PyErr_Print();

    this->pageId = PyInt_AsSsize_t( page_id_obj );
    setTitle( PyString_AsString( title_obj ) );
    setSubTitle( PyString_AsString( subtitle_obj ) );

    if ( !PyList_Check( widget_matrix_obj ) ) {
        qInfo( "WizardPage did not return a list for the widget_matrix!?!" );
        return;
    }

    QGridLayout *grid_layout = new QGridLayout;
    Py_ssize_t number_of_rows = PyList_Size( widget_matrix_obj );
    for ( int i = 0; i < number_of_rows; i++ ) {
        PyObject *matrix_row = PyList_GetItem( widget_matrix_obj, i );
        Py_ssize_t number_of_cols = PyList_Size( matrix_row );
        for( Py_ssize_t j = 0 ; j < number_of_cols ; j++ ) {
            PyObject *widget_obj = PyList_GetItem( matrix_row, j );
            GuiWidget *gui_widget = new GuiWidget( widget_obj, this, true, widgetRegistry->getFieldsDict(), wizard_page_dict_obj, external_data );
//            gui_widget->setWizardPages( wizardPageDictObj );
//            gui_widget->setWizardData( externalData );
            connectGuiWidgetToCompleteChanged( gui_widget );
            widgetRegistry->registerWidget( gui_widget );
            if ( gui_widget->getWidgetLayout() != NULL ) {
                grid_layout->addLayout( gui_widget->getWidgetLayout(), i, j, gui_widget->getRowSpan(), gui_widget->getColSpan(), gui_widget->getAlignFlag() );
            }
        }
    }

    setLayout( grid_layout );

}

int GuiWizardPage::getPageId() {
    return pageId;
}

void GuiWizardPage::initializePage() {
    PyObject *return_value_obj = PyObject_CallMethodObjArgs( wizardPageObj,
                                                             PyString_FromString( (char *) "initialize_page" ),
                                                             widgetRegistry->getFieldsDict(), wizardPageDictObj, externalData, NULL );
    PyErr_Print();

    if ( PyDict_Check( return_value_obj ) ) {
        widgetRegistry->fillFields( return_value_obj );
    }
}

bool GuiWizardPage::isComplete() const {
    PyObject *return_value_obj = PyObject_CallMethodObjArgs( wizardPageObj,
                                                             PyString_FromString( (char *) "is_complete" ),
                                                             widgetRegistry->getFieldsDict(), wizardPageDictObj, externalData, NULL);
    PyErr_Print();

    if ( PyObject_IsTrue( return_value_obj ) ) {
        return true;
    } else {
        return false;
    }
}

int GuiWizardPage::nextId() const {
    PyObject *return_value_obj = PyObject_CallMethodObjArgs( wizardPageObj,
                                                             PyString_FromString( (char *) "get_next_page_id" ),
                                                             widgetRegistry->getFieldsDict(), wizardPageDictObj, externalData, NULL);
    PyErr_Print();

    Py_ssize_t return_value;
    if ( !PyInt_Check( return_value_obj ) ) {
        qInfo( "Wizard returned a non-integer for get_next_page_id!" );
        return_value = -2;
    }

    return_value = PyInt_AsSsize_t( return_value_obj );
//    qInfo( "return_value: %i", return_value );
    if ( return_value == -2 ) {
        bool found_current_page = false;
        foreach ( int id, ( (GuiWizard *) wizard() )->pageIds() ) {
            if ( id == this->pageId ) {
                found_current_page = true;
            } else if ( found_current_page ) {
                return id;
            }
        }
        return -1;
    }

    return return_value;
}

void GuiWizardPage::connectGuiWidgetToCompleteChanged( GuiWidget *gui_widget ) {
    QWidget *widget = gui_widget->getWidget();
    QString widget_type = gui_widget->getWidgetType();

    if ( widget_type.toLower() == "checkbox" ) {
        QCheckBox *checkbox = (QCheckBox *) widget;
        connect( checkbox, &QCheckBox::clicked, [=] () {
            emit completeChanged();
        });

    } else if ( widget_type.toLower() == "textedit" ) {
        QTextEdit *text_edit = (QTextEdit *) widget;
        connect( text_edit, &QTextEdit::textChanged, [=] () {
            emit completeChanged();
        });


    } else if ( widget_type.toLower() == "lineedit" ) {
        QLineEdit *line_edit = (QLineEdit *) widget;
        connect( line_edit, &QLineEdit::textChanged, [=] () {
            emit completeChanged();
        });

    } else if ( widget_type.toLower() == "spinbox" ) {
        QSpinBox *spin_box = (QSpinBox *) widget;
        spin_box->connect( spin_box, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=] () {
            emit completeChanged();
        });

    } else if ( widget_type.toLower() == "combobox" ) {
        QComboBox *combo_box = (QComboBox *) widget;
        connect( combo_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=] () {
            emit completeChanged();
        });

    } else if ( widget_type.toLower() == "listbox" ) {
        QListWidget *list_box = (QListWidget *) widget;
        connect( list_box, &QListWidget::currentRowChanged, [=] () {
            emit completeChanged();
        });

    } else if ( widget_type.toLower() == "duallist" ) {
        DualListWidget *dual = (DualListWidget *) widget;
        QStandardItemModel *model = (QStandardItemModel *) dual->getChosenList()->model();

        connect( model, &QStandardItemModel::rowsInserted, [=] () {
            emit completeChanged();
        });

        connect( model, &QStandardItemModel::rowsRemoved, [=] () {
            emit completeChanged();
        });

    } else if ( widget_type.toLower() == "image" ) {
        ImageWidget *image = (ImageWidget *) widget;
        connect( image, &ImageWidget::imageChanged, [=] () {
            emit completeChanged();
        });
    }

}

