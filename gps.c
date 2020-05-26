#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <setjmp.h>
#include "gps.h"

void inputGPSFrame(gps * data) {
    printf("Saisissez une trame GPS :\n");
    (*data).message = NULL;
    size_t taille = 0;
    getline(&((*data).message), &taille, stdin);
}

void readGPSFrameFromFile(gps * data, char *fileName) {
    FILE* file;
    file = fopen(fileName, "r");
    if (file == NULL) {
        printf("Impossible d'ouvrir le fichier \"%s\" en lecture.\n", fileName);
    } else {
        (*data).message = NULL;
        size_t taille = 0;
        getline(&((*data).message), &taille, file);
    }
}

void syntaxCheck(const gps * data, jmp_buf resPt) {
    int res = NOT_GPGGA_FRAME;  //initialisation du résultat
    int error;                  //variable d'erreur
    regex_t regex;              //initialisation variable d'expression régulière

    //copie le message
    const char * str_tested = (*data).message;
    //chaine représentant les attentes du message
    const char * str_req = "^\\$[G-][P-]GGA,[[:digit:]]{6}[\\.]?[[:digit:]]{0,4},[[:digit:]]{1,5}[\\.]?[[:digit:]]*,[NS],[[:digit:]]{1,5}[\\.]?[[:digit:]]*,[EW],[0-8],[01][[:digit:]],[[:digit:]]*[\\.]?[[:digit:]]{0,4},[[:digit:]]*[\\.]?[[:digit:]]{0,4},M?,[-]?[[:digit:]]*[\\.]?[[:digit:]]{0,4},M?,[[:digit:]]?[\\.]?[[:digit:]]*,[[:digit:]]{0,4}\\*?[[:digit:]]{0,2}";
    //parametrage de la variable d'expression régulière en fonction de la syntaxe spécifiée (compilation) --> renvoie 0 si pas d'erreur
    error = regcomp (&regex, str_req, REG_NOSUB | REG_EXTENDED);
    
    if (error == 0) {   //s'il n'y a pas d'erreur --> str_req correspond bien à ce qui est attendue pour une expression régulière
        int match;      //initilaisation de la variable de correspondance
        match = regexec (&regex, str_tested, 0, NULL, 0);   //renvoie 1 si le message ne correspond pas à la syntaxe attendue
        regfree(&regex);    //libère la mémoire allouée lors de la compilation

        if (match != REG_NOMATCH) {     //si le message à la bonne syntaxe
            
            //vérification de la validité du checksum s'il existe
            char *p = strchr(str_tested, '*');
            if (p != NULL) {
                int ck = strtol(p+1, NULL, 16);
                printf("%d\n", checksum(str_tested));
                if (ck != checksum(str_tested))
                    longjmp(resPt, INCORRECT_CHECKSUM);
            }
        } else {
            longjmp(resPt, NOT_GPGGA_FRAME);
        }
    }
}

void extractFields(gps * data, jmp_buf resPt) {
    const char *message = (*data).message;      //trame saisie par l'utilisateur
    char *p1;       //pointeur sur la première ','
    char *p2;       //pointeur sur la seconde ','
    int len;        //longueur de l'information

    int i = 0;
    p1 = strchr(message, ',');                              //recherche de la première occurence de ','
    len = (p1-message > 19) ? 19 : p1-message;              //calcul de la longueur
    strncpy((*data).extractedMsg[i], message, len);         //enregistre l'information dans la structure de données
    (*data).extractedMsg[i][len]='\0';
    i++;

    while(NULL != (p2 = strchr(p1+1, ','))) {               //parcours des différentes 'informations'
        len = (p2-p1-1 > 19) ? 19 : p2-p1-1;                //calcul de la longueur de l'information
        strncpy((*data).extractedMsg[i], p1+1, len);        //enregistre de l'informmation dans la structure de données
        (*data).extractedMsg[i][len]='\0';
        p1 = p2;
        i++;
    }
    strcpy((*data).extractedMsg[i], p1+1);

    double lat = strtod((*data).extractedMsg[2], NULL);
    double lon = strtod((*data).extractedMsg[4], NULL);
    double time = strtod((*data).extractedMsg[1], NULL);

    if (time < 0 || time > 240000 || (int) time % 10000 > 6000 || (int) time % 100 > 60) {
        longjmp(resPt, TIME_OUT_OF_BOUNDS);
    }

    if (lat < 0 || lat > 9000 || lon < 0 || lon > 18000) {
        longjmp(resPt, POS_OUT_OF_BOUNDS);
    } else {
        (*data).position.latitude.orientation = (*data).extractedMsg[3][0];
        (*data).position.longitude.orientation = (*data).extractedMsg[5][0];
    }
}

void convertTime(gps * data) {
    const double time = strtod((*data).extractedMsg[1], NULL);

    int h = (*data).time.hours = ((int) time)/10000;
	int m = (*data).time.minutes = ((int) time - h*10000)/100;
	(*data).time.seconds = (int) time - h*10000 - m*100;
}

void convertPosition(gps * data) {
    const double lat = strtod((*data).extractedMsg[2], NULL);
    const double lon = strtod((*data).extractedMsg[4], NULL);

    int lat1 = (*data).position.latitude.degrees = ((int) lat)/100;
	(*data).position.latitude.minutes = ((int) lat - lat1*100);
	(*data).position.latitude.seconds = (lat - (int) lat) * 60;
    
	int lon1 = (*data).position.longitude.degrees = ((int) lon)/100;
	(*data).position.longitude.minutes = ((int) lon - lon1*100);
	(*data).position.longitude.seconds = (lon - (int) lon) * 60;
}

void displayTime(const gps * data) {
    const struct extractedTime time = (*data).time;

    printf("Heure : %dh%dm%ds\n", time.hours, time.minutes, (int) time.seconds);
}

void displayPosition(const gps * data) {
    const struct extractedPosition pos = (*data).position;

    printf("Position : %d°%d'%.4g\" %c ; %d°%d'%.4g\" %c\n",
        pos.latitude.degrees, pos.latitude.minutes, pos.latitude.seconds, pos.latitude.orientation,
        pos.longitude.degrees, pos.longitude.minutes, pos.longitude.seconds, pos.longitude.orientation);
}

void saveInFile(const gps * data, char *fileName) {
    const struct extractedTime time = (*data).time;
    const struct extractedPosition pos = (*data).position;
    FILE* file;
    file = fopen(fileName, "w");

    if (file == NULL) {
        printf("Impossible d'ouvrir le fichier \"%s\" en écriture.\n", fileName);
    } else {
        fprintf(file, "Heure : %dh%dm%ds\n", time.hours, time.minutes, (int) time.seconds);
        fprintf(file, "Position : %d°%d'%.4g\" %c ; %d°%d'%.4g\" %c\n",
            pos.latitude.degrees, pos.latitude.minutes, pos.latitude.seconds, pos.latitude.orientation,
            pos.longitude.degrees, pos.longitude.minutes, pos.longitude.seconds, pos.longitude.orientation);
        printf("\nLes données ont été enregistrées dans le fichier\033[0;34m %s\033[0m.\n", fileName);
        fclose(file);
    }
}

int checksum(const char *s) {
    s++;
    int c = 0;
    while (*s != '*') {
        c ^= *s++;
    }
    return c;
}