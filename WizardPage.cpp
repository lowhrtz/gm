#include "WizardPage.h"
#include "Dice.h"
#include "Dialogs.h"
#include "PyToolButton.h"
#include "PDFCreator.h"
#include <QButtonGroup>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDirIterator>
#include <QDrag>
#include <QDragMoveEvent>
#include <QFileDialog>
#include <QGridLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QListWidget>
#include <QMimeData>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QSqlRecord>
#include <QTextStream>
#include <QToolButton>

WizardPage::WizardPage(PyObject *pyWizardPageInstance, QWidget *parent) :
    QWizardPage(parent) {
    this->pyWizardPageInstance = pyWizardPageInstance;
    this->wizard = (CharacterCreationWizard *) parent;

    pageTitle = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_title", NULL));
    pageSubtitle = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_subtitle", NULL));
    pageId = PyInt_AsSsize_t(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_id", NULL));
    content = PyObject_CallMethod(pyWizardPageInstance, (char *) "get_content", NULL);
    if(pageTitle == NULL || pageSubtitle == NULL || content == NULL || pageId == -1) {
        PyErr_Print();
    }
    setTitle(pageTitle);
    setSubTitle(pageSubtitle);

    // Create a field attribute for each python wizard page instance that will get updated on each new page
    // and when the fields are changed.
    PyObject_SetAttrString(pyWizardPageInstance, "fields", wizard->fieldsDict);

    // Create a pages attribute after addig this instance
    QString py_class_name = PyString_AsString(PyObject_GetAttrString(PyObject_Type(pyWizardPageInstance), "__name__"));
//    qInfo("py_class_name: %s", py_class_name.toStdString().data());
    PyDict_SetItemString(wizard->pyWizardPageList, py_class_name.toStdString().data(), pyWizardPageInstance);
    PyObject_SetAttrString(pyWizardPageInstance, "pages", wizard->pyWizardPageList);
}

void WizardPage::publicRegisterField( const QString &name, QWidget *widget, const char *property, const char *changedSignal ) {
    registerField( name, widget, property, changedSignal );
    wizard->field_name_to_widget_hash[name] = widget;

    QString new_name = name;
    new_name.replace( "*", "" );
    PyDict_SetItemString( wizard->fieldsDict, new_name.toStdString().data(), Py_None );

    QString widget_type = widget->metaObject()->className();
    if ( widget_type == "QLineEdit" ) {
        connect( (QLineEdit *) widget, &QLineEdit::textChanged, [=]( QString text ) {
//           qInfo("text: %s", text.toStdString().data());
            PyObject *text_obj = PyString_FromString( text.toStdString().data() );
            PyDict_SetItemString( wizard->fieldsDict, new_name.toStdString().data(), text_obj );
        });
    } else if ( widget_type == "StackedWidget" ) {
        StackedWidget *stacked_widget = (StackedWidget *) widget;
        if ( strcmp( property, "currentItemIndex" ) == 0 ) {
            connect( stacked_widget, &StackedWidget::currentItemChanged, [=] ( int index ) {
                if ( index < stacked_widget->getDataList().length() && index >= 0 ) {
                    PyObject *data = stacked_widget->getData( index );
                    PyDict_SetItemString( wizard->fieldsDict, new_name.toStdString().data(), data );
                }
            });
        } else {
            connect( stacked_widget, &StackedWidget::dataListChanged, [=] ( QList<PyObject *> data_list ) {
                PyObject *list_obj = PyList_New(0);
                foreach( PyObject *data_item, data_list ) {
                    PyList_Append( list_obj, data_item );
                }
                PyDict_SetItemString( wizard->fieldsDict, new_name.toStdString().data(), list_obj );
            });
        }
    } else if ( widget_type == "QCheckBox" ) {
        QCheckBox *checkbox = (QCheckBox *) widget;
        connect( checkbox, &QCheckBox::clicked, [=] ( bool checked ) {
            PyObject *bool_obj = PyBool_FromLong( checked );
            PyDict_SetItemString( wizard->fieldsDict, new_name.toStdString().data(), bool_obj );
        });
    } else if ( widget_type == "QSpinBox" ) {
        QSpinBox *spinbox = (QSpinBox *) widget;
        connect(spinbox, static_cast<void(QSpinBox::*)(int)>( &QSpinBox::valueChanged ), [=] ( int value ) {
            Py_ssize_t pysafe_value = value;
            PyDict_SetItemString( wizard->fieldsDict, new_name.toStdString().data(), PyInt_FromLong( pysafe_value ) );
        });
    }
}

void WizardPage::cleanupPage() {
    disconnect( wizard, &CharacterCreationWizard::customButtonClicked, 0, 0 );
    wizard->setOption(QWizard::HaveCustomButton1, false);
}

void WizardPage::initializePage() {
    PyObject *keys = PyDict_Keys( wizard->fieldsDict );
    for( Py_ssize_t i = 0 ; i < PyList_Size(keys) ; i++ ) {
        PyObject *field_name_obj = PyList_GetItem(keys, i);
        QString field_name = PyString_AsString(field_name_obj);
        if ( wizard->pyOnlyFields.contains( field_name ) ) {
            continue;
        }
        QVariant field_data = field( field_name );
        QString field_data_type = field_data.typeName();

        if ( field_data_type == "QString" ) {
            PyObject *field_data_obj = Py_BuildValue("s", field_data.toString().toStdString().data());
            PyDict_SetItemString(wizard->fieldsDict, field_name.toStdString().data(), field_data_obj);

        } else if ( field_data_type == "bool" ) {
            //qInfo("bool: %s", field_name.toStdString().data());
            PyObject *bool_obj = PyBool_FromLong( field_data.toBool() );
            PyDict_SetItemString( wizard->fieldsDict, field_name.toStdString().data(), bool_obj );

        } else if ( field_data_type == "int" ) {
            QWidget *field_widget = wizard->field_name_to_widget_hash[field_name];
            QString field_widget_type = field_widget->metaObject()->className();
            if ( field_widget_type == "StackedWidget" ) {
                StackedWidget *stacked_widget = (StackedWidget *) field_widget;
                QList<PyObject *> data_list = stacked_widget->getDataList();
                if ( data_list.count() > 0 ) {
//                    qInfo("Field Name: %s, Data List Count: %i", field_name.toStdString().data(), data_list.count());
                    QString chosen_item = stacked_widget->getCurrentItemText();
                    int data_index = stacked_widget->getCurrentItemIndex();
                    PyObject *data = stacked_widget->getData(data_index);
                    PyObject *field_data_obj;
                    if ( data != Py_None ) {
                        field_data_obj = data;
                    } else {
                        field_data_obj = PyString_FromString(chosen_item.toStdString().data());
                    }
                    PyDict_SetItemString( wizard->fieldsDict, field_name.toStdString().data(), field_data_obj );
                }
            } else if ( field_widget_type == "QSpinBox" ) {
                Py_ssize_t spinbox_value = field_data.toInt();
                PyDict_SetItemString( wizard->fieldsDict, field_name.toStdString().data(), PyInt_FromLong( spinbox_value ) );
            }

        } else if ( field_data_type == "QList<PyObject*>" ) {
            QWidget *field_widget = wizard->field_name_to_widget_hash[field_name];
            QString field_widget_type = field_widget->metaObject()->className();
            if ( field_widget_type == "StackedWidget" ) {
                StackedWidget *stacked_widget = (StackedWidget *) field_widget;
                QList<PyObject *> data_list = stacked_widget->getDataList();
//                qInfo("Field Name: %s, Data List Count: %i", field_name.toStdString().data(), data_list.count());
                PyObject *data_list_obj = PyList_New(0);
                foreach( PyObject *data, data_list ) {
                    PyList_Append(data_list_obj, data);
                }
                PyObject *field_data_obj = data_list_obj;
                PyDict_SetItemString( wizard->fieldsDict, field_name.toStdString().data(), field_data_obj );
            }
        }
    }

    // Custom Button
    PyObject *custom_button1_tuple = PyObject_GetAttrString( pyWizardPageInstance, "custom_button1" );
    if ( custom_button1_tuple != Py_None ) {
        PyObject *button_text_obj = PyTuple_GetItem( custom_button1_tuple, 0 );
        PyObject *action_type_obj = PyTuple_GetItem( custom_button1_tuple, 1 );
        PyObject *callable = PyTuple_GetItem( custom_button1_tuple, 2 );
        QString custom_button_text = PyString_AsString( button_text_obj );
        //qInfo( "custom_button_text: %s", custom_button_text.toStdString().data() );
        QString action_type = PyString_AsString( action_type_obj );
        PyObject *callable_obj;
        if ( PyString_Check( callable ) ) {
            char *callable_name = PyString_AsString( callable );
            callable_obj = PyObject_GetAttrString( pyWizardPageInstance, callable_name );
        } else {
            callable_obj = callable;
        }

        if ( action_type.toLower() == "pdf" ) {
            //qInfo( "Set up PDF function" );
            connect( wizard, &CharacterCreationWizard::customButtonClicked, [=] ( int which ) {
                if ( which == QWizard::CustomButton1 ) {
                    PyObject *callable_return = PyObject_CallObject( callable_obj, NULL );
                    PyErr_Print();
                    PyObject *filename_obj = PyTuple_GetItem( callable_return, 0 );
                    PyObject *markup_obj = PyTuple_GetItem( callable_return, 1 );
                    QString filename = PyString_AsString( filename_obj );
                    QString markup = PyString_AsString( markup_obj );
                    buttonPushedPDF( markup, filename );
                }
            });
        }

        wizard->setButtonText( QWizard::CustomButton1, custom_button_text );
        wizard->setOption( QWizard::HaveCustomButton1, true );
    }

}

int WizardPage::nextId() const {
//    int return_value;
//    qInfo("Type: %s", nextIdArgs.typeName());
//    if (nextIdArgs.isNull()) {
//        qInfo( "NextId NULL args" );
//        return_value = PyInt_AsSsize_t(PyObject_CallMethod(this->pyWizardPageInstance, (char *) "get_next_page_id", NULL));
//        PyErr_Print();
//    }

//    else if (strcmp(nextIdArgs.typeName(), "int") == 0) {
//        qInfo( "NextId int args" );
//        return_value = PyInt_AsSsize_t(PyObject_CallMethod(this->pyWizardPageInstance, (char *) "get_next_page_id", (char *) "i", nextIdArgs.toInt()));
//        PyErr_Print();
//    }

//    else if (strcmp(nextIdArgs.typeName(), "bool") == 0) {
//        int argInt = 0;
//        if (nextIdArgs.toBool()) {
//            argInt = 1;
//        }
//        return_value = PyInt_AsSsize_t(PyObject_CallMethod(this->pyWizardPageInstance, (char *) "get_next_page_id", (char *) "i", argInt));
//        PyErr_Print();

//    } else if (strcmp(nextIdArgs.typeName(), "QString") == 0) {
//        if(nextIdArgs.toString().startsWith("^$")) {
////            qInfo("nextIdArgs: %s", nextIdArgs.toString().toStdString().data());
//            PyObject *method = PyObject_GetAttrString(this->pyWizardPageInstance, "get_next_page_id");
//            PyObject *arg = this->parseArgTemplateString(nextIdArgs.toString());
//            if(arg == NULL) {
//                return_value = -1;
//            } else {
//                return_value = PyInt_AsSsize_t(PyObject_CallObject(method, arg));
//                PyErr_Print();
//            }
////            qInfo("return_value: %i", return_value);
//        }
//    }

//    else {
//        return_value = PyInt_AsSsize_t(PyObject_CallMethod(this->pyWizardPageInstance, (char *) "get_next_page_id", NULL));
//    }


    int return_value = PyInt_AsSsize_t(PyObject_CallMethod( this->pyWizardPageInstance, (char *) "get_next_page_id", NULL ) );
    PyErr_Print();
//    qInfo( "return_value: %i", return_value );
    if ( return_value == -2 ) {
        bool foundCurrentPage = false;
        foreach ( int id, wizard->pageIds() ) {
            if ( id == this->pageId ) {
                foundCurrentPage = true;
            } else if ( foundCurrentPage ) {
                return id;
            }
        }
        return -1;
    }

    return return_value;
}

PyObject *WizardPage::parseArgTemplateString(QString templateString) const {
    PyObject *arg;
    QList<PyObject *> pyobject_list;
    QRegularExpression re("(\\w+\\{[^\\{^\\}]*\\}+)");
    QRegularExpressionMatchIterator match_iter = re.globalMatch(templateString);
    Py_ssize_t match_index = 0;
    while(match_iter.hasNext()) {
        QRegularExpressionMatch match = match_iter.next();
        QString match_string = match.captured();
        QStringList match_string_split = match_string.split("{");
        QString source_type = match_string_split[0];
        QString source = match_string_split[1].replace("}", "");

        QStringList source_split = source.split(".");
        if(source_type == "DB") {
            arg = PyList_New(0);
            QString table_name = source_split[0];
            QString cols = "*";
            QString where;
            QString where_value;
            QList<QString> additional_where_values;
            QList<QSqlRecord> sql_rows;
            if(source_split.length() > 1) {
                for(int i = 1;i < source_split.length();i++) {
                    QString query_type = source_split[i];
                    if(query_type.toLower().startsWith("cols")) {
                        cols = query_type;
                        cols.replace("cols(", "");
                        cols.chop(1);
//                        cols.replace(" ", "");
                    } else if(query_type.toLower().startsWith("where")) {
                        QString where_string;
                        where_string = query_type;
                        where_string.replace("where(", "");
                        where_string.chop(1);
                        QList<QString> where_split = where_string.split("=");
                        where = where_split[0].trimmed();
                        QList<QString> where_value_split = where_split[1].trimmed().split(":");
                        if(where_value_split.length() > 1) {
                            QString where_value_field = where_value_split[0];
                            QString where_value_column = where_value_split[1];
                            QWidget *field_widget = wizard->field_name_to_widget_hash[where_value_field];
                            QString field_widget_type = field_widget->metaObject()->className();
                            if(field_widget_type == "StackedWidget") {
                                StackedWidget *stacked_widget = (StackedWidget *) field_widget;
                                int data_index = stacked_widget->getCurrentItemIndex();
                                PyObject *data = stacked_widget->getData(data_index);
                                //                            qInfo("data dict: %s", PyDict_Check(data) ? "true":"false");
                                if(PyDict_Check(data)) {
                                    PyObject *where_value_pyobject = PyDict_GetItemString(data, where_value_column.toStdString().data());
                                    if(PyList_Check(where_value_pyobject)) {
                                        int where_list_size = PyList_Size(where_value_pyobject);
                                        where_value = PyString_AsString(PyList_GetItem(where_value_pyobject, 0));
                                        for(int j = 1;j < where_list_size;j++) {
                                            PyObject *add_value_oject = PyList_GetItem(where_value_pyobject, j);
                                            additional_where_values.append(PyString_AsString(add_value_oject));
                                        }
                                    } else {
                                        where_value = PyString_AsString(where_value_pyobject);
                                    }
                                }
                            }
                        } else {
                            where_value = where_value_split[0];
//                            qInfo("additional_where_values.length: %i", additional_where_values.length());
//                            qInfo("where: %s", where.toStdString().data());
//                            qInfo("where_value: %s", where_value.toStdString().data());
                        }
                    }
                }
                sql_rows = wizard->getDb()->getColRowsAsSqlRecord(table_name, cols, where, where_value, additional_where_values);
            } else {
                sql_rows = wizard->getDb()->getColRowsAsSqlRecord(table_name);
            }
            QString meta_table_name = wizard->getPythonInterpreter()->getMetaTableName(table_name);
            for(Py_ssize_t i = 0;i < sql_rows.length();i++) {
//                QList<QVariant> *row = sql_rows.at(i);
                QSqlRecord sql_row = sql_rows.at(i);
//                qInfo("row: %d", i);
//                PyObject *row_dict = wizard->getPythonInterpreter()->makeDictFromRow(sql_row, table_name, wizard->getDb());
                PyObject *row_dict = wizard->getPythonInterpreter()->makeDictFromSqlRecord(sql_row);
                if(meta_table_name != NULL) {
                    int refCol = wizard->getPythonInterpreter()->getMetaReferenceCol(meta_table_name);
                    QString refColName = wizard->getDb()->getColName(meta_table_name, refCol);
                    QString refIndexName = wizard->getPythonInterpreter()->getReferenceIndexName(meta_table_name);
//                    int refIndex = wizard->getDb()->getColFromName(table_name, refIndexName);
                    QString refName = PyString_AsString(PyDict_GetItemString(row_dict, refIndexName.toStdString().data()));
//                    qInfo("refName: %s", refName.toStdString().data());
                    QList<QSqlRecord> meta_rows = wizard->getDb()->getColRowsAsSqlRecord(meta_table_name, "*", refColName, refName);
                    PyObject *meta_list = PyList_New(0);
                    for(Py_ssize_t j = 0;j < meta_rows.length();j++) {
                        QSqlRecord meta_row = meta_rows.at(j);
                        PyObject *meta_row_dict = wizard->getPythonInterpreter()->makeDictFromSqlRecord(meta_row);
                        PyList_Append(meta_list, meta_row_dict);
                    }
                    PyDict_SetItemString(row_dict, meta_table_name.toStdString().data(), meta_list);
                }
                PyList_Append(arg, row_dict);
            }
            pyobject_list.append(arg);

        } else if(source_type == "WP") {
//            PyObject *arg;
            if(source_split[0] == "attributes") {
                if(wizard->attributes.isEmpty()) {
                    arg = Py_None;
                } else if(source_split.length() == 1) {
                    arg = PyDict_New();
                    QHashIterator<QString, QString> attr_iter(wizard->attributes);
                    while(attr_iter.hasNext()) {
                        attr_iter.next();
                        const QString attr_key = attr_iter.key();
                        const QString attr_score_string = attr_iter.value();
                        PyDict_SetItemString(arg, attr_key.toStdString().data(), Py_BuildValue("s", attr_score_string.toStdString().data()));
                    }
                } else {
                    QString attr_name = source_split[1];
                    QString attr_score = wizard->attributes[attr_name];
                    arg = PyString_FromString(attr_score.toStdString().data());
                }
            } else {
                int page_id = source_split[0].toInt();
//                qInfo("page_id: %i", page_id);
                WizardPage *ref_page = (WizardPage *) wizard->page(page_id);
                arg = Py_None;
                if(source_split.length() > 1) {
                    QString page_prop_name = source_split[1];
                    PyObject *page_prop_obj = PyObject_GetAttrString(ref_page->pyWizardPageInstance, page_prop_name.toStdString().data());
                    arg = page_prop_obj;
                    if(source_split.length() > 2) {
                        QString dict_key = source_split[2];
                        if(PyDict_Check(page_prop_obj)) {
//                            qInfo("dict_key: %s", dict_key.toStdString().data());
                            arg = PyDict_GetItemString(page_prop_obj, dict_key.toStdString().data());
                        } else if(PyList_Check(page_prop_obj)) {
                            QString list_index = source_split[2];
                            arg = PyList_GetItem(page_prop_obj, list_index.toInt());
                        }
                    }
                }
            }
//            PyTuple_SetItem(return_object, match_index, wp_arg);
            pyobject_list.append(arg);

        } else if(source_type == "F") {
//            PyObject *arg;
            QString field_name = source_split[0];
            QVariant field_data = field(field_name);
            QString field_data_type = field_data.typeName();
//            qInfo("Field Data Type: %s", field_data_type.toStdString().data());
            if(field_data_type == "QString") {
                arg = PyString_FromString(field_data.toString().toStdString().data());
            } else if(field_data_type == "int") {
                QWidget *field_widget = wizard->field_name_to_widget_hash[field_name];
                QString field_widget_type = field_widget->metaObject()->className();
                if(field_widget_type == "StackedWidget") {
                    StackedWidget *stacked_widget = (StackedWidget *) field_widget;
                    QString chosen_item = stacked_widget->getCurrentItemText();
                    int data_index = stacked_widget->getCurrentItemIndex();
                    PyObject *data = stacked_widget->getData(data_index);
                    if(data != Py_None) {
                        arg = data;
                    } else {
                        arg = PyString_FromString(chosen_item.toStdString().data());
                    }
                }
            } else if(field_data_type == "QList<PyObject*>") {
                QWidget *field_widget = wizard->field_name_to_widget_hash[field_name];
                QString field_widget_type = field_widget->metaObject()->className();
                if(field_widget_type == "StackedWidget") {
                    StackedWidget *stacked_widget = (StackedWidget *) field_widget;
                    QList<PyObject *> dataList = stacked_widget->getDataList();
//                    int match_index = return_string.indexOf(match_string, match.lastCapturedIndex());
//                    QString data_list_string;
//                    if(dataList.isEmpty()) {
//                        data_list_string = "&lt;None&gt;";
//                    } else {
//                        foreach(PyObject *data, dataList) {
//                            QString display_key("Name");
//                            if(source_split.length() > 1) {
//                                display_key = source_split[1];
//                            }
//                            PyObject *data_object = PyDict_GetItemString(data, display_key.toStdString().data());
//                            QString data_string = PyString_AsString(data_object);
//                            data_list_string += data_string + ", ";
//                        }
//                        data_list_string.chop(2);
//                    }
//                    return_string.replace(match_index, match_string.length(), data_list_string);
                    PyObject *dataListObj = PyList_New(0);
                    foreach(PyObject *data, dataList) {
                        PyList_Append(dataListObj, data);
                    }
                    arg = dataListObj;
                }
            }
            pyobject_list.append(arg);

        } else if(source_type == "LIT") {
            QString literal = source_split[0];
            QRegExp digit_re("\\d*");
            if(literal.startsWith("'") || literal.startsWith("\"")) {
                literal.remove(0, 1);
                if(literal.endsWith("'") || literal.endsWith("\"")) {
                    literal.chop(1);
                }
//                qInfo("literal: %s", literal.toStdString().data());
                arg = PyString_FromString(literal.toStdString().data());
            } else if(digit_re.exactMatch(literal)) {
                arg = PyInt_FromLong(literal.toInt());
            }
            pyobject_list.append(arg);
        }

        match_index++;
    }

    Py_ssize_t pyobject_list_length = pyobject_list.length();
    PyObject *return_object = PyTuple_New(pyobject_list_length);
    for(Py_ssize_t i = 0;i < pyobject_list_length;i++) {
        PyTuple_SetItem(return_object, i, pyobject_list.at(i));
    }

//    qInfo("Tuple Size: %d", PyTuple_Size(return_object));

    return return_object;
}

QString WizardPage::parseTextTemplateString(QString templateString) {
    QString return_string(templateString);
//    QRegularExpression re("(\\w+\\{[^\\{^\\}]*\\}+)");
//    QRegularExpression re("(\\w+\\{.*\\}+)");
//    QRegularExpression re("(WP\\{.*\\}+|F\\{[^\\{^\\}]*\\}+)");
    QRegularExpression re("(WP\\{.*\\}+|F\\{.+?\\}+|MD\\{.*\\}+|LIT\\{.*\\}+)");
    QRegularExpressionMatchIterator match_iter = re.globalMatch(templateString);
    while(match_iter.hasNext()) {
        QRegularExpressionMatch match = match_iter.next();
        QString match_string = match.captured();
        QString source_type = match_string.section("{", 0, 0);
        QString source = match_string.section("{", 1);
        source.chop(1);
        QStringList source_split = source.split(".");

        if(source_type == "F") {
            QString field_name = source_split[0];
            QVariant field_data = field(field_name);
            QString field_data_type = field_data.typeName();

            if(field_data_type == "QString") {
                return_string.replace(match_string, field_data.toString());

            } else if(field_data_type == "int") {
                QWidget *field_widget = wizard->field_name_to_widget_hash[field_name];
                QString field_widget_type = field_widget->metaObject()->className();
                if(field_widget_type == "StackedWidget") {
                    StackedWidget *stacked_widget = (StackedWidget *) field_widget;
                    QString chosen_item = stacked_widget->getCurrentItemText();
                    int match_index = return_string.indexOf(match_string, match.lastCapturedIndex());
//                    return_string.replace(match_string, chosen_item);
                    return_string.replace(match_index, match_string.length(), chosen_item);
                }

            } else if(field_data_type == "QList<PyObject*>") {
                QWidget *field_widget = wizard->field_name_to_widget_hash[field_name];
                QString field_widget_type = field_widget->metaObject()->className();
                if(field_widget_type == "StackedWidget") {
                    StackedWidget *stacked_widget = (StackedWidget *) field_widget;
                    QList<PyObject *> dataList = stacked_widget->getDataList();
                    int match_index = return_string.indexOf(match_string, match.lastCapturedIndex());
                    QString data_list_string;
                    if(dataList.isEmpty()) {
                        data_list_string = "&lt;None&gt;";
                    } else {
                        foreach(PyObject *data, dataList) {
                            QString display_key("Name");
                            if(source_split.length() > 1) {
                                display_key = source_split[1];
                            }
                            PyObject *data_object = PyDict_GetItemString(data, display_key.toStdString().data());
                            QString data_string = PyString_AsString(data_object);
                            data_list_string += data_string + ", ";
                        }
                        data_list_string.chop(2);
                    }
                    return_string.replace(match_index, match_string.length(), data_list_string);
                }
            }

        } else if(source_type == "WP") {
//            qInfo("WP Text");
            QString wp_type = source_split[0];
            QRegExp digit_re("\\d*");  // a digit (\d), zero or more times (*)
            if(wp_type == "attributes") {
                QString method_name_args = source_split[1];
                QStringList method_name_args_list = method_name_args.split("(");
                QString method_name = method_name_args_list[0];
                QString args = method_name_args_list[1];
                args = args.replace(")", "");
                QString attr_display_string;
                if(wizard->attributes.isEmpty()) {
                    PyObject *method_object = PyObject_GetAttrString(pyWizardPageInstance, (char *) method_name.toStdString().data());
                    PyObject *method_return = PyObject_CallObject(method_object, parseArgTemplateString(args));
                    PyErr_Print();
                    if(method_return == NULL || !PyDict_Check(method_return)) {
                        qInfo("Problem calling method: %s\nMake sure the naming is correct and that it returns a dictionary", method_name.toStdString().data());
                        continue;
                    }

                    foreach(QString attr_name, wizard->attributeList) {
                        PyObject *score_object = PyDict_GetItemString(method_return, attr_name.toStdString().data());
                        if(!score_object) {
                            qInfo("Attribute: %s, not a key in returned dictionary. Make sure the dictionary keys are the same as the attribute list given in the Roll Methods Page.", attr_name.toStdString().data());
                            continue;
                        }
                        QString score = PyString_AsString(score_object);
                        attr_display_string += "<b>" + attr_name + "</b>: " + score + "<br />";
                    }
                } else {
                    foreach(QString attrName, wizard->attributeList) {
                        attr_display_string += "<b>" + attrName + "</b>: " + wizard->attributes[attrName] + "<br />";
                    }
                }

                return_string.replace(match_string, attr_display_string);
            } else if(digit_re.exactMatch(wp_type)) {
                int page_id = wp_type.toInt();
                WizardPage *ref_page = (WizardPage *) wizard->page(page_id);
                QString wp_string;
                if(source_split.length() > 1) {
                    QString page_prop_name = source_split[1];
                    PyObject *page_prop_obj = PyObject_GetAttrString(ref_page->pyWizardPageInstance, page_prop_name.toStdString().data());
                    if(PyString_Check(page_prop_obj)) {
                        wp_string = PyString_AsString(page_prop_obj);
                    } else if(PyInt_Check(page_prop_obj)) {
                        wp_string = QString::number(PyInt_AsLong(page_prop_obj));
                    } else if(PyDict_Check(page_prop_obj)) {
                        if(source_split.length() > 2) {
                            QString dict_key = source_split[2];
                            PyObject *dict_item_obj = PyDict_GetItemString(page_prop_obj, dict_key.toStdString().data());
                            if(!PyString_Check(dict_item_obj)) {
                                qInfo("%s shoud return a string.", source.toStdString().data());
                                continue;
                            }
                            wp_string = PyString_AsString(dict_item_obj);
                        } else {
                            wp_string = source;
                        }
                    } else if(PyList_Check(page_prop_obj)) {
                        if(source_split.length() > 2) {
                            QString list_index = source_split[2];
                            PyObject *list_item_obj = PyList_GetItem(page_prop_obj, list_index.toInt());
                            if(!PyString_Check(list_item_obj)) {
                                qInfo("%s shoud return a string.", source);
                                continue;
                            }
                            wp_string = PyString_AsString(list_item_obj);
                        } else {
                            wp_string = source;
                        }
                    }

                }

                int match_index = return_string.indexOf(match_string, match.lastCapturedIndex());
//                return_string.replace(match_string, wp_string);
                return_string.replace(match_index, match_string.length(), wp_string);
            }
        } else if(source_type == "MD") {
//            QString method_name_args = source_split[0];
            QString method_name_args = source;
            QStringList method_name_args_split = method_name_args.split("(");
            QString method_name = method_name_args_split[0];
            QString method_args = method_name_args_split[1];
            method_args.replace(")", "");
            PyObject *method_object = PyObject_GetAttrString(pyWizardPageInstance, (char *) method_name.toStdString().data());
            PyObject *method_return = PyObject_CallObject(method_object, parseArgTemplateString(method_args));
            PyErr_Print();
            if(!PyString_Check(method_return)) {
                qInfo("Problem calling method: %s\nMake sure the naming is correct and that it returns a string", method_name.toStdString().data());
                continue;
            }
            QString output_string = PyString_AsString(method_return);
//            qInfo("output_string: %s", output_string.toStdString().data());
//            int match_index = return_string.indexOf(match_string, match.lastCapturedIndex());
//            qInfo("match_index: %i", match_index);
//            qInfo("match_string.length(): %i", match_string.length());
//            return_string.replace(match_index, match_string.length(), output_string);
            return_string.replace(match_string, output_string);
        }
    }
    return return_string;
}

void WizardPage::setBanner(QString bannerFilePath) {
    QPixmap bannerPixmap(bannerFilePath);

    setPixmap(QWizard::WatermarkPixmap, bannerPixmap);
    setPixmap(QWizard::BackgroundPixmap, bannerPixmap);
}

void WizardPage::buttonPushedPDF(QString markup, QString filename) {
    PDFCreator *pdf = new PDFCreator( parseTextTemplateString( markup ), filename );
    pdf->save();
}

RollMethodsPage::RollMethodsPage(PyObject *pyWizardPageInstance, QWidget *parent) :
    WizardPage(pyWizardPageInstance, parent) {

    QPixmap dicePix(":/images/dice.png"); //, rollBanner(":/images/rollBanner.jpg");
    QLabel *diceLabel;
    PyObject *pyContent, *pyAttributeList, *pyAttribute,
            *pyContentItem, *firstTupleItem, *secondTupleItem, *thirdTupleItem;
    Py_ssize_t pyAttributeListSize, pyContentSize, tupleSize;
    int nextRow = 0;
    QGridLayout *layout;
//    QString pageTitle, pageSubtitle, rollMethod, rollMethodDisplayString, diceString;
    QString rollMethod, rollMethodDisplayString, diceString;

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
    wizard->attributeList = attributeList;

//    rollBanner
//    setPixmap(QWizard::WatermarkPixmap, rollBanner);
//    setPixmap(QWizard::BackgroundPixmap, rollBanner);
    setBanner(":/images/rollBanner.jpg");

    pyContent = PyObject_CallMethod(pyWizardPageInstance, (char *) "get_content", NULL);
    if(!PyList_Check(pyContent)) {
        qInfo("get_content doesn't return a Python list!\n");
        return;
    }

//    pageTitle = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_title", NULL));
//    pageSubtitle = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_subtitle", NULL));
//    pageId = PyInt_AsSsize_t(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_id", NULL));
//    setTitle(pageTitle);
//    setSubTitle(pageSubtitle);

    layout = new QGridLayout;

    pool = new QLineEdit(this);
    pool->setFixedWidth(35);
    pool->setEnabled(false);

    rollMethodSelector = new QComboBox(this);
    pyContentSize = PyList_Size(pyContent);
    for(int i = 0;i < pyContentSize;i++) {
        pyContentItem = PyList_GetItem(pyContent, i);
        if(!pyContentItem ||
                !PyTuple_Check(pyContentItem)) {
            qInfo("content item is not a tuple.\n");
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
    connect(rollButton, &QToolButton::clicked, this, &RollMethodsPage::buttonClicked);
    rollButton->setText("Roll");
    layout->addWidget(rollButton, nextRow, 2);
    layout->addWidget(pool, nextRow, 2);
    layout->addWidget(poolLabel, nextRow, 3);

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
    WizardPage::initializePage();
    emit rollMethodSelector->currentIndexChanged(0);
}

bool RollMethodsPage::validatePage(){
    bool return_bool = true;
    QString rollMethod = rollMethodSelector->currentData(RollMethodRole).toString().toLower();
    if(rollMethod.startsWith("pool")) {
        int pool_int = pool->text().toInt();
        if(pool_int > 0) {
            QString points = "points";
            if(pool_int == 1) points = "point";
            QString messageString;
            QTextStream(&messageString) << "You still have " << pool_int << " " << points << " left";

            if(rollMethod.endsWith("forceuse")) {
                QTextStream(&messageString) << " to distribute.";
                PopupDialog *popup = new PopupDialog("Points Remaining", messageString, this);
                popup->exec();
                return_bool = false;
            } else {
                QTextStream(&messageString) << ". Continue?";
                YesNoDialog *dialog = new YesNoDialog("Points Remaining", messageString, this);
                return_bool = dialog->exec();
            }
        }
    }

    if(return_bool == true) {
        foreach(QString attr, attributeList) {
//            qInfo("attr: %s, score: %s", attr.toStdString().data(), wizard->attributes[attr].toStdString().data());
            QString score = getField(attr).toString();
            wizard->attributes[attr] = score;
//            qInfo("attr: %s, score: %s", attr.toStdString().data(), wizard->attributes[attr].toStdString().data());
        }
    }

    return return_bool;
}

void RollMethodsPage::cleanupPage() {
    WizardPage::cleanupPage();
    wizard->attributes.clear();
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

InfoPage::InfoPage(PyObject *pyWizardPageInstance, QWidget *parent) :
    WizardPage(pyWizardPageInstance, parent) {
    PyObject *pyContent, *pyContentItem,
            *firstTupleItem, *secondTupleItem, *banner;
    Py_ssize_t pyContentSize, tupleSize;
//    QGridLayout *layout;
    QBoxLayout *layout, *choose_layout;
//    QLayout *layout, *choose_layout;
    Qt::Alignment layoutAlign;
//    QString pageTitle, pageSubtitle, layoutString, itemType, itemName, bannerString;
    QString layoutString, itemType, itemName, bannerString;

    pyContent = PyObject_CallMethod(pyWizardPageInstance, (char *) "get_content", NULL);
    if(!PyList_Check(pyContent)) {
        printf("Content not a Python list!\n");
        return;
    }

//    pageTitle = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_title", NULL));
//    pageSubtitle = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_subtitle", NULL));
//    pageId = PyInt_AsSsize_t(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_page_id", NULL));
//    setTitle(pageTitle);
//    setSubTitle(pageSubtitle);

    layoutString = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_layout", NULL));
    /*if(layoutString.toLower() == "grid") {
        layout = new QGridLayout;
        layoutAlign = Qt::AlignVCenter;
    } else*/ if(layoutString.toLower() == "horizontal") {
        layout = new QHBoxLayout;
        layoutAlign = Qt::AlignVCenter;
    } else {
        layout = new QVBoxLayout;
        layoutAlign = Qt::AlignHCenter;
    }
    layout->setAlignment(Qt::AlignCenter);
//    layout = new QGridLayout;
    pyContentSize = PyList_Size(pyContent);
    for(int i = 0;i < pyContentSize;i++) {
        pyContentItem = PyList_GetItem(pyContent, i);
        if(!PyTuple_Check(pyContentItem) ||
                (tupleSize = PyTuple_Size(pyContentItem)) < 2) {
            qInfo("Content item not a Python tupel of at least size 2!\n");
            continue;
        }
        firstTupleItem = PyTuple_GetItem(pyContentItem, 0);
        secondTupleItem = PyTuple_GetItem(pyContentItem, 1);
        itemType = PyString_AsString(firstTupleItem);
        itemName = PyString_AsString(secondTupleItem);

        bool hide_field_name = false;
        if(itemName.endsWith("_")) {
            hide_field_name = true;
            itemName.chop(1);
        }

        if(itemType.toLower() == "text") {
            QLabel *textLabel;
            if(tupleSize > 2 && (PyTuple_GetItem(pyContentItem, 2) == Py_True)) {
                layoutAlign = Qt::AlignTop;
                textLabel = new QLabel(this);
                addParsableString(textLabel, itemName);
            } else {
                textLabel = new QLabel(itemName, this);
            }
//            layout->addWidget(textLabel, i, 0, 1, 2, Qt::AlignCenter);
            layout->addWidget(textLabel, (12 / pyContentSize), layoutAlign);

        } else if(itemType.toLower() == "field") {
            QBoxLayout *field_layout;
            QLabel *fieldLabel = new QLabel(itemName);
            QLineEdit *field = new QLineEdit(this);
            publicRegisterField(getMandatoryString(itemName, pyContentItem), field);
//            connect(field, &QLineEdit::textChanged, [=](QString text) {
////                qInfo("field data: %s", text.toStdString().data());
//                PyObject *text_obj = PyString_FromString(text.toStdString().data());
//                PyDict_SetItemString(wizard->fieldsDict, itemName.toStdString().data(), text_obj);
//            });
            if(layoutString.toLower() == "horizontal") {
                field_layout = new QVBoxLayout;
            } else {
                field_layout = new QHBoxLayout;
            }
            if(!hide_field_name) {
//                layout->addWidget(fieldLabel, i, 0, Qt::AlignLeft);
                field_layout->addWidget(fieldLabel);
            }
//            layout->addWidget(field, i, 1);
            field_layout->addWidget(field);
            layout->addLayout(field_layout, layoutAlign);

        } else if(itemType.toLower().startsWith("choose") || itemType.toLower().startsWith("listbox")) {
            if(tupleSize < 4) {
                qInfo("Choose tuple has a size of less than 4!\n");
                continue;
            }
            QLabel *chooseLabel = new QLabel(itemName);
//            ComboRow *choose = new ComboRow(this, wizard->getDb(), wizard->getPythonInterpreter());
            StackedWidget *choose = new StackedWidget(this, itemType, wizard->getDb(), wizard->getPythonInterpreter());
//            choose->setObjectName(itemName);
            QString source(PyString_AsString(PyTuple_GetItem(pyContentItem, 2)));
            PyObject *fourthTupleItem = PyTuple_GetItem(pyContentItem, 3);
            PyObject *fifthTupleItem;
            QString fifthItemString;
            if(tupleSize < 5 ||
                    (fifthTupleItem = PyTuple_GetItem(pyContentItem, 4)) == Py_None) {
                fifthTupleItem = NULL;
            } else {
                if(PyString_Check(fifthTupleItem)) {
                    fifthItemString = PyString_AsString(fifthTupleItem);
                }
            }
            if(itemType.toLower() == "listbox" || layoutString.toLower() == "horizontal") {
                choose_layout = new QVBoxLayout;
//                choose->addWidget(new QListWidget(this));
//                choose->addWidget(new QListWidget(choose));
            } else {
                choose_layout = new QHBoxLayout;
//                choose->addWidget(new QComboBox(this));
//                choose->addWidget(new QComboBox(choose));
            }
            if(source.toLower() == "settings") {
                QString propName(PyString_AsString(fourthTupleItem));
                choose->addItems(wizard->getPythonInterpreter()->settingAsStringList(propName));
            } else if( source.toLower().startsWith("method") || source.toLower().startsWith("function") ) {
                if(source.toLower().startsWith("method")) {
                    PyObject *method = PyObject_GetAttr(pyWizardPageInstance, fourthTupleItem);
                    addInitCallable(choose, method, fifthItemString);
                } else if(source.toLower().startsWith("function")) {
                    addInitCallable(choose, fourthTupleItem, fifthItemString);
                }
            } else {
                QString propName(PyString_AsString(fourthTupleItem));
//                fillCombo(choose, propName);
                choose->fillComboRow(propName);
            }
            PyObject *sixth_tuple_item;
            QString template_string;
            if(tupleSize > 5) {
                sixth_tuple_item = PyTuple_GetItem(pyContentItem, 5);
            } else {
                sixth_tuple_item = NULL;
            }
            if(itemType.toLower().endsWith("determinesnext")) {
                nextIdArgs = "dummy";
                if(PyString_Check(sixth_tuple_item)) {
                    template_string = PyString_AsString(sixth_tuple_item);
                }
                connect(choose, &StackedWidget::currentItemChanged, [=](int itemIndex) {
                    nextIdArgs = template_string;
                });
//                QString templateString;
//                QTextStream(&templateString) << "^$ F{" << itemName << "}";
//                nextIdArgs = templateString;
            } else if(itemType.toLower().endsWith("halfwidth")) {
//                ((QListWidget *) choose->currentWidget())->setFixedWidth(100);
                choose->setFixedWidth(100);
            } else if(itemType.toLower().endsWith("halfheight")) {
                choose->setFixedHeight(100);
            } else if(itemType.toLower().endsWith("halfsize")) {
                choose->setFixedWidth(100);
                choose->setFixedHeight(100);
            }
            publicRegisterField(getMandatoryString(itemName, pyContentItem), choose, "currentItemIndex", "currentItemChanged");
//            publicRegisterField(getMandatoryString(itemName, pyContentItem), choose);
//            connect(choose, &StackedWidget::currentItemChanged, [=]( int item_index ) {
//                PyObject *data = choose->getData( item_index );
//                PyDict_SetItemString(wizard->fieldsDict, itemName.toStdString().data(), data);
//            });
            if(!hide_field_name) {
                choose_layout->addWidget(chooseLabel);
            }
            choose_layout->addWidget(choose);
//            layout->addLayout(choose_layout, i, 0);
            layout->addLayout(choose_layout, layoutAlign);

        } else if(itemType.toLower().startsWith("checkbox")) {
            QCheckBox *checkbox;
            if(!hide_field_name) {
                checkbox = new QCheckBox(itemName, this);
            } else {
                checkbox = new QCheckBox(this);
            }
            if(itemType.toLower().endsWith("checked")) {
                checkbox->setChecked(true);
            }
            if(tupleSize > 2) {
                nextIdArgs = checkbox->isChecked();
                connect(checkbox, &QCheckBox::toggled, [=](bool checked) {
                    nextIdArgs = checked;
                });
            }
            publicRegisterField(getMandatoryString(itemName, pyContentItem), checkbox);
//            connect(checkbox, &QCheckBox::toggled)
//            layout->addWidget(checkbox, i, 0);
            layout->addWidget(checkbox, layoutAlign);

        } else if(itemType.toLower() == "button" || itemType.toLower() == "buttoncenter") {
            PyToolButton *button = new PyToolButton(this);
            if(!hide_field_name) {
                button->setText(itemName);
            }
            button->setToolButtonStyle(Qt::ToolButtonTextOnly);
//            layout->addWidget(button, i, 0, 1, 2, Qt::AlignCenter);
            layout->addWidget(button, layoutAlign);
            if(tupleSize > 2) {
                QString action_type = PyString_AsString(PyTuple_GetItem(pyContentItem, 2));
                if(action_type.toLower() == "printpdf") {
                    PyObject *callable = PyTuple_GetItem(pyContentItem, 3);
                    QString arg_template = PyString_AsString(PyTuple_GetItem(pyContentItem, 4));
                    button->setPyObject(callable, arg_template);
                }
                connect(button, SIGNAL(clicked(PyObject*, QString)), this, SLOT(buttonPushedPDF(PyObject*, QString)));
//                if(!pyAction ||
//                        !PyTuple_Check(pyAction)) {
//                    PyErr_Print();
//                    PyErr_Clear();
//                    printf("Error getting action tuple!\n");
//                    continue;
//                }
////                currentActionTuple = pyAction;
//                button->setPyObject(pyAction);
//                connect(button, SIGNAL(clicked(PyObject*)), this, SLOT(buttonPushed(PyObject*)));
            }
        } else if(itemType.toLower() == "buttonleft") {
            QToolButton *button = new QToolButton(this);
            button->setText(itemName);
            button->setToolButtonStyle(Qt::ToolButtonTextOnly);
//            layout->addWidget(button, i, 0, Qt::AlignCenter);
            layout->addWidget(button);
        } else if(itemType.toLower() == "buttonright") {
            QToolButton *button = new QToolButton(this);
            button->setText(itemName);
            button->setToolButtonStyle(Qt::ToolButtonTextOnly);
//            layout->addWidget(button, i, 1, Qt::AlignCenter);
            layout->addWidget(button);

        } else if(itemType.toLower().startsWith("image")) {
            QLabel *image_label = new QLabel(this);
            char *path_or_callable_name = PyString_AsString(PyTuple_GetItem(pyContentItem, 2));
            QString image_style;
            PyObject *style_string_object;
            if(itemType.toLower().endsWith("method") ||
                    itemType.toLower().endsWith("function")) {
                PyObject *callable;
                QString bind_widget;
                PyObject *bind_widget_object;
                if(itemType.toLower().endsWith("method")) {
                    callable = PyObject_GetAttrString(pyWizardPageInstance, path_or_callable_name);
                } else {
                    callable = PyTuple_GetItem(pyContentItem, 2);
                }
                if((bind_widget_object = PyTuple_GetItem(pyContentItem, 3)) != Py_None) {
                    bind_widget = PyString_AsString(bind_widget_object);
                }
                addBindCallable(image_label, callable, bind_widget, "image");
                if(tupleSize > 4 &&
                        (style_string_object = PyTuple_GetItem(pyContentItem, 4)) != Py_None) {
                    image_style = PyString_AsString(style_string_object);
                }
            } else {
                QString image_path("portraits/");
                QTextStream(&image_path) << path_or_callable_name;
                QPixmap image_pixmap(image_path);
                image_label->setPixmap(image_pixmap);
            }
            if(!image_style.isEmpty()) {
                image_label->setStyleSheet(image_style);
            }
            layout->addWidget(image_label);

        } else if(itemType.toLower().startsWith("fillhook")) {
            if(tupleSize < 3) {
                printf("fillHook tuple has a size of less than 3!\n");
                continue;
            }
//            QWidget *fillArea = new QWidget(this);
//            QGridLayout *fillAreaLayout = new QGridLayout;
            QBoxLayout *fillAreaLayout;
            if(layoutString.toLower() == "horizontal") {
                fillAreaLayout = new QVBoxLayout;
            } else {
                fillAreaLayout = new QHBoxLayout;
            }
            if(itemType.toLower().endsWith("horizontal")) {
                fillAreaLayout = new QHBoxLayout;
            } else if(itemType.toLower().endsWith("vertical")) {
                fillAreaLayout = new QVBoxLayout;
            }
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
                        QCheckBox *fillCheckbox = new QCheckBox(fillString, this);
                        publicRegisterField(getMandatoryString(fillString, pyContentItem), fillCheckbox);
//                        fillAreaLayout->addWidget(fillCheckbox, j, 1);
                        fillAreaLayout->addWidget(fillCheckbox);
                        continue;
                    } else if(fillHookType.toLower() == "radiobutton") {
                        QString fillString(PyString_AsString(pyFillString));
                        QRadioButton *radioButton = new QRadioButton(fillString, this);
                        radioButtonGroup->addButton(radioButton);
                        radioButtonGroup->setId( radioButton, j );
                        if(j == 0) radioButton->setChecked(true);
                        if (tupleSize > 4) {
                            PyObject *branch_identifier = PyTuple_GetItem(pyContentItem, 4);
                            QString branch_identifier_string = PyString_AsString(branch_identifier);
                            if (branch_identifier_string.compare("index") == 0) {
//                                nextIdArgs = QString::number(0);
                                connect(radioButton, &QRadioButton::pressed, [=]() {
                                    nextIdArgs = j;
                                });
                            }
                        }
                        publicRegisterField(fillString, radioButton);
//                        fillAreaLayout->addWidget(radioButton, j, 1);
                        fillAreaLayout->addWidget(radioButton);
                        continue;
                    } else if(fillHookType.toLower() == "spinbox") {
                        QString fillString(PyString_AsString(pyFillString));
//                        QString fieldName(fillString);
//                        fieldName.append("-spinbox");
                        QSpinBox *spinBox = new QSpinBox(this);
//                        printf("field name: %s\n", fillString.toStdString().data());
                        publicRegisterField(getMandatoryString(fillString, pyContentItem), spinBox);
//                        fillAreaLayout->addWidget(new QLabel(fillString), j, 0);
                        fillAreaLayout->addWidget(new QLabel(fillString));
//                        fillAreaLayout->addWidget(spinBox, j, 1);
                        fillAreaLayout->addWidget(spinBox);
                        continue;
                    }
                }
                QString fillString(PyString_AsString(pyFillString));
                QLabel *fillStringLabel = new QLabel(fillString, this);
                QLineEdit *fillField = new QLineEdit(this);
                publicRegisterField(getMandatoryString(fillString, pyContentItem), fillField);
                QBoxLayout *fill_field_layout;
                if(layoutString.toLower() == "horizontal") {
                    fill_field_layout = new QVBoxLayout;
                } else {
                    fill_field_layout = new QHBoxLayout;
                }
//                fillAreaLayout->addWidget(fillStringLabel, j, 0, Qt::AlignLeft);
//                fillAreaLayout->addWidget(fillField, j, 1);
                fill_field_layout->addWidget(fillStringLabel);
                fill_field_layout->addWidget(fillField);
                fillAreaLayout->addLayout(fill_field_layout);
            }

            // If there are radio buttons and a name has been given for the button group and there is a branch identifier then directly create an
            // entry in the fields dictionary for the button group since it is not a widget and cannot be registered.
            if ( radioButtonGroup->buttons().length() > 0 ) {
                QStringList item_type_split = itemType.split('-');
                if ( item_type_split.length() > 1 &&  item_type_split[1].toLower() != "vertical" && item_type_split[1].toLower() != "horizontal"  ) {
                    if ( tupleSize > 4 ) {
                        PyObject *branch_identifier = PyTuple_GetItem(pyContentItem, 4);
                        QString branch_identifier_string = PyString_AsString(branch_identifier);
                        QString button_group_name = item_type_split[1];
                        if ( branch_identifier_string.toLower() == "index" ) {
                            Py_ssize_t current_id = radioButtonGroup->checkedId();
                            PyDict_SetItemString( wizard->fieldsDict, button_group_name.toStdString().data(), PyInt_FromLong( current_id ) );
                            wizard->pyOnlyFields.append( button_group_name );
                            connect( radioButtonGroup, static_cast<void(QButtonGroup::*)(int)>( &QButtonGroup::buttonClicked ), [=] ( int id ) {
                                //qInfo( "radio button id: %i", id );
                                Py_ssize_t id_for_py = id;
                                PyDict_SetItemString( wizard->fieldsDict, button_group_name.toStdString().data(), PyInt_FromLong( id_for_py ) );
                            });
                        } else if ( branch_identifier_string.toLower() == "string" ) {
                            QString current_button_text = radioButtonGroup->checkedButton()->text();
                            PyObject *text_obj = PyString_FromString( current_button_text.toStdString().data() );
                            PyDict_SetItemString( wizard->fieldsDict, button_group_name.toStdString().data(), text_obj );
                            wizard->pyOnlyFields.append( button_group_name );
                            connect( radioButtonGroup, static_cast<void(QButtonGroup::*)(QAbstractButton *)>( &QButtonGroup::buttonClicked ), [=] ( QAbstractButton *button ) {
                                QString text = button->text();
                                PyObject *t_obj = PyString_FromString( text.toStdString().data() );
                                PyDict_SetItemString( wizard->fieldsDict, button_group_name.toStdString().data(), t_obj );
                            });
                        }
                    }
                }
            }
//            fillAreaLayout->addStretch(10);
//            fillArea->setLayout(fillAreaLayout);
//            layout->addWidget(fillArea, i, 0, 1, 2, Qt::AlignCenter);
//            layout->addWidget(fillArea, Qt::AlignCenter);
            layout->addLayout(fillAreaLayout, layoutAlign);
        }

    }

    layout->addStretch(12 / pyContentSize);
    setLayout(layout);

//    banner = PyObject_CallMethod(pyWizardPageInstance, (char *) "get_banner", NULL);
//    if(PyString_Check(banner)) {
//        bannerString = PyString_AsString(banner);
//    } else if(PyTuple_Check(banner) && PyTuple_Size(banner) > 1) {
//        bannerString = PyString_AsString(PyTuple_GetItem(banner, 0));
//        if(bannerString.startsWith("bind")) {
//            QString banner_bind(PyString_AsString(PyTuple_GetItem(banner, 1)));
//            PyObject *method_name = PyTuple_GetItem(banner, 2);
//            PyObject *method_object = PyObject_GetAttr(pyWizardPageInstance, method_name);
//            QWidget *bind_widget = field_name_to_widget_hash.value(banner_bind);
//            QString bind_widget_type(bind_widget->metaObject()->className());
////            qInfo("bind_widget class: %s", bind_widget_type.toStdString().data());
//            QString bind_widget_parent_type(bind_widget->parent()->metaObject()->className());
//            if(bind_widget_parent_type == "StackedWidget" && bind_widget_type == "QListWidget") {
//                QListWidget *list_widget = (QListWidget *) bind_widget;
//                connect(list_widget, &QListWidget::currentRowChanged, [=](int current_row) {
//                   StackedWidget *stacked = (StackedWidget *) list_widget->parent();
//                   PyObject *data = stacked->getData(current_row);
//                   QString filename = PyString_AsString(PyObject_CallObject(method_object, data));
//                });
//            } else if (bind_widget_parent_type == "StackedWidget" && bind_widget_type == "QComboBox") {
//                QComboBox *combo_widget = (QComboBox *) bind_widget;
////                connect(combo_widget, &QComboBox::currentIndexChanged, [=](int current_index) {
////                    StackedWidget *stacked = (StackedWidget *) combo_widget->parent();
////                    PyObject *data = stacked->getData(current_index);
////                    QString filename = PyString_AsString(PyObject_CallObject(method_object, data));
////                });
//            }
//        }
//    }
//    setPixmap(QWizard::WatermarkPixmap, rollBanner);
//    setPixmap(QWizard::BackgroundPixmap, rollBanner);
    foreach(BindCallable bind_callable, bind_callable_list) {
        QWidget *widget = bind_callable.getWidget();
        PyObject *callable = bind_callable.getCallable();
        QString bind_widget_string = bind_callable.getArgs();
        QString context = bind_callable.getContext();
        QString widget_type = widget->metaObject()->className();
        QWidget *bind_widget = wizard->field_name_to_widget_hash[bind_widget_string];
        QString bind_widget_type = bind_widget->metaObject()->className();
//        if(bind_widget_type == "QListWidget") {
        if(bind_widget_type == "StackedWidget") {
            StackedWidget *stacked_widget = (StackedWidget *) bind_widget;
            connect(stacked_widget, &StackedWidget::currentItemChanged, [=](int row_index) {
//                StackedWidget *stacked = (StackedWidget *) list_widget->parent();
//                PyObject *data = stacked->getData(row_index);
//                qInfo("bind_widget_string: %s", bind_widget_string.toStdString().data());
//                QString template_string;
//                QTextStream(&template_string) << "^$ F{" << bind_widget_string << "}";
//                qInfo("template_string: %s", template_string.toStdString().data());
//                PyObject *arg_tuple = parseArgTemplateString(template_string);
//                if(arg_tuple == NULL) {
//                    PyErr_Print();
//                }
//                qInfo( "row_index: %i", row_index );
                if ( row_index > stacked_widget->getDataList().length() ) {
                    return;
                }
//                PyObject *arg_tuple = PyTuple_New( 1 );
                PyObject *data = stacked_widget->getData( row_index );
//                PyTuple_SetItem( arg_tuple, 0, data );
                PyObject *callable_return = PyObject_CallObject(callable, Py_BuildValue( "(O)", data ));
                if(context == "image" && widget_type == "QLabel") {
                    QLabel *label_widget = (QLabel *) widget;
                    QString system_path = wizard->getPythonInterpreter()->getSystemPath();
                    if(callable_return == NULL) {
                        PyErr_Print();
                    }
//                    qInfo("is string? %d", PyString_Check(callable_return));
//                    qInfo("callable return string: %s", PyString_AsString(callable_return));
                    QString relative_path = PyString_AsString(callable_return);
                    QString image_path = QDir::cleanPath(system_path + QDir::separator() + relative_path);
                    QPixmap label_pixmap(image_path);
                    if(!label_pixmap.isNull() && label_pixmap.height() != 200) {
                        label_pixmap = label_pixmap.scaledToHeight(200);
                    }
                    label_widget->setPixmap(label_pixmap);
                }
                row_index++;
            });
        }
    }

//    PyObject *custom_button_label_obj = PyObject_GetAttrString( pyWizardPageInstance, "custom_button1" );
//    if ( custom_button_label_obj != Py_None ) {
//        QString custom_button_label = PyString_AsString( custom_button_label_obj );
//        wizard->setButtonText( QWizard::CustomButton1, custom_button_label );
//        wizard->setOption(QWizard::HaveCustomButton1, true);
//    } else {
//        wizard->setOption(QWizard::HaveCustomButton1, false);
//    }


}

void InfoPage::initializePage() {
    WizardPage::initializePage();
    foreach(WidgetWithCallableAndArgs wwcaa, page_init_callable_list) {
        QWidget *widget = wwcaa.getWidget();
        QString class_name = widget->metaObject()->className();
        PyObject *callable = wwcaa.getCallable();
        PyObject *argsTuple = NULL;
        QString args = wwcaa.getArgs();

        if(args.startsWith("^$")) {
            argsTuple = parseArgTemplateString(args);
        }
        PyObject *callable_return = PyObject_CallObject(callable, argsTuple);
        PyErr_Print();
        if(!callable_return ||
                !PyList_Check(callable_return)) {
            qInfo("Error calling callable!\n");
            continue;
        }

        if(class_name == "StackedWidget") {
            StackedWidget *stacked_widget = (StackedWidget *) widget;
            stacked_widget->clear();
            Py_ssize_t listSize = PyList_Size(callable_return);
            for(int i = 0;i < listSize;i++) {
                PyObject *pyListItem = PyList_GetItem(callable_return, i);
//                if(PyTuple_Check(pyListItem)) {qInfo("Tuple");}
                if(!pyListItem &&
                        !PyString_Check(pyListItem) && !PyTuple_Check(pyListItem)) {
                    qInfo("Function returned a list that contains at least 1 item that is not a string or tuple.\n");
                    PyErr_Print();
                    PyErr_Clear();
                    continue;
                }
                if(PyString_Check(pyListItem)) {
                    QString list_item = PyString_AsString(pyListItem);
                    stacked_widget->addItem(list_item);
                } else if(PyTuple_Check(pyListItem)) {
                    QString list_item_name = PyString_AsString(PyTuple_GetItem(pyListItem, 0));
                    PyObject *list_item_dict = PyTuple_GetItem(pyListItem, 1);
                    stacked_widget->addItem(list_item_name, list_item_dict);
                }
            }
        }
    }

    foreach (QLabelWithTextTemplate lwtt, page_init_string_parse) {
        QLabel *label = lwtt.getLabel();
        QString parsed_string = parseTextTemplateString(lwtt.getTemplateString());
        label->setText(parsed_string);
    }

}

void InfoPage::cleanupPage() {
    WizardPage::cleanupPage();
    foreach(QObject *child, children()) {
        if(strcmp(child->metaObject()->className(), "StackedWidget") == 0) {
            StackedWidget *stacked = (StackedWidget *) child;
            stacked->clear();
        }
    }
}

QString InfoPage::getMandatoryString(QString fillString, PyObject *pyContentItem) {
    PyObject *mandatory;
    Py_ssize_t itemSize = PyTuple_Size(pyContentItem);
    QString contentType = PyString_AsString(PyTuple_GetItem(pyContentItem, 0));
    if((itemSize < 5 && (contentType.toLower() == "fillhook" || contentType.toLower() == "choose")) ||
            (itemSize < 3 && contentType.toLower() == "field")) {
        return fillString;
    }

    if(contentType.toLower() == "field") {
        mandatory = PyTuple_GetItem(pyContentItem, 2);
    } else {
        mandatory = PyTuple_GetItem(pyContentItem, 4);
    }
    if(!mandatory ||
            mandatory != Py_True) {
        return fillString;
    }

    return fillString.append("*");
}

void InfoPage::addInitCallable(QWidget *widget, PyObject *callable, QString argsTemplate) {
    WidgetWithCallableAndArgs widget_with_callable_and_args(widget, callable, argsTemplate);
    page_init_callable_list.append(widget_with_callable_and_args);
}

void InfoPage::addBindCallable(QWidget *widget, PyObject *callable, QString bindWidget, QString context) {
    BindCallable bind_callable(widget, callable, bindWidget, context);
    bind_callable_list.append(bind_callable);
}

void InfoPage::addParsableString(QLabel *label, QString templateString) {
    QLabelWithTextTemplate widget_with_text_template(label, templateString);
    page_init_string_parse.append(widget_with_text_template);
}

void InfoPage::buttonPushedPDF( PyObject *callable, QString arg_template ) {
    PyObject *args = parseArgTemplateString( arg_template );
    PyObject *callable_return = PyObject_CallObject( callable, args );
    PyErr_Print();
    if(!PyTuple_Check( callable_return ) ) {
        qInfo( "Callable doesn't return a tuple." );
    }
    PyObject *filename_obj = PyTuple_GetItem( callable_return, 0 );
    PyObject *markup_obj = PyTuple_GetItem( callable_return, 1 );
    QString filename = PyString_AsString( filename_obj );
    QString markup = PyString_AsString( markup_obj );
//    qInfo("markup: %s", markup.toStdString().data());
    PDFCreator *pdf = new PDFCreator( parseTextTemplateString( markup ), filename );
    pdf->save();
}

StackedWidget::StackedWidget(QWidget *parent, QString displayType, DatabaseHandler *db, PythonInterpreter *interpreter) :
    QStackedWidget(parent) {
//    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
//    setObjectName(name);
    this->db = db;
    this->interpreter = interpreter;
    if(displayType.toLower().startsWith("listbox")) {
        QListWidget *list_widget = new QListWidget(this);
        addWidget(list_widget);
        connect(list_widget, &QListWidget::currentRowChanged, this, &StackedWidget::currentItemChangedSlot);
    } else {
        QComboBox *combo_box = new QComboBox(this);
        addWidget(combo_box);
        connect(combo_box, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &StackedWidget::currentItemChangedSlot);
    }
}

PyObject *StackedWidget::getData(int index) {
    return m_dataList.at(index);
}

QList<PyObject *> StackedWidget::getDataList() {
    return m_dataList;
}

void StackedWidget::addRowItem(QList<QVariant> *row, int displayColumn) {
    QString displayString = row->at(displayColumn).toString();
//    addItem(displayString, *row);
    addItem(displayString);
}

void StackedWidget::fillComboRow(QString tableName) {
    if(!db->checkPersistentTable(tableName)) {
        qInfo("Table, %s, missing!\n", tableName.toStdString().data());
        return;
    }

    QList<QList<QVariant> *> rows = db->getRows(tableName);
    int displayCol = interpreter->getDisplayCol(tableName);

    for(int i = 0;i < rows.size();i++) {
        QList<QVariant> *row = rows.at(i);
        addRowItem(row, displayCol);
    }
}

void StackedWidget::addItem(QString displayString, PyObject *data, QString toolTip) {
    if(currentIndex() == -1) {
        return;
    }

    m_dataList.append(data);
//    qInfo("StackedWidget ClassName: %s", widget(currentIndex())->metaObject()->className());
    QString class_name = currentWidget()->metaObject()->className();
    if(class_name == "QListWidget") {
        QListWidget *list_widget = (QListWidget *) currentWidget();
        QListWidgetItem *item = new QListWidgetItem(displayString);
        if(toolTip != NULL) {
            item->setToolTip(toolTip);
        }
        list_widget->addItem(item);
    } else {
        QComboBox *combo_widget = (QComboBox *) currentWidget();
        combo_widget->addItem(displayString);
    }
    emit dataListChanged(getDataList());
}

void StackedWidget::addItems(QList<QString> displayStringList) {
    foreach(QString displayString, displayStringList) {
        addItem(displayString);
    }
}

int StackedWidget::getCurrentItemIndex() {
    return m_currentItemIndex;
}

void StackedWidget::setCurrentItemIndex(int itemIndex) {
    m_currentItemIndex = itemIndex;
}

QString StackedWidget::getCurrentItemText() {
    QWidget *widget = currentWidget();
    QString widget_type = widget->metaObject()->className();
    QString current_text;
    if(widget_type == "QListWidget") {
        QListWidget *list_widget = (QListWidget *) widget;
//        current_text = list_widget->currentItem()->text();
//        qInfo("getCurrentItemIndex(): %i", getCurrentItemIndex());
        current_text = list_widget->item(getCurrentItemIndex())->text();
    } else {
        QComboBox *combo_box = (QComboBox *) widget;
//        current_text = combo_box->currentText();
        current_text = combo_box->itemText(getCurrentItemIndex());
    }
    return current_text;
}

void StackedWidget::clear() {
    currentWidget()->blockSignals(true);
    QString class_name = currentWidget()->metaObject()->className();
    if(class_name == "QListWidget") {
        QListWidget *list_widget = (QListWidget *) currentWidget();
        list_widget->clear();
        list_widget->clearSelection();
    } else {
        QComboBox *combo_widget = (QComboBox *) currentWidget();
        combo_widget->clear();
    }
    m_dataList.clear();
    setCurrentItemIndex(0);
    currentWidget()->blockSignals(false);
}

QString StackedWidget::getCurrentToolTip() {
    QString class_name = currentWidget()->metaObject()->className();
    if(class_name == "QListWidget") {
        QListWidget *list_widget = (QListWidget *) currentWidget();
        int index = getCurrentItemIndex();
        int list_widget_size = list_widget->count();
        if(list_widget_size == 1) {
            index = 0;
        }
        return list_widget->item(index)->toolTip();
    } else {
        return QString();
    }
}

QListWidgetItem *StackedWidget::takeItem(int index) {
    QString class_name = currentWidget()->metaObject()->className();
    if(class_name == "QListWidget") {
        QListWidget *list_widget = (QListWidget *) currentWidget();
        QListWidgetItem *taken_item = list_widget->takeItem(index);
        m_dataList.removeAt(index);
        emit list_widget->currentRowChanged(list_widget->currentRow());
        emit dataListChanged(getDataList());
        return taken_item;
    }
}

void StackedWidget::insertItem(int index, QString displayString, PyObject *data, QString toolTip) {
    QString class_name = currentWidget()->metaObject()->className();
    if(class_name == "QListWidget") {
        m_dataList.insert(index, data);
        QListWidget *list_widget = (QListWidget *) currentWidget();
        QListWidgetItem *item = new QListWidgetItem(displayString);
        item->setToolTip(toolTip);
        list_widget->insertItem(index, item);
        emit dataListChanged(getDataList());
    }
}

void StackedWidget::currentItemChangedSlot(int itemIndex) {
    setCurrentItemIndex(itemIndex);
//    m_currentItemIndex = itemIndex;
    emit currentItemChanged(itemIndex);
}

WidgetWithCallableAndArgs::WidgetWithCallableAndArgs(QWidget *widget, PyObject *callable, QString args) {
    setWidget(widget);
    setCallable(callable);
    setArgs(args);
}

QWidget *WidgetWithCallableAndArgs::getWidget() {
    return widget;
}

PyObject *WidgetWithCallableAndArgs::getCallable() {
    return callable;
}

QString WidgetWithCallableAndArgs::getArgs() {
    return args;
}

void WidgetWithCallableAndArgs::setWidget(QWidget *widget) {
    this->widget = widget;
}

void WidgetWithCallableAndArgs::setCallable(PyObject *callable) {
    this->callable = callable;
}

void WidgetWithCallableAndArgs::setArgs(QString args) {
    this->args = args;
}

BindCallable::BindCallable(QWidget *targetWidget, PyObject *callable, QString bindWidgetName, QString context) :
    WidgetWithCallableAndArgs(targetWidget, callable, bindWidgetName){
    setContext(context);
}

QString BindCallable::getContext() {
    return context;
}

void BindCallable::setContext(QString context) {
    this->context = context;
}

DualListSelection::DualListSelection(PyObject *pyWizardPageInstance, QWidget *parent) :
    WizardPage(pyWizardPageInstance, parent) {
    QBoxLayout *mainLayout, *slotsLabelLayout, *listLayout, *buttonLayout;
    Qt::Alignment layoutAlign;
    QString layoutString, fieldName, argString, slotsMethodName, slotsArgs;
    char *methodName;
    PyObject *slotsObject, *slotsMethodObject, *methodObject;
    Py_ssize_t contentSize;

    slotsTextLabel = new QLabel(this);
    slotsTotalLabel = new QLabel(this);

    mainLayout = new QVBoxLayout;
    slotsLabelLayout = new QHBoxLayout;
    slotsLabelLayout->setAlignment(Qt::AlignHCenter);
    slotsLabelLayout->addWidget(slotsTextLabel);
    slotsLabelLayout->addWidget(slotsTotalLabel);
    mainLayout->addLayout(slotsLabelLayout);

    layoutString = PyString_AsString(PyObject_CallMethod(pyWizardPageInstance, (char *) "get_layout", NULL));
    if(layoutString.toLower() == "vertical") {
        listLayout = new QVBoxLayout;
        layoutAlign = Qt::AlignHCenter;
        buttonLayout = new QHBoxLayout;
    } else {
        listLayout = new QHBoxLayout;
        layoutAlign = Qt::AlignVCenter;
        buttonLayout = new QVBoxLayout;
    }
    listLayout->setAlignment(Qt::AlignCenter);

    firstList = new StackedWidget(this, "listbox", wizard->getDb(), wizard->getPythonInterpreter());
    secondList = new StackedWidget(this, "listbox", wizard->getDb(), wizard->getPythonInterpreter());
    addButton = new QPushButton("Add", this);
    removeButton = new QPushButton("Remove", this);
//    addButton = new QPushButton(">>", this);
//    removeButton = new QPushButton("<<", this);

    firstList->setMaximumWidth(175);
    secondList->setMaximumWidth(175);

    listLayout->addWidget(firstList);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    listLayout->addLayout(buttonLayout);
    listLayout->addWidget(secondList);

    contentSize = PyTuple_Size(content);
    fieldName = PyString_AsString(PyTuple_GetItem(content, 0));
    methodName = PyString_AsString(PyTuple_GetItem(content, 1));
    argString = PyString_AsString(PyTuple_GetItem(content, 2));
    methodObject = PyObject_GetAttrString(pyWizardPageInstance, methodName);
    addInitCallable("listfill", methodObject, argString);
    if(contentSize > 3) {
        toolTipField = PyString_AsString(PyTuple_GetItem(content, 3));
    }
    if(contentSize > 4) {
        QString fill_second_list_method = PyString_AsString(PyTuple_GetItem(content, 4));
        QString fill_second_list_args = PyString_AsString(PyTuple_GetItem(content, 5));
        PyObject *fill_second_list_method_object = PyObject_GetAttrString(pyWizardPageInstance, fill_second_list_method.toStdString().data());
        addInitCallable("secondlistfill", fill_second_list_method_object, fill_second_list_args);
    }

    publicRegisterField(fieldName + "Available", firstList, "currentItemIndex", "currentItemChanged");
    publicRegisterField(fieldName, secondList, "currentItemIndex", "currentItemChanged");
    publicRegisterField(fieldName + "List", secondList, "dataList", "dataListChanged");

    slotsObject = PyObject_GetAttrString(pyWizardPageInstance, "slots");
    slotsType = PyString_AsString(PyTuple_GetItem(slotsObject, 0));
    slotsMethodName = PyString_AsString(PyTuple_GetItem(slotsObject, 1));
    slotsArgs = PyString_AsString(PyTuple_GetItem(slotsObject, 2));
    slotsMethodObject = PyObject_GetAttrString(pyWizardPageInstance, slotsMethodName.toStdString().data());
    addInitCallable("slots", slotsMethodObject, slotsArgs);
    if(slotsType.toLower() == "complex") {
        QString add_callback = PyString_AsString(PyTuple_GetItem(slotsObject, 3));
        PyObject *add_args_object = PyTuple_GetItem(slotsObject, 4);
        QString add_args;
        if(add_args_object != Py_None) {
            add_args = PyString_AsString(add_args_object);
//            qInfo("add_args: %s", add_args.toStdString().data());
        }
        PyObject *add_callback_object = PyObject_GetAttrString(pyWizardPageInstance, add_callback.toStdString().data());
        addInitCallable("addCallback", add_callback_object, add_args);
        QString del_callback = PyString_AsString(PyTuple_GetItem(slotsObject, 5));
        PyObject *del_args_object = PyTuple_GetItem(slotsObject, 6);
        QString del_args;
        if(del_args_object != Py_None) {
            del_args = PyString_AsString(del_args_object);
//            qInfo("del_args: %s", del_args.toStdString().data());
        }
        PyObject *del_callback_object = PyObject_GetAttrString(pyWizardPageInstance, del_callback.toStdString().data());
        addInitCallable("delCallback", del_callback_object, del_args);
//        qInfo("%s, %s, %s, %s", add_callback.toStdString().data(), add_args.toStdString().data(), del_callback.toStdString().data(), del_args.toStdString().data());
        QString is_complete_callback = PyString_AsString(PyTuple_GetItem(slotsObject, 7));
        PyObject *is_complete_args_object = PyTuple_GetItem(slotsObject, 8);
        QString is_complete_args;
        if(is_complete_args_object != Py_None) {
            is_complete_args = PyString_AsString(is_complete_args_object);
        }
        PyObject *is_complete_callback_object = PyObject_GetAttrString(pyWizardPageInstance, is_complete_callback.toStdString().data());
        addInitCallable("isCompleteCallback", is_complete_callback_object, is_complete_args);
    } else {
        connect(addButton, &QPushButton::clicked, [=](bool checked) {
            if(slotsTotal > 0 && firstList->getCurrentItemIndex() != -1) {
                int current_index = firstList->getCurrentItemIndex();
                PyObject *data = firstList->getData(current_index);
                QListWidgetItem *current_item = firstList->takeItem(current_index);
                QString current_text = current_item->text();
                secondList->addItem(current_text, data, current_item->toolTip());
                slotsTotal = slotsTotal - 1;
                slotsTotalLabel->setText(QString::number(slotsTotal));
                secondListIndices.append(firstListIndices.takeAt(current_index));
//                qInfo("Index: %d", current_index);
                emit completeChanged();
            }
        });

        connect(removeButton, &QPushButton::clicked, [=](bool checked) {
            if(secondList->getCurrentItemIndex() != -1) {
                int current_index = secondList->getCurrentItemIndex();
                // If the widget is disabled then do nothing
                if(((QListWidget *)secondList->currentWidget())->item(current_index)->flags() == Qt::NoItemFlags) {
                    return;
                }
                int original_index = secondListIndices.at(current_index);
                int new_index;
                if(original_index == 0) {
                    new_index = 0;
                } else {
                    for(int i = 0;i < firstListIndices.length();i++) {
                        int index_item = firstListIndices.at(i);
//                    qInfo("original: %d, item: %d", original_index, index_item);
                        if(original_index > index_item) {
                            new_index = index_item;
                        }
                    }
                    new_index++;
                }
                PyObject *data = secondList->getData(current_index);
                QListWidgetItem *current_item = secondList->takeItem(current_index);
                QString current_text = current_item->text();
                firstList->insertItem(new_index, current_text, data, current_item->toolTip());
                slotsTotal = slotsTotal + 1;
                slotsTotalLabel->setText(QString::number(slotsTotal));
                firstListIndices.insert(new_index, secondListIndices.takeAt(current_index));
//                qInfo("index: %d", new_index);
                emit completeChanged();
            }
        });
    }

    mainLayout->addLayout(listLayout);
    setLayout(mainLayout);
}

void DualListSelection::initializePage() {
//    qInfo("Initialize Page");
    WizardPage::initializePage();
    foreach(DualListCallableAndArgs dlcaa, page_init_callable_list) {
        QString toolTipTextWrap;
        QString type = dlcaa.getType();
//        qInfo("type: %s", type.toStdString().data());
        PyObject *callable = dlcaa.getCallable();
        QString argString = dlcaa.getArgs();

        if(type == "listfill") {
            PyObject *args = parseArgTemplateString(argString);
            PyObject *callable_return = PyObject_CallObject(callable, args);
            PyErr_Print();
            Py_ssize_t return_size = PyList_Size(callable_return);
//            qInfo("list size: %d", return_size);
            for(int i=0;i < return_size;i++) {
                PyObject *tuple = PyList_GetItem(callable_return, i);
                QString item_name = PyString_AsString(PyTuple_GetItem(tuple, 0));
                PyObject *data = PyTuple_GetItem(tuple, 1);
                if(toolTipField != NULL) {
                    QString tool_tip_text;
                    if(toolTipField.toLower() == "$display") {
                        tool_tip_text = item_name;
                    } else {
                        PyObject *tool_tip_object = PyDict_GetItemString(data, toolTipField.toStdString().data());
                        tool_tip_text = PyString_AsString(tool_tip_object);
                    }
                    toolTipTextWrap = "<font>" + tool_tip_text + "</font>";
                }
                firstList->addItem(item_name, data, toolTipTextWrap);
                firstListIndices.append(i);
            }

        } else if(type == "secondlistfill") {
            PyObject *args = parseArgTemplateString(argString);
            PyObject *callable_return = PyObject_CallObject(callable, args);
            PyErr_Print();
            Py_ssize_t return_size = PyList_Size(callable_return);
            for(int i=0;i < return_size;i++) {
                PyObject *tuple = PyList_GetItem(callable_return, i);
                Py_ssize_t tuple_size = PyTuple_Size(tuple);
                QString item_name = PyString_AsString(PyTuple_GetItem(tuple, 0));
                PyObject *data = PyTuple_GetItem(tuple, 1);
                if(toolTipField != NULL) {
                    QString tool_tip_text;
                    if(toolTipField.toLower() == "$display") {
                        tool_tip_text = item_name;
                    } else {
                        PyObject *tool_tip_object = PyDict_GetItemString(data, toolTipField.toStdString().data());
                        tool_tip_text = PyString_AsString(tool_tip_object);
                    }
                    toolTipTextWrap = "<font>" + tool_tip_text + "</font>";
                }
                secondList->addItem(item_name, data, toolTipTextWrap);
                if(tuple_size > 2) {
                    bool remove_from_first_list = PyTuple_GetItem(tuple, 2) == Py_True;
                    bool item_removable = false;
                    if(tuple_size > 3) {
                        item_removable = PyTuple_GetItem(tuple, 3) == Py_True;
                    }
                    if(remove_from_first_list) {
                        QListWidget *list_widget = (QListWidget *)firstList->currentWidget();
//                        qInfo("item_name: %s", item_name.toStdString().data());
//                        qInfo("List Widget: %i", list_widget->count());
                        QList<QListWidgetItem *> list_items = list_widget->findItems(item_name, Qt::MatchExactly);
//                        qInfo("list_items len: %i", list_items.size());
                        QListWidgetItem *list_widget_item = list_items.at(0);
                        int item_index = list_widget->row(list_widget_item);
//                        qInfo("item_index: %i", item_index);
                        firstList->takeItem(item_index);
                        secondListIndices.append(firstListIndices.takeAt(item_index));
                    }
                    if(!item_removable) {
                        ((QListWidget *)secondList->currentWidget())->findItems(item_name, Qt::MatchExactly).at(0)->setFlags(Qt::NoItemFlags);
                    }
                }
            }

        } else if(type == "slots") {
            PyObject *args = parseArgTemplateString(argString);
            PyObject *callable_return = PyObject_CallObject(callable, args);
            PyErr_Print();
            PyObject *slots_text_object = PyTuple_GetItem(callable_return, 0);
            PyObject *slots_total_object = PyTuple_GetItem(callable_return, 1);
            slotsTextLabel->setText(PyString_AsString(slots_text_object));
            slotsTotal = PyInt_AsSsize_t(slots_total_object);
            slotsTotalFloat = (double) slotsTotal;
            slotsTotalLabel->setText(QString::number(slotsTotal));
//            qInfo("slotsTotal: %d", (int) slotsTotal);

        } else if(type == "addCallback") {
            connect(addButton, &QPushButton::clicked, [=](bool checked) {
//               qInfo("Checked: %s", checked ? "true" : "false");
               int current_index = firstList->getCurrentItemIndex();
               PyObject *callable_return;
               if(!argString.isNull()) {
                   PyObject *args = parseArgTemplateString(argString);
//                   QString argBool = "False";
//                   if(PyTuple_Check(args)) {
//                       if(PyDict_Check(PyTuple_GetItem(args, 0))) {
//                           argBool = "True";
//                       }
//                   }
//                   qInfo("Is arg dict?: %s", argBool.toStdString().data());
                   callable_return = PyObject_CallObject(callable, args);
                   PyErr_Print();
               } else {
                   callable_return = PyObject_CallObject(callable, NULL);
                   PyErr_Print();
               }
               if ( callable_return != Py_False ) {
                   // Expects a tuple: (amount to subtract from total, boolean on whether to remove item, (optional) new display string, (optional) bool on wether to replace if it exists in the new list already)
                   PyObject *sub_object = PyTuple_GetItem(callable_return, 0);
                   double subtract_amount = PyFloat_AsDouble(sub_object);
                   bool remove_item = PyTuple_GetItem(callable_return, 1) == Py_True;
                   int tuple_size = PyTuple_Size( callable_return );
                   QString new_display = firstList->getCurrentItemText();
                   if ( tuple_size > 2 ) {
                       PyObject *new_display_obj = PyTuple_GetItem(callable_return, 2);
                       new_display = PyString_AsString( new_display_obj);
                   }

                   if((slotsTotalFloat - subtract_amount) >= 0) {
                       slotsTotalFloat = slotsTotalFloat - subtract_amount;
                       slotsTotalLabel->setText(QString::number(slotsTotalFloat));
                       if(remove_item) {
                           secondListIndices.append(firstListIndices.takeAt(current_index));
                           firstList->takeItem(current_index);
                       }
                       if ( tuple_size > 3 ) {
                           bool replace = PyTuple_GetItem( callable_return, 3 ) == Py_True;
                           if ( replace ) {
                               PyObject *current_data = firstList->getData( firstList->getCurrentItemIndex() );
                               //qInfo( "current index: %i", firstList->getCurrentItemIndex() );
                               if ( secondList->getDataList().contains( current_data ) ) {
                                   int second_list_item_index = secondList->getDataList().indexOf( current_data );
                                   ( ( QListWidget * ) secondList->currentWidget() )->item( second_list_item_index )->setText( new_display );
                               } else {
                                   secondList->addItem( new_display, firstList->getData(current_index), firstList->getCurrentToolTip() );
                               }
                           } else {
                               secondList->addItem( new_display, firstList->getData(current_index), firstList->getCurrentToolTip() );
                           }
                       } else {
                           secondList->addItem( new_display, firstList->getData(current_index), firstList->getCurrentToolTip() );
                       }
                       emit completeChanged();
                   }
               }
            });

        } else if(type == "delCallback") {
            connect(removeButton, &QPushButton::clicked, [=](bool checked) {
//               qInfo("Checked: %s", checked ? "true" : "false");
//               qInfo("secondList Index: %d", secondList->getCurrentItemIndex());
               if(secondList->getCurrentItemIndex() != -1) {
                   PyObject *callable_return;
                   if(!argString.isNull()) {
                       PyObject *args = parseArgTemplateString(argString);
//                       QString argBool = "False";
//                       if(PyTuple_Check(args)) {
//                           if(PyDict_Check(PyTuple_GetItem(args, 0))) {
//                               argBool = "True";
//                           }
//                       }
//                       qInfo("Is arg dict?: %s", argBool.toStdString().data());
                       callable_return = PyObject_CallObject(callable, args);
                       PyErr_Print();
                   } else {
                       callable_return = PyObject_CallObject(callable, NULL);
                       PyErr_Print();
                   }
                   if(callable_return != Py_False) {
                       // Expects a tuple: (amount to add back to total, boolean on whether to replace item back in the first list, (optional) string to change item text instead of removing it)
                       int tuple_size = PyTuple_Size( callable_return );
                       PyObject *sub_object = PyTuple_GetItem(callable_return, 0);
                       double add_amount = PyFloat_AsDouble(sub_object);
                       bool replace_item = PyTuple_GetItem(callable_return, 1) == Py_True;
                       int current_index = secondList->getCurrentItemIndex();
                       PyObject *data = secondList->getData(current_index);
                       QListWidgetItem *current_item;
                       if ( tuple_size > 2 && PyTuple_GetItem( callable_return, 2 ) != Py_None ) {
                           PyObject *new_display_obj = PyTuple_GetItem( callable_return, 2 );
                           QString new_display = PyString_AsString( new_display_obj );
                           current_item = ( ( QListWidget * ) secondList->currentWidget() )->currentItem();
                           current_item->setText( new_display );
                       } else {
                           current_item = secondList->takeItem(current_index);
                       }
                       QString current_text = current_item->text();
                       slotsTotalFloat = slotsTotalFloat + add_amount;
                       slotsTotalLabel->setText(QString::number(slotsTotalFloat));
                       if(replace_item) {
                           int original_index = secondListIndices.at(current_index);
                           int new_index;
                           for(int i = 0;i < firstListIndices.length();i++) {
                               int index_item = firstListIndices.at(i);
                               if(original_index > index_item) {
                                   new_index = index_item;
                               }
                           }
                           new_index++;
                           firstList->insertItem(new_index, current_text, data, current_item->toolTip());
                           firstListIndices.insert(new_index, secondListIndices.takeAt(current_index));
                           emit completeChanged();
                       }
                   }
               }
            });
        }
    }
}

void DualListSelection::cleanupPage() {
    WizardPage::cleanupPage();
    firstList->clear();
    secondList->clear();
    if(slotsType.toLower() == "complex") {
        addButton->disconnect();
        removeButton->disconnect();
    }
}

bool DualListSelection::isComplete() const {
//    qInfo("isComplete Fired!");
    if(slotsTotalFloat != NULL) {
        PyObject_SetAttrString(pyWizardPageInstance, "slots_remaining", PyFloat_FromDouble(slotsTotalFloat));
    } else {
        PyObject_SetAttrString(pyWizardPageInstance, "slots_remaining", PyInt_FromLong(slotsTotal));
    }
    foreach(DualListCallableAndArgs dlcaa, page_init_callable_list) {
        QString type = dlcaa.getType();
        PyObject *callable = dlcaa.getCallable();
        QString argString = dlcaa.getArgs();
        if(type == "isCompleteCallback") {
            if(!argString.isNull()) {
                PyObject *args = parseArgTemplateString(argString);
                return PyObject_CallObject(callable, args) == Py_True;
            } else {
                return PyObject_CallObject(callable, NULL) == Py_True;
            }
        }
    }

    return slotsTotal == 0;
}

void DualListSelection::addInitCallable(QString type, PyObject *callable, QString argsTemplate) {
    DualListCallableAndArgs dual_list_callable_and_args(type, callable, argsTemplate);
    page_init_callable_list.append(dual_list_callable_and_args);
}

DualListCallableAndArgs::DualListCallableAndArgs(QString type, PyObject *callable, QString args) :
    WidgetWithCallableAndArgs(new QWidget, callable, args){
    setType(type);
}

QString DualListCallableAndArgs::getType() {
    return type;
}

void DualListCallableAndArgs::setType(QString type) {
    this->type = type;
}

QLabelWithTextTemplate::QLabelWithTextTemplate(QLabel *label, QString templateString) {
    setLabel(label);
    setTemplateString(templateString);
}

QLabel *QLabelWithTextTemplate::getLabel() {
    return label;
}

QString QLabelWithTextTemplate::getTemplateString() {
    return templateString;
}

void QLabelWithTextTemplate::setLabel(QLabel *label) {
    this->label = label;
}

void QLabelWithTextTemplate::setTemplateString(QString templateString) {
    this->templateString = templateString;
}

ChoosePortraitPage::ChoosePortraitPage(PyObject *pyWizardPageInstance, QWidget *parent) :
    WizardPage( pyWizardPageInstance, parent ) {

    PyObject *py_content;
    Py_ssize_t py_content_size;
    QLabel *portrait_label;
    StackedWidget *selector;
    QBoxLayout *layout;
    QBoxLayout *list_and_button_layout;
    QString field_name, relative_portrait_path;
    QLineEdit *hidden_field;
    QString system_path_string( wizard->getPythonInterpreter()->getSystemPath() );
    QStringList name_filters;
    QPushButton *open_file;

    portrait_label = new QLabel(this);
    hidden_field = new QLineEdit;
    hidden_field->setHidden( true );
    selector = new StackedWidget( this, "listbox", wizard->getDb(), wizard->getPythonInterpreter() );
    int height = 213;

    QPixmap image_pixmap = QPixmap(":/images/noImage.jpg").scaledToHeight( height );
    portrait_label->setPixmap( image_pixmap );

    py_content = PyObject_CallMethod(pyWizardPageInstance, (char *) "get_content", NULL);
    py_content_size = PyTuple_Size( py_content );

    // get_content should return a tuple containing the field name string and a string path for the default folder where the default portraits can be found
    if ( !PyTuple_Check( py_content ) ) {
        PyErr_Print();
        qInfo( "The content attribute of a ChoosePortraitPage class should be a tuple." );
        return;
    }
    PyObject *field_name_obj = PyTuple_GetItem( py_content, 0 );
    PyObject *relative_portrait_path_obj = PyTuple_GetItem( py_content, 1 );
    if ( py_content_size > 2 ) {
        PyObject *style_string_obj = PyTuple_GetItem( py_content, 2 );
        QString style_string = PyString_AsString( style_string_obj );
        portrait_label->setStyleSheet( style_string );
    }
    field_name = PyString_AsString( field_name_obj );
    relative_portrait_path = PyString_AsString( relative_portrait_path_obj );
    QDir portrait_path( QDir( system_path_string ).filePath( relative_portrait_path ) );
    //qInfo( "portrait_path: %s", portrait_path.absolutePath().toStdString().data() );
    name_filters.append( "*.jpg" );
    name_filters.append( "*.jpeg" );
    name_filters.append( "*.png" );
    name_filters.append( "*.gif" );

    portrait_path.setNameFilters( name_filters );
    QDirIterator it( portrait_path, QDirIterator::Subdirectories );
    while ( it.hasNext() ) {
        QString file_path = it.next();
        PyObject *file_path_obj = PyString_FromString( file_path.toStdString().data() );
        QString file_name = it.fileName();
        //qInfo( "file: %s", file_path.toStdString().data() );
        selector->addItem( file_name, file_path_obj );
    }

    connect( selector, &StackedWidget::currentItemChanged, [=] ( int index ) {
        PyObject *data_obj = selector->getData( index );
        QString data = PyString_AsString( data_obj );
        QPixmap portrait( data );
        portrait = portrait.scaledToHeight( height );
        portrait_label->setPixmap( portrait );
        hidden_field->setText( data );
    });

    open_file = new QPushButton( "Browse File" );

    connect( open_file, &QPushButton::clicked, [=] ( bool checked ) {
        QString user_filename = QFileDialog::getOpenFileName( this, "Open Image", QDir::homePath() );
        if ( !user_filename.isNull() ) {
            portrait_label->setPixmap( QPixmap( user_filename ).scaledToHeight( height ) );
            hidden_field->setText( user_filename );
        }
    });

    layout = new QHBoxLayout;
    list_and_button_layout = new QVBoxLayout;
    layout->addWidget( portrait_label );
    list_and_button_layout->addWidget( selector );
    list_and_button_layout->addWidget( open_file );
    layout->addLayout( list_and_button_layout );

    this->setLayout( layout );

    publicRegisterField( field_name, hidden_field );

}
