#include "sensor.h"
#include <QDebug>
#include <QThread>



sensor::sensor(float a, float b): COEFA(a), COEFB(b)
{
#if SENSOR

    //Recherche du port série du capteur
    bool PortTrouve = false;
    QList<QSerialPortInfo> liste = QSerialPortInfo::availablePorts();
    for (int i = 0;i<liste.size();i++){
        if (QString(liste[i].serialNumber()).contains("A104WP3S"))
        {
            serial.setPortName(liste[i].portName());
            qDebug() << "Nom        : " << liste[i].portName();
            qDebug() << "Manufacturer: " << liste[i].manufacturer();
            qDebug() << "Busy: " << liste[i].isBusy() << endl;
            PortTrouve = true;
        }
    }

    if(!PortTrouve) //Si le port n'est pas trouvé on quitte le programme
    {
        qDebug() << "Capteur non trouvé." << endl;
         exit(EXIT_PORT_SENSOR_NON_TROUVE);
    }

    serial.open(QIODevice::ReadWrite);
    serial.setBaudRate(QSerialPort::Baud115200);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    QThread::msleep(1500);
    serial.waitForReadyRead(50);
    serial.waitForReadyRead(50);
    serial.waitForReadyRead(50);
    serial.clear();
#endif
}

float sensor::GetDistance()
{
#if SENSOR
    QByteArray Bresponse;
    QString response;


    if (serial.isOpen() && serial.isWritable())
    {
        //Préparation de la trame
        QByteArray ba = QString("DIST").toLocal8Bit(); //conversion QString -> QByteArray
        ba.append("\r");
        //serial.clear();
        serial.write(ba);
        //serial.flush();
        serial.waitForBytesWritten(-1);
        //while(serial.flush());

        qDebug() << "Demande de distance" << endl;

        //Attente datas disponibles
        //while(!serial.waitForReadyRead(10));

        //Récupération de la distance (valeur  = 0 to 1023)
        while(serial.waitForReadyRead(500))
        {
            //serial.flush();
            //serial.waitForReadyRead(10);
            Bresponse = serial.readAll();
            response += QString(Bresponse);
            qDebug() << "response : " << response << endl;
            if(response.contains('\n'))
                break;
        }

        //Calcul de la distance en mm
        float dist = COEFA * response.toFloat() + COEFB;


        if(response.toFloat() < 10)//Si obstacle trop proche du capteur
            return 0;
        else if(response.toFloat() > 1020)//Si obstacle trop loin du capteur
            return 1000;
        else
            return dist;
    }
    else
        qDebug() << "ERREUR de Port COM" << endl;

    return 0;
#else
    return 45.45;
#endif
}
