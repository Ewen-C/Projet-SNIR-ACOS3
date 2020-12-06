# Projet SNIR ACOS3
Projet de fin de deuxième année de BTS par Ewen Celibert.
Le but global du projet est d'ouvirir une porte en récupérant les données d'une carte à puce et en les transmettant au serveur qui actionne une gâche électrique.
Ma partie se concentre sur l'identification de la carte à puce et l'envoi des données au serveur par Ethernet.

# Composants Utilisés
Ce projet utilise la bibliothèque logicielle winscard.h pour se connecter à la carte et lui donner des commandes.
Son lecteur est du modèle ACR38, et la carte est du modèle ACOS3. Les deux composants sont de la marque ACS (Advanced Card Systems)
L'Interface Homme-Machine et le code du projet ont été écrits avec le framework C++ QT Creator.

