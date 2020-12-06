/*
    En gros, je me connecte au lecteur puis à la carte, je lui demande son ID unique,
    je fais la vérification de sécurité, j'envoie le code PIN, et si tout est bon,
    une trame est envoyée au serveur à l'adresse '86.238.113.11' (avant '192.168.241.88') et au port '1234'.
    
    En utilisateur, tout s'exécute à la chaîne.
    En admin, on peut choisir quelles trames APDU envoyer.
    
    Fonctions pour 'envoi réseau' et 'écriture sur la carte' pas encore implémentées.
*/

#include "detector.h"

using namespace std;


Detector::Detector(int admin)
{
    cout << " - Debut Constructeur - " << endl;
    etape = 1; //Etape 1
    
    if (admin == 0)
        scardErr = SCardEstablishContext(SCARD_SCOPE_USER, nullptr, nullptr, &Context);
    else
        scardErr = SCardEstablishContext(SCARD_SCOPE_SYSTEM, nullptr, nullptr, &Context);
    
    cout << "Fonction SCardEstablishContext - " << (admin == 0 ? "User" : "Admin") << " - " << (scardErr != SCARD_S_SUCCESS ? "Erreur" : "Ok") << endl;
    cout << " - Fin Constructeur - " << endl << endl;
}


//SCARD_S_SUCCESS vaut 0.


Detector::~Detector()
{
    cout << " - Debut Destructeur - " << endl;
    if (etape == 2)
    {
        SCardFreeMemory(Context, mszReaders); // Memory block to be released
        cout << "Fonction SCardFreeMemory - " << (scardErr != SCARD_S_SUCCESS ? "Erreur " : "Ok");
        if (scardErr != SCARD_S_SUCCESS) {cout << scardErr << endl;}
        cout << endl;
    }
    
    if (etape >= 4) // Si SCardConnect marche :
    {
        scardErr = SCardDisconnect(hCard, SCARD_LEAVE_CARD);
        cout << "Fonction SCardDisconnect - " << (scardErr != SCARD_S_SUCCESS ? "Erreur " : "Ok");
        if (scardErr != SCARD_S_SUCCESS) {cout << scardErr << endl;}
        cout << endl;
    }
    
    if (etape >= 2) // Enlever le contexte en dernier
    {
        scardErr = SCardReleaseContext(Context);
        cout << "Fonction SCardReleaseContext - " << (scardErr != SCARD_S_SUCCESS ? "Erreur " : "Ok");
        if (scardErr != SCARD_S_SUCCESS) {cout << scardErr << endl;}
        cout << endl;
    }
    cout << " - Fin Destructeur - " << endl << endl;
}

