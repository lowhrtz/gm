#include "Dice.h"
#include <QRegExp>
#include <QTime>

#include <iostream>
using namespace std;

Dice::Dice() {
    divider = 'd';

    // Create seed for qrand()
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());
}

int Dice::generateArbitraryRandomInt(int low, int high) {
    return qrand() % ((high + 1) - low) + low;
}

int Dice::rollDice(int sides) {
    int low = 1;
    int high = sides;
    return generateArbitraryRandomInt(low, high);
//    return qrand() % ((high + 1) - low) + low;
}

int Dice::rollDice(int number, int sides) {
    int dieResultTotal = 0;

    for(int i = 0 ; i < number ; i++)
    {
        int dieResult = rollDice(sides);
        dieResultTotal += dieResult;
    }

    return dieResultTotal;
}

int Dice::rollDice(QString diceString) {
    QRegExp singleDiceFormat = QRegExp("^[d]{0,1}[0-9]+$");
    QRegExp diceFormat = QRegExp("^[0-9]+[d][0-9]+$");

    if(diceString.contains(singleDiceFormat))
    {
        diceString.replace("d", "");
        cout << "diceString.toInt(): " << diceString.toInt() << endl;
        return rollDice(diceString.toInt());
    }
    else if (diceString.contains(diceFormat)) {
        const char *dividerChar = "d";
        QChar divider(*dividerChar);
        int dividerIndex = diceString.indexOf(divider);
        QString numberString = diceString.left(dividerIndex);
        QString sidesString = diceString.right(dividerIndex);
        int number = numberString.toInt();
        int sides = sidesString.toInt();
//        cout << "number: " << numberString.toStdString() << endl;
//        cout << "sides: " << sidesString.toStdString() << endl;
        return rollDice(number, sides);
    }

    return -1;
}

int Dice::rollDiceDropLowest(int number, int sides) {
    return rollDiceDropNLowest(number, sides, 1);
}

int Dice::rollDiceDropNLowest(QString diceString, int dropRolls) {
    QRegExp singleDiceFormat = QRegExp("^[d]{0,1}[0-9]+$");
    QRegExp diceFormat = QRegExp("^[0-9]+[d][0-9]+$");

    if(diceString.contains(singleDiceFormat)) {
        diceString.replace((QString)divider, "");
        if(dropRolls > 0) return -1;
        return rollDiceDropNLowest(1, diceString.toInt(), dropRolls);
    } else if(diceString.contains(diceFormat)) {
//        const char *dividerChar = "d";
//        QChar divider('d');
        int dividerIndex = diceString.indexOf(divider);
        QString numberString = diceString.left(dividerIndex);
        QString sidesString = diceString.right(dividerIndex);
        int number = numberString.toInt();
        int sides = sidesString.toInt();
        return rollDiceDropNLowest(number, sides, dropRolls);
    }

    return -1;
}

int Dice::rollDiceDropNLowest(int number, int sides, int dropRolls) {
    if(number <= dropRolls) return 0;
    int roll = 0;
    QList<int> rollList;

    for(int i = 0;i < number;i++) {
        rollList.append(rollDice(sides));
//        cout << "roll: " << rollList.at(i) << " ";
    }
//    cout << endl;

    qSort(rollList.begin(), rollList.end());
    for(int i = 0;i < rollList.size();i++) {
        if(i >= dropRolls) roll += rollList.at(i);
    }

    return roll;
}

int Dice::rollDiceDropLowest(QString diceString) {
    return rollDiceDropNLowest(diceString, 1);
}
