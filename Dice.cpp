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
    QRegExp complexFormat = QRegExp("^\\(?[0-9]+[d][0-9]+(\\s*\\+[0-9]+\\s*)?\\)?(\\s*[x|×]\\s*[0-9]+)?$");

    if(diceString.contains(singleDiceFormat)) {
        diceString.replace("d", "");
//        cout << "diceString.toInt(): " << diceString.toInt() << endl;
        return rollDice(diceString.toInt());

    } else if (diceString.contains(diceFormat)) {
        const char *dividerChar = "d";
        QChar divider(*dividerChar);
//        int dividerIndex = diceString.indexOf(divider);
        QStringList diceStringList = diceString.split(divider);
        QString numberString = diceStringList[0];
        QString sidesString = diceStringList[1];
        int number = numberString.toInt();
        int sides = sidesString.toInt();
//        cout << "number: " << numberString.toStdString() << endl;
//        cout << "sides: " << sidesString.toStdString() << endl;
//        qInfo("diceString: %s", diceString.toStdString().data());
//        qInfo("sidesString: %s", sidesString.toStdString().data());
        return rollDice(number, sides);

    } else if(diceString.contains(complexFormat)) {
        QString number_string;
        QString sides_string;
        QString add_string;
        QString mul_string;
        QStringList split_on_d = diceString.split("d");
        number_string = split_on_d[0];
        number_string = number_string.replace("(", "");
        QString after_d = split_on_d[1];
        after_d = after_d.replace(")", "");
        for(int i = 0;i < after_d.length();i++) {
            if(after_d[i].isDigit()) {
                sides_string.append(after_d[i]);
            } else {
                break;
            }
        }
        if(after_d.contains("+")) {
            QStringList split_on_plus = after_d.split("+");
//            sides_string = split_on_plus[0];
            QString after_plus = split_on_plus[1];
            after_plus = after_plus.trimmed();
            for(int i = 0;i < after_plus.length();i++) {
                if(after_plus[i].isDigit()) {
                    add_string.append(after_plus[i]);
                } else {
                    break;
                }
            }
        }
        if(after_d.contains("x") || after_d.contains("×")) {
            QStringList split_on_x = after_d.split(QRegExp("[x|×]"));
            QString after_x = split_on_x[1];
            mul_string = after_x.trimmed();
        }
        int base_roll = rollDice(number_string.toInt(), sides_string.toInt());
        int total = base_roll;
        if(!add_string.isNull()) {
            total += add_string.toInt();
        }
        if(!mul_string.isNull()) {
            total *= mul_string.toInt();
        }
        return total;
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