bool Detector::Communication()
{
    etape ++; //Etape 2
    mszReaders = nullptr; dwReaders = SCARD_AUTOALLOCATE;
    scardErr = SCardListReaders(Context, nullptr, (LPTSTR)&mszReaders, &dwReaders);
    
    cout << "Fonction SCardListReaders - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok");
    if (scardErr == SCARD_E_NO_READERS_AVAILABLE) {cout << "pas de lecteur detecte."; return 1;}
    if (scardErr == SCARD_E_READER_UNAVAILABLE)   {cout << "lecteur actuellement indisponible."; return 1;}
    cout << endl << endl;
    
    
    mszCurrentReader = nullptr;
    unsigned int idx = 0;
    mszCurrentReader = mszReaders;
    cout << "Liste des lecteurs :" << endl;
    
    while ('\0' != *mszCurrentReader)
    {
        idx++;
        cout << idx << ". - Lecteur " << mszCurrentReader << endl; // Display the value.
        //cout << "Test " << mszCurrentReader[0] <<  mszCurrentReader[1] <<  mszCurrentReader[2] << endl;
        
        // Advance to the next value.
        mszCurrentReader = mszCurrentReader + strlen(mszCurrentReader) + 1; // sizeof ou strlen ?
    }
    
    
    //cout << "Veuillez choisir un numero de lecteur :" << endl;
    //unsigned int uiReader = getchar() - 0x30;
    
    mszCurrentReader = mszReaders;
    
    /*if (uiReader > 1) // Choix de lecteur (bug et dispensable)
    {
        cout << "Test - " << uiReader << endl;
        for (idx=1; idx<uiReader; idx++)
        {
            mszCurrentReader = mszCurrentReader + strlen(mszCurrentReader) + 1;
        }
    }*/
    cout << "Dernier lecteur (" << idx << ") selectionne auto : " << mszCurrentReader << endl << endl;
    
    
    etape ++; //Etape 3
    hCard = 0; dwActiveProtocol = 0;
    scardErr = SCardConnect(Context, mszCurrentReader, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0|SCARD_PROTOCOL_T1,  (SCARDHANDLE*)&hCard, &dwActiveProtocol);
    
    cout << "Fonction SCardConnect - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok");
    if (scardErr == SCARD_E_NOT_READY) {cout << "le lecteur n'est pas pret" << endl; return 0;}
    if (scardErr != SCARD_E_NOT_READY && scardErr != SCARD_S_SUCCESS) {cout << scardErr << endl; return 0;}
    cout << endl << endl;
    
    
    etape ++; //Etape 4
    LPBYTE pbAtr = nullptr; // Recepteur de l'ATR (Answer To Reset)
    DWORD dwAtr = 0; // Taille ATR en octets
    // Récupération taille de l'ATR (réponse de mise sous tension, identification de la carte)
    scardErr = SCardGetAttrib(hCard, SCARD_ATTR_ATR_STRING,	nullptr, &dwAtr);
    
    cout << "Fonction SCardGetAttrib 1 - Taille ATR - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok");
    if (scardErr == ERROR_NOT_SUPPORTED) {cout << "erreur inconnue" << endl; return 0;}
    if (scardErr != ERROR_NOT_SUPPORTED && scardErr != SCARD_S_SUCCESS) {cout << scardErr << endl; return 0;}
    cout << endl;
    
    cout << "Allocation memoire ATR (Answer To Reset)" << endl;
    pbAtr = new byte[dwAtr];
    // Récupération de l'ATR
    scardErr = SCardGetAttrib(hCard, SCARD_ATTR_ATR_STRING,	pbAtr, &dwAtr);
    
    cout << "Fonction SCardGetAttrib 2 - Contenu ATR - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok");
    if (scardErr == ERROR_NOT_SUPPORTED) {cout << "erreur inconnue" << endl; return 0;}
    if (scardErr != ERROR_NOT_SUPPORTED && scardErr != SCARD_S_SUCCESS) {cout << scardErr << endl; return 0;}
    
    cout << endl << endl << "Taille ATR : " << dwAtr << " octets" << endl;
    cout << "Lecture hexa ATR : ";
    for (int i=0; i< (long int)dwAtr; i++)
    {
        printf("%02X ", pbAtr[i]); // Réponse héxa
    }
    
    ATRCheck(pbAtr);
    
    cout << "Desallocation memoire ATR (Answer To Reset)";
    delete [] pbAtr;
    cout << endl << endl;
    
    
    etape ++; //Etape 5
    scardErr = SCardBeginTransaction(hCard); // Empêche à partir d'ici les autres applications d'utiliser la carte
    
    cout << "Fonction SCardBeginTransaction - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok");
    if (scardErr != SCARD_S_SUCCESS) {cout << scardErr << endl; return 0;}
    cout << endl << endl;
    
    
    // Début de l'envoi des commandes à la carte et la reception de ses réponses, nommées trames APDU
    // (Application Data Protocol Units), selon un mode Maître/Esclave, la carte attendant les commandes.
    // ID des fichiers pouvant être sélectionnés : FF 00 -> FF 07.
    
    etape ++; //Etape 6
    SCARD_IO_REQUEST ioSend; // Protocol header structure for the instruction
    ioSend.dwProtocol = dwActiveProtocol;
    ioSend.cbPciLength = sizeof(SCARD_IO_REQUEST);
    
    
    //byte abSelectFile[] = {0x00, 0xa4, 0x08, 0x0c, 0x04, 0x11, 0x00, 0x60, 0x00}; // Code incorrect ! L'en tête ne correspond pas.
    
    byte abTest[] = {0x80, 0xa4, 0x00, 0x00, 0x02, 0xFF, 0x00};
    byte abResponse[2]={0}; // Réponse de la carte, sur 2 octets. Utilisé uniquement quand la carte doit renvoyer "90 00" et pas une chaîne plus longue.
    DWORD dwLenR = sizeof(abResponse); // Taille de abResponse (2) ou d'une autre chaine 
    
    
    scardErr = SCardTransmit(hCard, &ioSend, abTest, sizeof(abTest), nullptr, abResponse, &dwLenR);
    cout << "Fonction SCardTransmit (Test Selection Fichier) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
    if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << "code " << scardErr << endl; return 0;}
    cout << endl; Reponse(abResponse, dwLenR); cout << endl << endl;
    
    etape ++; //Etape 7
    return 1;
}


