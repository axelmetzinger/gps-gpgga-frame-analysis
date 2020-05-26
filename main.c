#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include "gps.h"

void main() {
    jmp_buf resPt;
    Exception pb;
    gps data;

    int choice;
    char buffer[1024];
    char * err;

    do {
        buffer[0] = '\0';
        err = NULL;
        printf("\n\nChoisissez le moyen de saisie de trame GPS - GPGGA :\n");
        printf("0 - Lecture fichier gpgga_frame.txt\n");
        printf("1 - Entrer de la trame dans la console\n\n");
        printf("Saisissez le numéro conrespondant :\n");
        fgets(buffer,1024,stdin);
        choice = (int) strtod(buffer, &err);
        if ((*err!='\0') && (*err!='\n')) {
            choice = -1;
            printf("\n Attention caractère invalide dans le nombre fourni\n");
        }
    } while (choice != 0 && choice != 1);

    if (choice == 0) {      //choix = lecture de la trame via fichier
        pb = setjmp(resPt);

        switch (pb) {
            case OK:                    //si la trame saisie est valide
                readGPSFrameFromFile(&data, "gpgga_frame.txt");
                syntaxCheck(&data, resPt);
                extractFields(&data, resPt);
                break;
            case INCORRECT_CHECKSUM:    //si latrame saisie est invalide pour cause de défaut au checksum (parité)
                printf("\nLa trame saisie est bien une trame GPGGA.\n");
                printf("Trame lue : %s\n\n", data.message);
                printf("\033[0;31mErreur lors du contrôle de parité (checksum).\033[0m Veuillez réessayer.\n\n\n");
                break;
            case NOT_GPGGA_FRAME:       //si la trame ne suit pas la syntaxe GPGGA
                printf("Trame lue : %s\n\n", data.message);
                printf("\n\033[0;31mErreur, la trame saisie n'est pas une trame GPGGA.\033[0m Veuillez réessayer.\n\n\n");
                break;
            case TIME_OUT_OF_BOUNDS:    //si l'heure de la trame n'est pas au format 24H
                printf("\nLa trame saisie est bien une trame GPGGA.\n");
                printf("Trame lue : %s\n\n", data.message);
                printf("\n\033[0;31mErreur, les valeurs d'heure saisies dans la trame dépassent les valeurs autorisées.\033[0m\n");
                printf("Les heures doivent être comprises entre 0 et 24, les minutes et les secondes doivent elles être comprises entre 0 et 60.\n\n\n");
                break;
            case POS_OUT_OF_BOUNDS:     //si la trame contient des positions non valables
                printf("\nLa trame saisie est bien une trame GPGGA.\n");
                printf("Trame lue : %s\n\n", data.message);
                printf("\n\033[0;31mErreur, les valeurs de positions saisies dans la trame dépassent les valeurs autorisées.\033[0m\n");
                printf("La latitude s'étend de 0 à 90 degrés tandis que la longitude est comprise entre 0 et 180 degrés.\n\n\n");
                break;
            default:
                printf("\n\033[0;31mErreur inconnue.\033[0m\n");
                printf("Veuillez réessayer.\n\n\n");
        }
    } else {        //saisie de la trame par l'utilisateur
        pb = setjmp(resPt);
    
        //trame lue dans un fichier
        //saisie de la trame à la main
        switch (pb) {
            case OK:                    //si la trame saisie est valide
                //s
                inputGPSFrame(&data);
                syntaxCheck(&data, resPt);
                extractFields(&data, resPt);
                break;
            case INCORRECT_CHECKSUM:    //si latrame saisie est invalide pour cause de défaut au checksum (parité)
                printf("\nLa trame saisie est bien une trame GPGGA.\n");
                printf("Trame lue : %s\n\n", data.message);
                printf("\033[0;31mErreur lors du contrôle de parité (checksum). \033[0m Veuillez réessayer.\n\n\n");
                inputGPSFrame(&data);
                syntaxCheck(&data, resPt);
                extractFields(&data, resPt);
                pb = OK;
                break;
            case NOT_GPGGA_FRAME:       //si la trame ne suit pas la syntaxe GPGGA
                printf("Trame lue : %s\n\n", data.message);
                printf("\n\033[0;31mErreur, la trame saisie n'est pas une trame GPGGA.\033[0m Veuillez réessayer.\n\n\n");
                inputGPSFrame(&data);
                syntaxCheck(&data, resPt);
                extractFields(&data, resPt);
                pb = OK;
                break;
            case TIME_OUT_OF_BOUNDS:    //si l'heure de la trame n'est pas au format H24
                printf("\nLa trame saisie est bien une trame GPGGA.\n");
                printf("Trame lue : %s\n\n", data.message);
                printf("\n\033[0;31mErreur, les valeurs d'heure saisies dans la trame dépassent les valeurs autorisées.\033[0m\n");
                printf("Les heures doivent être comprises entre 0 et 24, les minutes et les secondes doivent elles être comprises entre 0 et 60.\n");
                printf("Veuillez réessayer.\n\n\n");
                inputGPSFrame(&data);
                syntaxCheck(&data, resPt);
                extractFields(&data, resPt);
                pb = OK;
                break;
            case POS_OUT_OF_BOUNDS:     //si la trame contient des positions non valables
                printf("\nLa trame saisie est bien une trame GPGGA.\n");
                printf("Trame lue : %s\n\n", data.message);
                printf("\n\033[0;31mErreur, les valeurs de positions saisies dans la trame dépassent les valeurs autorisées.\033[0m\n");
                printf("La latitude s'étend de 0 à 90 degrés tandis que la longitude est comprise entre 0 et 180 degrés.\n");
                printf("Veuillez réessayer.\n\n\n");
                inputGPSFrame(&data);
                syntaxCheck(&data, resPt);
                extractFields(&data, resPt);
                pb = OK;
                break;
            default:
                printf("\n\033[0;31mErreur inconnue.\033[0m\n");
                printf("Veuillez réessayer.\n\n\n");
                inputGPSFrame(&data);
                syntaxCheck(&data, resPt);
                extractFields(&data, resPt);
                pb = OK;
        }
    }
    if (pb == OK) {
        printf("\nLa trame saisie est bien une trame GPGGA.\n\n");
        
        convertTime(&data);
        convertPosition(&data);
        displayTime(&data);
        displayPosition(&data);
        saveInFile(&data, "result.txt");
    }
    
}