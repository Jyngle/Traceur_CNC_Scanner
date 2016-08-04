#include <QCoreApplication>

#include <string.h>
#include <QString>
#include <QFile>
#include <QDebug>
#include <QCoreApplication>
#include <QThread>
#include <QList>
#include <math.h>

#include "config.h"

#if BT_SART_PAUSE || BT_STOP
#include <wiringPi.h>
#endif

#include "grbl.h"
#include "sensor.h"


#if BT_SART_PAUSE
bool PauseCycle = false;
bool StopCycle = false;

void StartPause()
{
    PauseCycle = !PauseCycle;
}
#endif

#if BT_STOP
void Stop()
{
    exit(EXIT_STOP_PAR_UTILISATEUR);
}
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if(argc < 2) //Si pas de nom de fichier quitter
        return 1;

    float Zref = 0;

    //Ouverture du fichier contenant les paramètres
    QString FileNameOut = QCoreApplication::applicationDirPath() +  "/" + argv[1];
    QFile out(FileNameOut);
    QTextStream stream_out(&out);

    if(out.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug() << "Fichier ouvert !" << endl;
    }
    else
    {
        qDebug() << "Probleme a la lecture du fichier";
        return EXIT_PRB_OUVERTURE_FICHIER_PARAM;
    }


    float X1 = 0, Y1 = 0, X2 = 0, Y2 = 0, XREF = 0, YREF = 0, PAS = 0, OFFSETX = 0, OFFSETY = 0, COEFA = 0, COEFB = 0;
    int PIN_PAUSE = 0, PIN_STOP = 0;
    QString ligne;
    int test = 0;

    //Recherche des paramètres dans le fichier
    while (!stream_out.atEnd())
    {
        ligne = stream_out.readLine();

        if(ligne.contains("X1"))
        {
            X1 = QString(ligne.split('=')[1]).toFloat();
            test++;
        }

        if(ligne.contains("Y1"))
        {
            Y1 = QString(ligne.split('=')[1]).toFloat();
            test++;
        }

        if(ligne.contains("X2"))
        {
            X2 = QString(ligne.split('=')[1]).toFloat();
            test++;
        }

        if(ligne.contains("Y2"))
        {
            Y2 = QString(ligne.split('=')[1]).toFloat();
            test++;
        }

        if(ligne.contains("XREF"))
        {
            XREF = QString(ligne.split('=')[1]).toFloat();
            test++;
        }

        if(ligne.contains("YREF"))
        {
            YREF = QString(ligne.split('=')[1]).toFloat();
            test++;
        }

        if(ligne.contains("PAS"))
        {
            PAS = QString(ligne.split('=')[1]).toFloat();
            test++;
        }

        if(ligne.contains("OFFSETX"))
        {
            OFFSETX = QString(ligne.split('=')[1]).toFloat();
            test++;
        }

        if(ligne.contains("OFFSETY"))
        {
            OFFSETY = QString(ligne.split('=')[1]).toFloat();
            test++;
        }

        if(ligne.contains("COEFA"))
        {
            COEFA = QString(ligne.split('=')[1]).toFloat();
            test++;
        }

        if(ligne.contains("COEFB"))
        {
            COEFB = QString(ligne.split('=')[1]).toFloat();
            test++;
        }

        if(ligne.contains("PIN_PAUSE"))
        {
            PIN_PAUSE = QString(ligne.split('=')[1]).toInt();
            test++;
        }

        if(ligne.contains("PIN_STOP"))
        {
            PIN_STOP = QString(ligne.split('=')[1]).toInt();
            test++;
        }
    }

    if(test != 13)//Si tous les paramètres ne sont pas trouvés quitter
        return EXIT_PARAMETRE_MANQUANT;

#if BT_SART_PAUSE || BT_STOP
    wiringPiSetup(); //Initialise les IOs du RPI
#endif

#if BT_SART_PAUSE
    pinMode(PIN_PAUSE, INPUT);
    wiringPiISR(PIN_PAUSE,INT_EDGE_FALLING,StartPause);
#endif

#if BT_STOP
    pinMode(PIN_STOP, INPUT);
    wiringPiISR(PIN_STOP,INT_EDGE_FALLING,Stop);
