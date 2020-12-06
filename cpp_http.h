#ifndef CPP_HTTP_H
#define CPP_HTTP_H

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <winsock2.h>

using namespace std;

class Cpp_http
{
private :
    WSADATA initialisation_win32;
    int erreur, tempo, nb_caracteres;
    char buffer[1500] = {0};
    
    SOCKET ID_SOCK;
    SOCKADDR_IN Infos;
    fstream Fich;
    
public:
    Cpp_http();
    ~Cpp_http();
    bool Connexion(string, unsigned short, string);
    string Reception();
};

#endif // CPP_HTTP_H
