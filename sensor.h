#ifndef SENSOR_H
#define SENSOR_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QString>
#include <QCoreApplication>

#include "config.h"

class sensor
{
public:
    sensor(float a, float b);
    float GetDistance();
private:
    QSerialPort serial;
    float COEFA, COEFB;
};

#endif // SENSOR_H
