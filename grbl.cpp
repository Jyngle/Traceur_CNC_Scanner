#include "grbl.h"
#include <QDebug>
#include <QThread>
#include <QString>

 grbl::grbl(float OFFSETX, float OFFSETY): _OFFSETX(OFFSETX), _OFFSETY(OFFSETY)
{
#if GRBL
    bool PortTrouve = false;

    //Recherche du port série de GRBL
    QList<QSerialPortInfo> liste = QSerialPortInfo::availablePorts();
    for (int i = 0;i<liste.size();i++){
        if (QString(liste[i].manufacturer()).contains("Arduino"))
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
        qDebug() << "GRBL non trouvé." << endl;
        stream_log << "Carte Arduino avec GRBL non trouvée" << endl;
        errFile.close();
        exit(EXIT_PORT_GRBL_NON_TROUVE);
    }

    serial.open(QIODevice::ReadWrite);
    serial.setBaudRate(QSerialPort::Baud115200);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    QThread::msleep(2000);//Attente lancement GRBL
    serial.waitForReadyRead(50);//Nécessaire
    serial.waitForReadyRead(50);
    serial.waitForReadyRead(50);
    serial.clear();
#endif
}





void grbl::MoveTo(float X, float Y)
{
#if GRBL
    if (serial.isOpen() && serial.isWritable())
    {
        //Préparation de la trame
        QByteArray ba = QString("G01 X" + QString::number(X + _OFFSETX) + " Y" + QString::number(Y + _OFFSETY) + " F" + QString::number(VITESSE)).toLocal8Bit(); //conversion QString -> QByteArray
        ba.append("\r");
        //serial.flush();

        //Envoi de la trame à GRBL
        serial.write(ba);
        serial.waitForBytesWritten(-1);
        serial.flush();

    }
    else
    {
        qDebug() << "ERREUR de Port COM" << endl;
        stream_log << "Erreur de port COM (Arduino GRBL)." << endl;
        errFile.close();
        exit(EXIT_PRB_PORT_COM_GRBL);

    }

    while(!IsAt(X + _OFFSETX,Y + _OFFSETY))//Attente que la tête arrive au point demmandé avant de rendre la main
        QThread::msleep(100);//test toute les 100ms
#else
    qDebug() << "G01 X" << QString::number(X) << " Y" << QString::number(Y) << " F15000" << endl;
#endif
}

bool grbl::IsAt(float X, float Y)
{
#if GRBL
    QByteArray Bresponse;
    QString response;
    QString ValXYZ;
    QStringList Vals;
    float Xm, Ym;
    if (serial.isOpen() && serial.isWritable())
    {
        //Préparation de la trame
        QByteArray ba = QString("?").toLocal8Bit(); //conversion QString -> QByteArray
        serial.flush();
        //Envoi de la trame à GRBL
        serial.write(ba);
        //serial.flush();
        serial.waitForBytesWritten(-1);
        qDebug() << "Demande de position outil a GRBL" << endl;



        while(serial.waitForReadyRead(500))//Attente tant que le dernier caratère n'est pas reçu
        {
            Bresponse = serial.readAll();
            response += QString(Bresponse);
            if(response.contains(">"))
                break;
        }

        //Récupérartion des positions de la tête (de l'outil)
        ValXYZ = QString(response.split("WPos:")[1]).split(">")[0];
        Vals = ValXYZ.split(",");

        qDebug() << "Xm :" << Vals[0] << "    Ym :" << Vals[1] << endl;
        Xm = Vals[0].toFloat();
        Ym = Vals[1].toFloat();


        //Si la tête est arrivée au point X Y à plus moins la télérance return true sinon false
        if(Xm > X-TOLERANCE && Xm < X+TOLERANCE && Ym > Y-TOLERANCE && Ym < Y+TOLERANCE)
        {
            qDebug() << "Position atteinte" << endl;
            return true;
        }
        else
            return false;


    }
    else
    {
        qDebug() << "ERREUR de Port COM" << endl;
        stream_log << "Erreur de port COM (Arduino GRBL)." << endl;
        errFile.close();
        exit(EXIT_PRB_PORT_COM_GRBL);
    }

    return false;
#else
        return true;
#endif
}











































