#include "PDFCreator.h"
#include <QFileDialog>
#include <QPrintPreviewDialog>
#include <QString>

PDFCreator::PDFCreator( QString markup, QString defaultFilename ) {
    this->defaultFilename = defaultFilename;
    QFont font( "Times", 10, QFont::Normal );
    document.setDefaultFont( font );
    //qInfo("%s", markup.toStdString().data());
    document.setHtml( markup );

    printer = new QPrinter( QPrinter::PrinterResolution );
    printer->setOutputFormat( QPrinter::PdfFormat );
    printer->setPaperSize( QPrinter::A4 );
    printer->setOutputFileName( defaultFilename );
    printer->setPageMargins( QMarginsF( 15, 15, 15, 15 ) );
    printer->setColorMode( QPrinter::Color );


    document.setPageSize( printer->pageRect().size() ); //This disables printing the page number

}

void PDFCreator::save() {
    QString filename;

    filename = QFileDialog::getSaveFileName( 0, "Save", defaultFilename, "PDF Files (*.pdf)" );
    if ( filename.isNull() ) {
        return;
    }
    printer->setOutputFileName( filename );

    if ( !filename.isNull() ) {
        document.print( printer );
    }

}

void PDFCreator::preview() {
    QPrintPreviewDialog *preview_dialog = new QPrintPreviewDialog( printer );
    preview_dialog->connect( preview_dialog, &QPrintPreviewDialog::paintRequested, [=] ( QPrinter *prntr ) {
        document.print( prntr );
    });
    preview_dialog->exec();
}
