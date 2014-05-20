#ifndef DICE_H
#define DICE_H

#include <QString>

class Dice
{
public:
    Dice();
    int generateArbitraryRandomInt(int low, int high);
    int rollDice(int sides);
    int rollDice(int number, int sides);
    int rollDice(QString diceString);
    int rollDiceDropLowest(int number, int sides);
    int rollDiceDropNLowest(int number, int sides, int dropRolls);
    int rollDiceDropLowest(QString diceString);
    int rollDiceDropNLowest(QString diceString, int dropRolls);

private:
    QChar divider;
};

#endif // DICE_H
