#include "cpp_http.h"

using namespace std;


Cpp_http::Cpp_http()
{
    // Initialisation
    
    erreur = WSAStartup(MAKEWORD(2,2), &initialisation_win32);
    if (erreur != 0)
    {
        cout << "Desole, une erreur de Winsock est survenue : " << erreur << WSAGetLastError() << endl;
    }
    else
    {
        cout << "Initialisation en cours..." << endl;
    }
    
    // Ouverture
    
    ID_SOCK = socket(AF_INET,SOCK_STREAM,0); // Protocole TCP IPv4
    
    if (ID_SOCK == INVALID_SOCKET)
    {
        cout << "Desole, une erreur de la socket est survenue : " << erreur << WSAGetLastError() << endl;
    }
    else
    {
        cout << "Socket cree..." << endl;
    }
    
    // Activation de l'algo de Nagle
    
    tempo = 1;
    erreur = setsockopt(ID_SOCK, IPPROTO_TCP, TCP_NODELAY, (char *)&tempo, sizeof(tempo) );
    
    if (erreur != 0)
    {
        cout << "Desole, une erreur de configuration est survenue : " << erreur << WSAGetLastError() << endl;
    }
    else
    {
        cout << "Configuration en cours..." << endl;
    }
}


bool Cpp_http::Connexion(string IP, unsigned short PORT, string commande)
{
    // Envoi du paramètre commande en TCP
    
    Infos.sin_family = AF_INET;
    Infos.sin_addr.s_addr = inet_addr(IP.c_str());
    Infos.sin_port = htons(PORT); // Port
    erreur = connect(ID_SOCK,(struct sockaddr*)&Infos, sizeof(Infos));
    
    if (erreur != 0)
    {
        cout << "Desole, impossible d'ecouter ce port : " << erreur << WSAGetLastError() << endl;
        return 0;
    }
    else
    {
        cout << "Port pret a l'ecoute..." << endl << endl;
        
        strcpy(buffer, commande.c_str()); // Conversion string -> char
        nb_caracteres=send(ID_SOCK, buffer, strlen(buffer),0);
        
        if(nb_caracteres==SOCKET_ERROR)
        {
            cout << endl << "Desole, je n'ai pas pu envoyer les donnees du a l'erreur : " << WSAGetLastError();
            return 0;
        }
        else
        {
            cout << "Envoi : OK" << endl;
            return 1;
        }
    }
}


string Cpp_http::Reception()
{
    cout << "Reception des donnees en cours..." << endl << endl;
    nb_caracteres = recv(ID_SOCK, buffer, sizeof (buffer), 0);
    if (nb_caracteres == SOCKET_ERROR)
    {
        cout << "Desole, echec de la reception : " << WSAGetLastError() << endl;
        return nullptr;
    }
    else
    {
        string recept = buffer;
        cout << "Donnees recues : " << recept << endl << endl;
        
        return recept;
    }
}


Cpp_http::~Cpp_http()
{
    // Fermeture
    
    erreur = shutdown(ID_SOCK, 2); // 2 : émission et écoute
    if (erreur != 0)
    {
        cout << "Desole, echec de la fermeture : " << WSAGetLastError() << endl;
    }
    else
    {
        cout << "Session fermee !" << endl;
    }
    
    // Liberation
    
    erreur = closesocket(ID_SOCK);
    if (erreur != 0)
    {
        cout << "Desole, echec de la liberation du socket : " << WSAGetLastError() << endl;
    }
    else
    {
        cout << "Socket libere !" << endl;
    }
    
    // Sortie
    
    erreur = WSACleanup();
    if (erreur != 0)
    {
        cout << "Desole, echec de la liberation de Winsock : " << WSAGetLastError() << endl;
    }
    else
    {
        cout << "WSA libere !" << endl;
    }
}
