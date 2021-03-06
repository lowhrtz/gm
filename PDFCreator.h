#ifndef PDFCREATOR_H
#define PDFCREATOR_H

#include <QPrinter>
#include <QTextDocument>



class PDFCreator {
public:
    PDFCreator( QString markup , QString defaultFilename );
    void save();
    void preview();

private:
    QString defaultFilename;
    QPrinter *printer;
    QTextDocument document;

};

#endif // PDFCREATOR_H