void Detector::ListError(BYTE * Rep, LONG CardErr, int CHOIX)
{
    // Erreurs héxa spécifiques
    
    if ( (Rep[0] == 0x69) && (Rep[1] == 0x82) && (CHOIX == 5) )
    {
        cout << "Erreur : Impossible si la carte est en User Stage, sinon donner le code IC d'abord.";
    }
    
    if ( (Rep[0] == 0x69) && (Rep[1] == 0x82) && (CHOIX == 4) )
    {
        cout << "Erreur : Donner le code PIN d'abord.";
    }
    
    if ( (Rep[0] == 0x69) && (Rep[1] == 0x85) && ((CHOIX == 3) || (CHOIX == 4)) )
    {
        cout << "Erreur : S'authentifier d'abord.";
    }
    
    if ( (Rep[0] == 0x69) && (Rep[1] == 0x85) && (CHOIX == 2) )
    {
        cout << "Erreur : Debuter une session immediatement avant.";
    }
    
    if ( (Rep[0] == 0x63) && (CHOIX == 2) )
    {
        int HexC = 0xC0, Nmb = Rep[1] - HexC; // 0xCi est donné, avec i le nombre d'essais restants, on retire donc le C devant
        
        cout << "Erreur : Code donne incorrect. "; printf("%02X essais restants.", Nmb);
    }
    
    if ( (Rep[0] == 0x6A) && (CHOIX == 8) )
    {
        cout << "Erreur : Le fichier n'existe pas." << endl;
    }
    
    
    // Erreurs héxa générales
    
    if ( (Rep[0] == 0x6A) && (Rep[1] == 0x86) )
    {
        cout << "Erreur : P1 ou P2 invalide.";
    }
    
    
    // Erreurs connexion
    
    if (CardErr == -2146434968) // SmartCard reset, réétablir la communication
    {
        cout << "La carte est pas contente. Redemarrer.";
    }
    
    if (CardErr == -2146435064) // Erreur -2146435064 : Buffer trop petit
    {
        cout << "Le buffer donne est trop petit. En mettre un plus grand";
    }
    
    
    cout << endl;
}

void Detector::ATRCheck(BYTE * ATR)
{
    // Est-ce que le numéro de série est dans l'ATR ?
    
    cout << endl << "Etape du cycle de vie : ";
    printf("%02X - ", ATR[16]); // 17ème octet
    cout << (ATR[16] == 0x01 ? "Manufacturing" : (ATR[16] == 0x02 ? "Personilzation" : "User") ) << " Stage" << endl; //Double Super cout !
    
    cout << "Numero de carte : " << (ATR[5] == 0x41 ? "ACOS" : "??") /*<< (ATR[6] == 0x01 ? "v1." : "??")*/;
    if (ATR[7] == 0x38) {cout << "3.8";} else if (ATR[7] == 0x20) {cout << "2.0";} else if (ATR[7] == 0x10) {cout << "1.0";}  else {cout << "??";} cout << endl;
    
    
}

void Detector::Reponse(BYTE * Rep, DWORD Len) // Rep : chaîne reçue, Len : taille de la réponse
{
    cout << "Reponse hexa de la carte : ";
    for (int i=0; i< (long int)Len; i++)
    {
        printf("%02X ", Rep[i]); // Réponse héxa - 90 00 est le seul code de 2 octets indiquant un bon fonctionnement
    }
}


