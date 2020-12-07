# Projet SNIR ACOS3

## Introduction

Projet de fin de deuxième année de BTS par Ewen Celibert.
Le but global du projet est d'ouvirir une porte en récupérant les données d'une carte à puce et en les transmettant au serveur qui actionne une gâche électrique.
Ma partie se concentre sur l'identification de la carte à puce et l'envoi des données au serveur par Ethernet.

## Composants utilisés

Ce projet utilise la bibliothèque logicielle winscard.h pour se connecter au lecteur et à la carte et leur donner des commandes.
Le lecteur est du modèle ACR38, et la carte est du modèle ACOS3. Les deux composants sont de la marque ACS (Advanced Card Systems)
L'Interface Homme-Machine et le code du projet ont été écrits avec le framework C++ QT Creator.
Pour ouvrir le projet depuis QT, ouvrir le fichier 'IHM_ACOS3.pro'.

## Connexion à la carte

Le contenu des trames envoyées et reçues en correspondant avec la carte est en code ASCII hexadécimal : 0x41 → 65 → A.
Le programme utilise les methodes de winscard.h pour se connecter à la carte, récupérer ses informations, puis la déconnecter.
La méthode permettant d'envoyer des commandes à la carte est ScardTransmit().

## Indentification de la carte

Pour identifier la carte, le contenu du fichier nommé FF 00 de la carte est lu. Il contient son numéro de série et son numéro de version.
Les numéros de série uniques des deux cartes issus des fichiers FF 00 sont les suivants :
- Carte 1 : 04 54 41 00 2E 02 29 42
- Carte 2 : 34 E3 99 0B EF E5 09 E2
Si les codes sont corrects, les données sont envoyées au serveur avec la fonction send() de winsock2.

## Interface Homme-Machine

L'IHM demande à l'utilisateur le code de sécutité 'ACOSTEST' pour envoyer les informations au serveur.
La trame envoyée contient le numéro de la carte suivi d'un mot de passe.
Si le serveur reçoit bien les informations, il renvoie 'Données reçues', et ce message est affiché sur l'interface.
En fonction de l'horaire, et des informations dans la base de données, il ouvre ou non la porte.
