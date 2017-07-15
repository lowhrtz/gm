#include "PDFCreator.h"
#include <QFileDialog>
#include <QString>

PDFCreator::PDFCreator( QString markup, QString defaultFilename ) {
    QFont font( "Times", 10, QFont::Normal );
    document.setDefaultFont( font );
//    qInfo("%s", markup.toStdString().data());
    document.setHtml( markup );

    printer = new QPrinter( QPrinter::PrinterResolution );
    printer->setOutputFormat( QPrinter::PdfFormat );
    printer->setPaperSize( QPrinter::A4 );
    QString filename = QFileDialog::getSaveFileName( 0, "Save", defaultFilename, "PDF Files (*.pdf)" );
    printer->setOutputFileName( filename );
    printer->setPageMargins( QMarginsF( 15, 15, 15, 15 ) );


    document.setPageSize( printer->pageRect().size() ); //This disables printing the page number

}

void PDFCreator::save() {
    document.print( printer );
}
