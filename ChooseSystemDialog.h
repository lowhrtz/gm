#ifndef CHOOSESYSTEMDIALOG_H
#define CHOOSESYSTEMDIALOG_H

#include <QInputDialog>



class ChooseSystemDialog : public QInputDialog
{
    Q_OBJECT
public:
    explicit ChooseSystemDialog(QWidget *parent = 0);
    QString showDialog();
    QStringList getFolderNames();
    QStringList getSystemNames();

signals:
    
public slots:
    
private:
    QStringList folderNames;
    QStringList systemNames;
};

#endif // CHOOSESYSTEMDIALOG_H
