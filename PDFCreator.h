#ifndef PDFCREATOR_H
#define PDFCREATOR_H

#include <QPrinter>
#include <QTextDocument>



class PDFCreator {
public:
    PDFCreator(QString markup);
    void save();

private:
    QPrinter *printer;
    QTextDocument document;

};

#endif // PDFCREATOR_H