bool Detector::TrameSecur()
{
    // Début session : génération code, le code est aléatoire et est dans les 8 premiers octets reçus, les 2 derniers sont 90 00.
    
    SCARD_IO_REQUEST ioSend; // Protocol header structure for the instruction
    ioSend.dwProtocol = dwActiveProtocol;
    ioSend.cbPciLength = sizeof(SCARD_IO_REQUEST);
    
    //byte abResponse[2]={0}; // Réponse de la carte, sur 2 octets. Utilisé uniquement quand la carte doit renvoyer "90 00" et pas une chaîne plus longue.
    
    // Codes pour authentification
    byte CodeSession[10]={0};
    //byte CodeRNDTerm[10]={0};
    
    
    DWORD dwLenR = sizeof(CodeSession); // Taille de abResponse (2) ou d'une autre chaine
    byte abSession[] = {0x80, 0x84, 0x00, 0x00, 0x08};
    scardErr = SCardTransmit(hCard, &ioSend, abSession, sizeof(abSession), nullptr, CodeSession, &dwLenR); // avec pin
    
    cout << "Fonction SCardTransmit (Code Session) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur " : "Ok ");
    if (scardErr != SCARD_S_SUCCESS || CodeSession[8] != 0x90) {cout << "code " << scardErr << endl; ListError(CodeSession, scardErr, choix);} // 90 00 à la fin
    cout << endl; Reponse(CodeSession, dwLenR);
    
    
    // Authentification (immédiatement après session) : utilisation code puis envoyer obligatoirement GET RESPONSE {0x80, 0xC0, 0x00, 0x00, 0x08}
    
    byte abAuthentif[21] = {0x80, 0x82, 0x00, 0x00, 0x10}; // {0x80, 0x82, 0x00, 0x00, 0x10, DES(RNDcard, #Kt}, RNDterminal} donc 5+8+8 = 21 octets
    
    for (int i=0; i<8; i++) // Récupération du code dans les 8 premiers octets
    {
        abAuthentif[5+i] = CodeSession[i];
    }
    
    // Il faut chiffrer les informations, mais je ne sais pas comment... Je préfère éviter de bloquer la carte.
    
    /*
        scardErr = SCardTransmit(hCard, &ioSend, abAuthentif, sizeof(abAuthentif), nullptr, abResponse, &dwLenR);
        
        cout << "Fonction SCardTransmit (Authentification) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << "code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        
        byte abGetResponse[] = {0x80, 0xC0, 0x00, 0x00, 0x08};
        scardErr = SCardTransmit(hCard, &ioSend, abGetResponse, sizeof(abGetResponse), nullptr, abResponse, &dwLenR); // Get Response juste après l'envoi du code
        
        cout << "Fonction SCardTransmit (Get Response apres Authentification) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << "code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        if(abResponse[0] == 0x90)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    */
    
    return 0;
}

bool Detector::TrameEnvoiPIN()
{
    // Envoi PIN (après authentification)
    
    SCARD_IO_REQUEST ioSend; // Protocol header structure for the instruction
    ioSend.dwProtocol = dwActiveProtocol;
    ioSend.cbPciLength = sizeof(SCARD_IO_REQUEST);
    
    byte abResponse[2]={0}; // Réponse de la carte, sur 2 octets. Utilisé uniquement quand la carte doit renvoyer "90 00" et pas une chaîne plus longue.
    DWORD dwLenR = sizeof(abResponse); // Taille de abResponse (2) ou d'une autre chaine
    
    byte abVerifyPin[] = {0x80, 0x20, 0x06, 0x00, 0x08, 0x41, 0x43, 0x4F, 0x53, 0x54, 0x45, 0x53, 0x54}; // ASCII en hxadécimal : {0x00, 0x20, 0x00, 0x02, 0x08, 'A', 'C', 'O', 'S', 'T', 'E','S', 'T'}
    scardErr = SCardTransmit(hCard, &ioSend, abVerifyPin, sizeof(abVerifyPin), nullptr, abResponse, &dwLenR); // avec pin
    
    cout << "Fonction SCardTransmit (Code PIN) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur " : "Ok ");
    if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << "code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
    cout << endl; Reponse(abResponse, dwLenR);
    
    if(abResponse[0] == 0x90)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

bool Detector::TrameChangePIN()
{
    // Changement PIN (après envoi PIN)
    
    SCARD_IO_REQUEST ioSend; // Protocol header structure for the instruction
    ioSend.dwProtocol = dwActiveProtocol;
    ioSend.cbPciLength = sizeof(SCARD_IO_REQUEST);
    
    byte abResponse[2]={0}; // Réponse de la carte, sur 2 octets. Utilisé uniquement quand la carte doit renvoyer "90 00" et pas une chaîne plus longue.
    DWORD dwLenR = sizeof(abResponse); // Taille de abResponse (2) ou d'une autre chaine
    
    byte abChangePin[] = {0x80, 0x24, 0x00, 0x00, 0x08 , 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41}; // Nouveau code PIN : AAAAAAAA
    scardErr = SCardTransmit(hCard, &ioSend, abChangePin, sizeof(abChangePin), nullptr, abResponse, &dwLenR); // avec pin
    
    cout << "Fonction SCardTransmit (Changement PIN) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur " : "Ok ");
    if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << "code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
    cout << endl; Reponse(abResponse, dwLenR);
    
    if(abResponse[0] == 0x90)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

