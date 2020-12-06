#ifndef DETECTOR_H
#define DETECTOR_H

#ifdef WIN32
#undef UNICODE
#endif

#pragma once
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winscard.h> //Windows Smart Scard
#include <windows.h>
#include <string>

using namespace std;

class Detector
{
private :
    int choix, etape, admin; // 0 = admin

    SCARDCONTEXT Context; // Handle to the established resource manager context.
    LONG scardErr; // Error code returned by the called functions

    LPTSTR mszReaders; // Multi-string that lists the card readers, LPCTSTR ou LPTSTR ?
    LPTSTR mszCurrentReader; // Current reader name within the multi-string list
    DWORD dwReaders;  // Length of the mszReaders buffer

    SCARDHANDLE hCard; // Handle that identifies the connection to the smart card in the designated reader.
    DWORD dwActiveProtocol;	// Flag that indicates the established active protocol.
    
    SCARD_IO_REQUEST ioSend;

public:
    Detector(int admin);
    ~Detector();

    bool Communication();
    
    bool TrameSecur();
    bool TrameEnvoiPIN();
    bool TrameChangePIN();
    bool TrameClear();
    int  TrameNcarte();
    bool TrameCreateFiles();
    
    void ATRCheck(BYTE *);
    void Reponse(BYTE *, DWORD);
    int Decompo(char);
    void ListError(BYTE *, LONG, int); // code héxa carte, code erreur lecteur, et étape pouvant changer la signification 
};

#endif // DETECTOR_H