#endif

    grbl Grbl(OFFSETX, OFFSETY);//instantiation de GRBL avec les offsets de position du capteur par rapport à l'outil
    sensor Sensor(COEFA, COEFB);//instantiation de Sensor avec les coefs de la droite de sortie du capteur (tension -> distance)

    Grbl.MoveTo(XREF, YREF);//Déplacement de la tête vers la position de référence
    QThread::msleep(300);
    Zref = Sensor.GetDistance();//Mesure de la hauteur de référence
    QThread::msleep(100);


    Grbl.MoveTo(X1, Y1);//Positionnement de la tête vers la première mesure a effectuer
    QThread::msleep(100);

    //Inversion des X1, X2 et Y1, Y2 dans le cas ou le rectangle est à l'envers
    float X1p = ((X1 > X2)?X2:X1);
    float X2p = ((X1 < X2)?X2:X1);
    float Y1p = ((Y1 < Y2)?Y2:Y1);
    float Y2p = ((Y1 > Y2)?Y2:Y1);

    //Calcul du nombre de répétition maximum possible en X et en Y en fonction du PAS
    const int iMax = (X2p - X1p) / PAS;
    const int jMax = (Y1p - Y2p) / PAS;

    //Calcul du nombre de points de mesure, si trop grand = quitter programme
    //(risque de remplire la mémoire vive)
    if((iMax+1) * (jMax+1) > 360000)
        return EXIT_TROP_DE_POINTS_A_MESURER;

    //Déclaration du tableau des altitudes relatives au point de référence
    float altitude[iMax+1][jMax+1];
    float dist;
    int i = 0, j = 0;
    QStringList gg;
float TestX, TestY;
    //Début du scan
    for(i=0; i<=iMax; i++)//i = déplacement en X
    {
        if(j==0)
        {
            for(j=0; j <= jMax; j++)//j = déplacement en Y
            {
                TestX = X1p + (i*PAS);
                TestY = Y1p - (j*PAS);
                Grbl.MoveTo(X1p + (i*PAS), Y1p - (j*PAS));//Déplacement vers le nouveau point de mesure
                dist = Sensor.GetDistance();
                qDebug() << QString::number(i) + " " + QString::number(j) + "    X:" + QString::number(X1p + (i*PAS)) + " Y:" + QString::number(Y1p - (j*PAS));
                if(dist == 0 || dist == 1000)
                    altitude[i][j] = 0;
                else
                    altitude[i][j] = dist - Zref;//Prise de la mesure (relative au point de référence)
#if BT_SART_PAUSE
                while(PauseCycle);//Si une pause est demmandée par l'utilisateur
#endif
            }
            j = jMax;
        }
        else
        {
            for(j=jMax; j >= 0; j--)
            {
                TestX = X1p + (i*PAS);
                TestY = Y1p - (j*PAS);
                Grbl.MoveTo(X1p + (i*PAS), Y1p - ((j)*PAS));
                dist = Sensor.GetDistance();
                qDebug() << QString::number(i) + " " + QString::number(j) + "    X:" + QString::number(X1p + (i*PAS)) + " Y:" + QString::number(Y1p - (j*PAS));
                if(dist == 0 || dist == 1000)
                    altitude[i][j] = 0;
                else
                    altitude[i][j] = dist - Zref;//Prise de la mesure (relative au point de référence)
#if BT_SART_PAUSE
                while(PauseCycle);
#endif
            }
            j = 0;
        }
    }

    Grbl.MoveTo(-OFFSETX,-OFFSETY);

    stream_out << endl << endl;
    j = jMax;

    //Sérialisation du tableau des altitudes dans le fichier
    for(i=0; i<=iMax; i++)
    {
        if(j==0)
        {
            for(j=0; j <= jMax; j++)//j = déplacement en Y
                stream_out << QString::number(i) << " " << QString::number(j) << " " << QString::number(altitude[i][jMax - j]) << endl;
            j = jMax;
        }
        else
        {
            for(j=jMax; j >= 0; j--)
              stream_out << QString::number(i) << " " << QString::number(jMax - j) << " " << QString::number(altitude[i][j]) << endl;
            j = 0;
        }
    }




    out.close();

    return EXIT_SUCCESS;
}
