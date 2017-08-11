#include "CustomWidgets.h"

ImageWidget::ImageWidget( QString base64_data, QWidget *parent )
    : QLabel( parent ) {
    setData( base64_data );
}

QString ImageWidget::getData() {
    return data;
}

void ImageWidget::setData( QString base64_data ) {
    ImageWidget::setImageWithBase64( this, base64_data );
    this->data = base64_data;
}

void ImageWidget::setImageWithBase64(QLabel *image_widget, QString base64_string) {
    QByteArray ba = QByteArray::fromBase64( base64_string.toStdString().data() );
    QImage image = QImage::fromData( ba );
    QPixmap pixmap = QPixmap( QPixmap::fromImage( image ) );
    if( pixmap.height() > 200 ) {
        pixmap = pixmap.scaledToHeight( 200 );
    }
    image_widget->setPixmap( pixmap );
}
