#include "Dialogs.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

YesNoDialog::YesNoDialog(QString title, QString message, QWidget *parent) :
    QDialog(parent) {
    setWindowTitle(title);
    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *messageLabel = new QLabel(message, this);
    QPushButton *yes_button = new QPushButton("Yes", this);
    QPushButton *no_button = new QPushButton("No", this);
    QDialogButtonBox *button_frame = new QDialogButtonBox(this);
    layout->addWidget(messageLabel);
    yes_button->setDefault(true);
    button_frame->addButton(yes_button, QDialogButtonBox::AcceptRole);
    button_frame->addButton(no_button, QDialogButtonBox::RejectRole);
    connect(button_frame, &QDialogButtonBox::accepted, this, &YesNoDialog::accept);
    connect(button_frame, &QDialogButtonBox::rejected, this, &YesNoDialog::reject);
    layout->addWidget(button_frame);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(layout);
}

PopupDialog::PopupDialog(QString title, QString message, QWidget *parent) :
    QDialog(parent) {
    setWindowTitle(title);
    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *messageLabel = new QLabel(message, this);
    QPushButton *ok_button = new QPushButton("OK", this);
    QDialogButtonBox *button_box = new QDialogButtonBox(this);
    layout->addWidget(messageLabel);
    ok_button->setDefault(true);
    button_box->addButton(ok_button, QDialogButtonBox::AcceptRole);
    connect(button_box, &QDialogButtonBox::accepted, this, &PopupDialog::accept);
    layout->addWidget(button_box);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    setLayout(layout);
}

EntryDialog::EntryDialog(QString title, EntryWidgetType type, QVariant *value, QWidget *parent , QString image_data)
    : QDialog( parent ) {
    QVBoxLayout *layout;
    QDialogButtonBox *button_frame;
    QPushButton *ok_button, *cancel_button;

    this->value = value;
    setWindowTitle( title );

    layout = new QVBoxLayout;

    if ( type == LINE_EDIT ) {
        entryWidget = new QLineEdit( this );


    } else if ( type == TEXT_EDIT ) {
        entryWidget = new QTextEdit( this );

    } else if ( type == SPIN_BOX ) {
        entryWidget = new QSpinBox( this );
        ( (QSpinBox *) entryWidget )->setRange( -1000000000, 1000000000 );

    } else if ( type == IMAGE ) {
        QPushButton *image_button = new QPushButton( "Choose Image", this );
        connect( image_button, &QPushButton::clicked, [=] ( bool clicked ) {
            filename = QFileDialog::getOpenFileName( this, "Open Image", QDir::homePath() );
            if ( !filename.isNull() ) {
                ( (ImageWidget *) entryWidget )->setPixmap( QPixmap( filename ).scaledToHeight( 200 ) );
            }
        });

        entryWidget = new ImageWidget( image_data );
        layout->addWidget( image_button );
    }

    layout->addWidget( entryWidget, 0, Qt::AlignCenter );

    button_frame = new QDialogButtonBox( this );
    ok_button = new QPushButton( "OK", this );
    cancel_button = new QPushButton( "Cancel", this );
    ok_button->setDefault( true );
    button_frame->addButton( ok_button, QDialogButtonBox::AcceptRole );
    button_frame->addButton( cancel_button, QDialogButtonBox::RejectRole );
    connect( button_frame, &QDialogButtonBox::accepted, [=] () {

        if ( type == LINE_EDIT ) {
            this->value->setValue( ( (QLineEdit *) entryWidget )->text() );


        } else if ( type == TEXT_EDIT ) {
            this->value->setValue( ( (QTextEdit *) entryWidget )->toPlainText() );

        } else if ( type == SPIN_BOX ) {
            this->value->setValue( ( (QSpinBox *) entryWidget )->value() );

        } else if ( type == IMAGE ) {
            this->value->setValue( filename );
        }

        accept();

    });

    connect( button_frame, &QDialogButtonBox::rejected, [=] () {
        value->clear();
        reject();
    });

    layout->addWidget( button_frame );
    layout->setSizeConstraint( QLayout::SetFixedSize );
    setLayout( layout );
}

DualListDialog::DualListDialog( QString title, PyObject *owned_item_list_obj, PyObject *action_data_obj, PyObject *fields_obj, QWidget *parent )
    : QDialog( parent ) {
    setWindowTitle( title );

    QVBoxLayout *layout = new QVBoxLayout;
    DualListWidget *dual_list = new DualListWidget( owned_item_list_obj, action_data_obj, fields_obj, this );
    layout->addWidget( dual_list );

    layout->setSizeConstraint( QLayout::SetFixedSize );
    setLayout( layout );
}
