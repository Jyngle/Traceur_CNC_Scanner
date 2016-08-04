
#ifndef SERIAL_H
#define SERIAL_H


#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QCoreApplication>
#include "config.h"

#define VITESSE 15000
#define TOLERANCE 0.5

class grbl
{

public:
    grbl(float OFFSETX, float OFFSETY);
    void MoveTo(float X, float Y);


private:
    bool IsAt(float X, float Y);
    QSerialPort serial;

    float _OFFSETX, _OFFSETY;
};


#endif // SERIAL_H
