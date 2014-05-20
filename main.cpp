#include "MainWindow.h"
#include "Dice.h"
#include "ChooseSystemDialog.h"
#include <QApplication>

#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
//    Dice dice;
//    int arb = dice.generateArbitraryRandomInt(5, 9);
//    int d6 = dice.rollDice(6);
//    int threeD6 = dice.rollDice(3, 6);
////    int fourD8 = dice.rollDice("4d8");
//    int fourD8 = dice.rollDice("d69");
//    cout << "Random between 5 and 9: " << arb << endl;
//    cout << "1d6: " << d6 << endl;
//    cout << "3d6: " << threeD6 << endl;
//    cout << "4d8(entered as string): " << fourD8 << endl;
//    cout << argc << " " << argv[0] <<  endl;

//    cout << "Hey!" << endl;
    QApplication app(argc, argv);

    ChooseSystemDialog chooseSystemDialog;
    QStringList systemNames = chooseSystemDialog.getSystemNames();
    QString systemPath;
    if(argc > 1 && systemNames.contains(argv[1]))
    {
        int systemIndex = systemNames.indexOf(argv[1]);
        QString dirPath = chooseSystemDialog.getFolderNames().at(systemIndex);
        systemPath = dirPath;
    }
    else {
        systemPath = chooseSystemDialog.showDialog();
    }
//      cout << "System Folder: " << systemPath.toStdString() << endl;

    MainWindow window(systemPath);
    window.resize(350, 250);
    window.setMinimumSize(350, 250);
    window.center();
    window.setWindowTitle("GM");
    window.show();

    return app.exec();
}
