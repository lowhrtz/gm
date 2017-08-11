#ifndef CUSTOMWIDGETS_H
#define CUSTOMWIDGETS_H

#include <QLabel>
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

#endif // CUSTOMWIDGETS_H

