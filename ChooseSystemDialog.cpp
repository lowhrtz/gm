#include "ChooseSystemDialog.h"
#include <QDir>
#include <QIcon>
#include <QTextStream>

#include <iostream>
using namespace std;

ChooseSystemDialog::ChooseSystemDialog(QWidget *parent) :
    QInputDialog(parent)
{
    QIcon icon(":/images/icon.png");
    setWindowIcon(icon);

    QDir searchDir("systems");
    if(!searchDir.exists())
    {
        cout << "Missing Systems Directory" << endl;
        exit(1);
    }
    searchDir.setFilter(QDir::Dirs);
    searchDir.setSorting(QDir::Name);
    QStringList nameFilters;
    nameFilters << "*";
//    searchDir.setNameFilters(nameFilters);
    QStringList directoryList = searchDir.entryList(nameFilters);
    for(int i = 0 ; i < directoryList.size() ; i++)
    {
        QString dirName = directoryList.at(i);
//        cout << "dirName: " << dirName.toStdString() << endl;
        if(dirName != "." && dirName != "..")
        {
//            cout << "Settings Dir: " << directoryList.at(i).toStdString() << endl;
            QDir subDir(searchDir);
            subDir.cd(dirName);
            subDir.setFilter(QDir::Files);
            searchDir.setSorting(QDir::Name);
            QStringList fileFilter("*.py");
            QStringList fileList = subDir.entryList(fileFilter);
            for(int j = 0 ; j < fileList.size() ; j++)
            {
                QString fileName = fileList.at(j);
                if(fileName == "SystemSettings.py") {
//                    cout << "File: " << fileName.toStdString() << " Abs Path: " << subDir.absoluteFilePath(fileName).toStdString() << endl;
                    QFile file(subDir.absoluteFilePath(fileName));
                    file.open(QIODevice::ReadOnly | QIODevice::Text);
                    QTextStream fileStream(&file);
                    while(!fileStream.atEnd())
                    {
                        QString line = fileStream.readLine();
                        if(line.contains("systemName"))
                        {
                            int equalIndex = line.indexOf("=");
                            QString systemName = line.remove(0, equalIndex + 1);
                            systemName = systemName.trimmed().remove(QRegExp("['\"]"));
//                            cout << systemName.toStdString() << endl;
                            folderNames << subDir.absolutePath();
                            systemNames << systemName;
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }
}

QString ChooseSystemDialog::showDialog()
{
//    QStringList testList;
//    testList << "One" << "Two" << "Three" << "Four";
    bool ok;
    QString result = getItem(this, "Choose A System", "RPG Systems Defined", systemNames, 0, false, &ok);
    if(!ok || result.isEmpty())
    {
        exit(0);
    }

    int resultIndex = systemNames.indexOf(result);
    QString dirPath = folderNames.at(resultIndex);

    return dirPath;
}

QStringList ChooseSystemDialog::getFolderNames()
{
    return folderNames;
}

QStringList ChooseSystemDialog::getSystemNames()
{
    return systemNames;
}