bool Detector::TrameClear()
{
    // Formatage des données (après envoi PIN)
    
    SCARD_IO_REQUEST ioSend; // Protocol header structure for the instruction
    ioSend.dwProtocol = dwActiveProtocol;
    ioSend.cbPciLength = sizeof(SCARD_IO_REQUEST);
    
    byte abResponse[2]={0}; // Réponse de la carte, sur 2 octets. Utilisé uniquement quand la carte doit renvoyer "90 00" et pas une chaîne plus longue.
    DWORD dwLenR = sizeof(abResponse); // Taille de abResponse (2) ou d'une autre chaine
    
    byte abClear[] = {0x80, 0x30, 0x00, 0x00, 0x00};
    scardErr = SCardTransmit(hCard, &ioSend, abClear, sizeof(abClear), nullptr, abResponse, &dwLenR);
    
    cout << "Fonction SCardTransmit (Formatage) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
    if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << "code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
    cout << endl; Reponse(abResponse, dwLenR);
    
    if(abResponse[0] == 0x90)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int Detector::TrameNcarte()
{
    SCARD_IO_REQUEST ioSend; // Protocol header structure for the instruction
    ioSend.dwProtocol = dwActiveProtocol;
    ioSend.cbPciLength = sizeof(SCARD_IO_REQUEST);
    
    byte abResponse[2]={0}; // Réponse de la carte, sur 2 octets. Utilisé uniquement quand la carte doit renvoyer "90 00" et pas une chaîne plus longue.
    DWORD dwLenR = sizeof(abResponse); // Taille de abResponse (2) ou d'une autre chaine
    
    byte Unique[16] = {0}; // Contenu du fichier : 8 octets uniques + état (si ok : 90 00)
    byte abSelectUnique[] = {0x80, 0xa4, 0x00, 0x00, 0x02, 0xFF, 0x00}; // Selection fichier FF 00 (MCU ID File)
    scardErr = SCardTransmit(hCard, &ioSend, abSelectUnique, sizeof(abSelectUnique), nullptr, abResponse, &dwLenR);
    
    cout << "Fonction SCardTransmit (Code Unique Select FF 00) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
    if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << "code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
    cout << endl; Reponse(abResponse, dwLenR); cout << endl << endl << endl;
    
    
    dwLenR = sizeof(Unique);
    byte abReadFile[] = {0x80, 0xb2, 0x00, 0x00, 0x08}; // 5e octet : nombre d'octets à lire (8)
    scardErr = SCardTransmit(hCard, &ioSend, abReadFile, sizeof(abReadFile), nullptr, Unique, &dwLenR);
    
    cout << "Fonction SCardTransmit (Code Unique Reception User) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
    if (scardErr != SCARD_S_SUCCESS || Unique[8] != 0x90) {cout << "code " << scardErr << endl; ListError(Unique, scardErr, choix);}
    cout << endl; Reponse(Unique, dwLenR); cout << endl;
    
    cout << "Code Unique : ";
    for (int i=0; i<8; i++)
    {
        printf("%02X ", Unique[i]); // Réponse héxa
    }
    cout << endl;
    
    if ( (Unique[0] == 0x04) && (Unique[1] == 0x54) && (Unique[2] == 0x41) && (Unique[3] == 0x00) && (Unique[4] == 0x2E) && (Unique[5] == 0x02) && (Unique[6] == 0x29) && (Unique[7] == 0x42) )
    {
        cout << "Carte 1 detectee." << endl;
        return 1;
    }
    else if ( (Unique[0] == 0x34) && (Unique[1] == 0xE3) && (Unique[2] == 0x99) && (Unique[3] == 0x0B) && (Unique[4] == 0xEF) && (Unique[5] == 0xE5) && (Unique[6] == 0x09) && (Unique[7] == 0xE2) )
    {
        cout << "Carte 2 detectee." << endl;
        return 2;
    }
    else 
    {
        cout << "Carte inconnue." << endl;
        return 0;
    }
}


bool Detector::TrameCreateFiles()
{
    cout << "Voulez-vous creer, modifier et supprimer des fichiers (0) ou simplement les lire (autre) ?" << endl;
    
    /*Reception choix utilisateur (pas avec cin) */ choix = 0;
    
    if (choix == 0)
    {
        
        // Envoie le code IC, sélectionne le fichier FF 02 (Personalization File), le modifie, séléctionne le fichier FF 04,
        // (User FIle Management File), renvoie le code IC, entre les noms des fichiers, séléctionne chacun les fichiers créés,
        // les lit, puis les reséléctionne pour écrire dessus. Je ne sais pas encore comment les supprimer, du coup le code est en commentaire.
        // L'Issuer Code peut être envoyé sans s'authentifier, contrairement au code PIN.
        // Selon le manuel, possible uniquement si la carte n'est pas en User Stage (donc possible sur une seule des 2 cartes).
        
        /*
        dwLenR = sizeof(abResponse);
        byte abIC[13] = {0x80, 0x20, 0x07, 0x00, 0x08, 0x41, 0x43, 0x4F, 0x53, 0x54, 0x45, 0x53, 0x54}; // submit IC CODE "ACOSTEST"
        
        scardErr = SCardTransmit (hCard, &ioSend, abIC, sizeof (abIC), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Code IC) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        // Selection Personalization File et envoi code IC
        
        byte abSelectPerso[] = {0x80, 0xA4, 0x00, 0x00, 0x02, 0xFF, 0x02}; // Select File FF 02
        scardErr = SCardTransmit (hCard, &ioSend, abSelectPerso, sizeof (abSelectPerso), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Select FF 02) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        scardErr = SCardTransmit (hCard, &ioSend, abIC, sizeof (abIC), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Code IC) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        
        // Création fichiers 1 (Allocation mémoire)
        
        byte abPersoEnvoi[] = {0x80, 0xD2, 0x00, 0x00, 0x04, 0x00, 0x00, 0x03, 0x00}; // Crée 3 fichiers simples sans options de sécurité
        scardErr = SCardTransmit (hCard, &ioSend, abPersoEnvoi, sizeof (abPersoEnvoi), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Ecriture FF 02) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        // Selection User File Management File
        
        byte abSelectUser[] = {0x80, 0xA4, 0x00, 0x00, 0x02, 0xFF, 0x04}; // Select File FF 04
        scardErr = SCardTransmit (hCard, &ioSend, abSelectUser, sizeof (abSelectUser), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Select FF 04) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        
        // Création fichiers 2 (Attribution noms)
        
        byte abCreate1[] = {0x80, 0xD2, 0x00, 0x00, 0x06,  0x0A, 0x03, 0x00, 0x00, 0xAA, 0x11}; // Write to first record of FF 04 <-- SELECT "AA11"
        scardErr = SCardTransmit (hCard, &ioSend, abCreate1, sizeof (abCreate1), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Envoi FF 04 Nom fichier 1) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        byte abCreate2[] = {0x80, 0xD2, 0x00, 0x00, 0x06,  0x10, 0x03, 0x00, 0x00, 0xBB, 0x22}; // Write to second record of FF 04 <-- SELECT "BB22"
        scardErr = SCardTransmit (hCard, &ioSend, abCreate2, sizeof (abCreate2), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Envoi FF 04 Nom fichier 2) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        byte abCreate3[] = {0x80, 0xD2, 0x00, 0x00, 0x06,  0x20, 0x03, 0x00, 0x00, 0xCC, 0x33}; // Write to third record of FF 04 <-- SELECT "CC33"
        scardErr = SCardTransmit (hCard, &ioSend, abCreate3, sizeof (abCreate3), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Envoi FF 04 Nom fichier 3) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        
        // Lecture fichiers créés 
        
        //Select "AA 11" first before reading it
        byte abSelectAA11[] = {0x80, 0xA4, 0x00, 0x00, 0x02, 0xAA, 0x11};
        scardErr = SCardTransmit (hCard, &ioSend, abSelectAA11, sizeof (abSelectAA11), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Select AA 11) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        byte abReadAA11[] = {0x80, 0xB2, 0x00, 0x00, 0x0A}; // Lire 10 octets
        scardErr = SCardTransmit (hCard, &ioSend, abReadAA11, sizeof (abReadAA11), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Lecture AA 11) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        //RecvLength = SendBuff[4] + 0x02; // taille à réserver pour la réception
        
        
        //Select "BB 22" first before reading it.
        byte abSelectBB22[] = {0x80, 0xA4, 0x00, 0x00, 0x02, 0xBB, 0x22};
        scardErr = SCardTransmit (hCard, &ioSend, abSelectBB22, sizeof (abSelectBB22), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Select BB 22) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        byte abReadBB22[] = {0x80, 0xB2, 0x00, 0x00, 0x10}; // Lire 10 octets
        scardErr = SCardTransmit (hCard, &ioSend, abReadBB22, sizeof (abReadBB22), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Lecture BB 22) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        //RecvLength = SendBuff[4] + 0x02; // taille à réserver pour la réception
        
        
        //Select "CC 33" first before reading it.
        byte abSelectCC33[] = {0x80, 0xA4, 0x00, 0x00, 0x02, 0xCC, 0x33};
        scardErr = SCardTransmit (hCard, &ioSend, abSelectCC33, sizeof (abSelectCC33), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Select CC 33) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        byte abReadCC33[] = {0x80, 0xB2, 0x00, 0x00, 0x20}; // Lire 10 octets
        scardErr = SCardTransmit (hCard, &ioSend, abReadCC33, sizeof (abReadCC33), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Lecture CC 33) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        //RecvLength = SendBuff[4] + 0x02; // taille à réserver pour la réception
        
        
        // Ecriture fichiers créés
        
        scardErr = SCardTransmit (hCard, &ioSend, abSelectAA11, sizeof (abSelectAA11), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Select AA 11) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        byte abWriteAA11[] = {0x80, 0xD2, 0x00, 0x00, 0x0A}; //Write Record on File AA 11 avec param P3 <---- nombre maxi d'octets à écrire
        cout << "Fonction SCardTransmit (Ecriture AA 11) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        scardErr = SCardTransmit (hCard, &ioSend, abWriteAA11, sizeof (abWriteAA11), nullptr, abResponse, &dwLenR);
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        
        scardErr = SCardTransmit (hCard, &ioSend, abSelectBB22, sizeof (abSelectBB22), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Select BB 22) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        byte abWriteBB22[] = {0x80, 0xD2, 0x00, 0x00, 0x10}; //Write Record on File BB22 avec param P3 <---- nombre maxi d'octets à écrire
        cout << "Fonction SCardTransmit (Ecriture BB 22) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        scardErr = SCardTransmit (hCard, &ioSend, abWriteBB22, sizeof (abWriteBB22), nullptr, abResponse, &dwLenR);
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        
        scardErr = SCardTransmit (hCard, &ioSend, abSelectCC33, sizeof (abSelectCC33), nullptr, abResponse, &dwLenR);
        cout << "Fonction SCardTransmit (Select CC 33) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        byte abWriteCC33[] = {0x80, 0xD2, 0x00, 0x00, 0x20}; //Write Record on File CC33 avec param P3 <---- nombre maxi d'octets à écrire
        cout << "Fonction SCardTransmit (Ecriture CC 33) - " << (scardErr != SCARD_S_SUCCESS ? "Erreur : " : "Ok ");
        scardErr = SCardTransmit (hCard, &ioSend, abWriteCC33, sizeof (abWriteCC33), nullptr, abResponse, &dwLenR);
        if (scardErr != SCARD_S_SUCCESS || abResponse[0] != 0x90) {cout << ": code " << scardErr << endl; ListError(abResponse, scardErr, choix);}
        cout << endl; Reponse(abResponse, dwLenR); cout << endl;
        
        
        // Selection Personalization File
        
        
        // Destruction fichiers créés (Désallocation mémoire) - Comment faire ?
        
        
        if(abResponse[0] == 0x90)
        {
            return 1;
        }
        else
        {
            return 0;
        }
        
        */
    }
    
    else 
    {
        /*
        if(true)
        {
            cout << "Pas de fichier detecte" << endl;
        }
        else 
        {
            cout << "Lecture des fichiers..." << endl;
        }
        */
    }
    
    return 0;
}
