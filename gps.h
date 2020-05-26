/*
* Purpose: Ce mini-projet en langage C a été réalisé dans le cadre du module M2101 de notre DUT Informatique.
* Celui-ci porte sur l'analyse de trames GPS suivant la norme NMEA 0183 afin de créer une
* "application de géolocalisation" en ligne de commande (CLI).
* 
* Authors: Zhao JIN ; Axel METZINGER
* Language: C
*/

/* ---------------------------- */
/* --- DEFINITION DES TYPES --- */
/* ---------------------------- */

//Enumération contenant les "exceptions"
typedef enum {OK, NOT_GPGGA_FRAME, INCORRECT_CHECKSUM, POS_OUT_OF_BOUNDS, TIME_OUT_OF_BOUNDS} Exception;
//Orientation latitudinale
typedef enum {N='N', S='S'} latOr;
//Orientation longitudinale
typedef enum {E='E', W='W'} lonOr;

//Structure contenant le temps formaté
struct extractedTime {
    int hours;      //hh
    int minutes;    //mm
    double seconds; //ss.ssss
};

//Structure contenant la position formaté
struct extractedPosition {
    struct {
        int degrees;        //xx°
        int minutes;        //xx'
        double seconds;     //xx.xxxx"
        latOr orientation;  //N ou S
    } latitude;
    struct {
        int degrees;        //yy°
        int minutes;        //yy'
        double seconds;     //yy.yyyy"
        lonOr orientation;  //E ou W
    } longitude;
};

//Structure contenant les différentes données du GPS
typedef struct {
    char * message;                 //message entré par l'utilisateur
    char extractedMsg[15][20];      //tableau contenant chaque "paquet d'information" de la trame entrée

    struct extractedPosition position;
    struct extractedTime time;
} gps;


/* ------------------------------- */
/* --- SIGNATURE DES FONCTIONS --- */
/* ------------------------------- */

//Permet la saisie d'une trame GPS
void inputGPSFrame(gps * data);

//Permet de lire un trame GPGGA dans un fichier
void readGPSFrameFromFile(gps * data, char *fileName);

//Vérifie la syntaxe de la trame saisie, utilisation des expressions régulières
void syntaxCheck(const gps * data, jmp_buf resPt);

//Extrait les differents champs et les stocke dans la structure GPS données en entrée
void extractFields(gps * data, jmp_buf resPt); 

//Converti l'heure saisie (HHMMSS.SS) au format suivant : HHhMMmSSs
void convertTime(gps * data);

//Converti la position saisie (xxyy.zzzz') dans le système sexagésimal (DMS) : xx°yy'zz.zz"
void convertPosition(gps * data);

//Affiche l'heure
void displayTime(const gps * data);

//Affiche la position
void displayPosition(const gps * data);

//Enregistre les données d'heure et de position dans un fichier texte
void saveInFile(const gps * data, char *fileName);

//Renvoie la somme de contrôle de parité (checksum)
int checksum(const char *s);