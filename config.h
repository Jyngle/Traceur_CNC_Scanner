#ifndef CONFIG_H
#define CONFIG_H




#define GRBL                                0
#define SENSOR                              0
#define BT_SART_PAUSE                       0
#define BT_STOP                             0


#define ERROR "/err.nc"
#define PATH_PARAM "/config/paramScanner.txt"


#define EXIT_PRB_OUVERTURE_FICHIER_PARAM_SCAN    2
#define EXIT_PORT_GRBL_NON_TROUVE                3
#define EXIT_PORT_SENSOR_NON_TROUVE              4
#define EXIT_PARAMETRE_MANQUANT_SCAN             5
#define EXIT_TROP_DE_POINTS_A_MESURER            6
#define EXIT_STOP_PAR_UTILISATEUR                7
#define EXIT_PRB_PORT_COM_GRBL                   8
#define EXIT_PRB_OUVERTURE_FICHIER_PARAM_SENSOR  9
#define EXIT_PARAMETRE_MANQUANT_SENSOR           10



#endif // CONFIG_H
