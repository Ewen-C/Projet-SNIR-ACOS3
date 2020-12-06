#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    
    phase = 1; verifAdmin = 0; NCARTE = -1;
    ui->labelInstructs->setText("Voulez-vous lancer le programme en tant qu'administrateur ou en tant qu'utilisateur ?");
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_radioButtonAdmin_clicked()
{
    if (phase == 1 && verifAdmin == 0)
    {
        ui->textEditEtat->append ("Veuillez entrer le code Administrateur : 'ACOSTEST2'.");
        verifAdmin = 1;
    }
}
// Changement de mode possible uniquement au début de la transmission
void Widget::on_radioButtonUser_clicked()
{
    if (phase == 1 && verifAdmin == 1)
    {
        ui->textEditEtat->append ("Retour au mode Utilisateur.");
        verifAdmin = 0;
    }
}


void Widget::on_pushButtonEnvoi_clicked()
{
    // phase 1 : connection, phase 2 : num carte, puis demande PIN si user ou choix commandes si admin, 
    // phase 3 : envoi au serveur de 'toString(NCARTE) + ",ewen,bobby"' et affichage de sa réponse.
    // Les étapes se suivent à la chaîne en User, on peut sélectionner les trames en Admin.
    
    if (phase == 1 && verifAdmin == 1 && ui->lineEditEnvoi->text() != "ACOSTEST2")
    {
        ui->textEditEtat->append ("Code Administrateur incorrect.");
    }
    
    if (phase == 1 && verifAdmin == 0) // User 1
    {
        ui->textEditEtat->append ("Début de la transmission...");
        etat = Connexion(0);
        
        if(etat == 1)
        {
            ui->textEditEtat->append ("Communication établie.");
            phase = 2; 
        }
        else 
        {
            ui->textEditEtat->append ("Une erreur de communication est survenue.");
        }
    }
    
    
    if (phase == 2 && verifAdmin == 0) // User 2
    {
        if(NCARTE == -1) // Eviter que le numéro soit vérifié plusieurs fois
        {
            //Carte->Trames (6, 0); // Commande 6 : lire MCU ID (FF 00)
            NCARTE = Carte->TrameNcarte();
            
            if (NCARTE == 0)
            {
                ui->textEditEtat->append ("La carte entrée est inconnue.");
            }
            else
            {
                ui->textEditEtat->append ("La carte entrée est la n°" + QString::number(NCARTE) + ".");
            }
            //delete Carte; // Eviter que la carte se déconnecte quand inactive et que le programme bug, ça bug pas en fait.
            ui->textEditEtat->append("Veuillez entrer le code PIN de la carte.");
            ui->labelInstructs->setText("Veuillez entrer le code PIN de la carte.");
        }
        
        if(ui->lineEditEnvoi->text() == "ACOSTEST")
        {
            ui->textEditEtat->append ("Code PIN correct. Déconnexion de la carte.");
            delete Carte;
            phase = 3;
        }
        else 
        {
            if (ui->lineEditEnvoi->text() != "")
            {
                ui->textEditEtat->append("Code PIN incorrect.");
            }
        }
    }
    
    if (phase == 3 && verifAdmin == 0) // User 3
    {
        ui->textEditEtat->append("Envoi des informations au serveur...");
        
        if(NCARTE == 1)
        {
            stringServeur = "045441002E022942";
        }
        if(NCARTE == 2)
        {
            stringServeur = "34E3990BEFE509E2";
        }
        
        stringServeur += ",ewen,bobby";
        
        etat = EnvoiServeur();
        
        if(etat == 1)
        {
            ui->textEditEtat->append("Envoi terminé. Chaîne envoyée : '" + QString::fromStdString(stringServeur) + "'." );
            
            
            reponseServeur = ReceptServeur();
            
            if (reponseServeur.length() > 0)
            {
                ui->textEditEtat->append("Réponse du serveur : '" + QString::fromStdString(reponseServeur) + "'.");
            }
            else
            {
                ui->textEditEtat->append("Un problème est survenu lors de la réponse du serveur.");
            }
            
            ui->textEditEtat->append("Transmission terminée.");
            phase = 1; delete monTcp;
            ui->labelInstructs->setText("Voulez-vous lancer le programme en tant qu'administrateur ou en tant qu'utilisateur ?");
        }
        else 
        {
            ui->textEditEtat->append ("Echec de l'envoi au serveur.");
        }
    }
    
    
    if (phase == 1 && verifAdmin == 1 && ui->lineEditEnvoi->text () == "ACOSTEST2") // Admin 1
    {
        ui->textEditEtat->append ("Vérification terminée. Début de la transmission...");
        etat = Connexion(1);
        
        if(etat == 1)
        {
            ui->textEditEtat->append ("Communication établie.");
            NCARTE = Carte->TrameNcarte();
            
            if (NCARTE == 0)
            {
                ui->textEditEtat->append ("La carte entrée est inconnue.");
            }
            else
            {
                ui->textEditEtat->append ("La carte entrée est la n°" + QString::number(NCARTE) + ".");
                phase = 2;
                
                ui->textEditEtat->append ("\nChoisir l'instruction : \n"
                                          "Actions : 1 - Debuter Session + Authentification | 2 - Envoi Code PIN\n"
                                          "Actions : 3 - Changement Code PIN | 4 - Formatage | 5 - Envoi code ID unique\n"
                                          "Actions : 6 - Création & Lecture de fichiers (à venir)\n"
                                          "Autre - Quitter");
                
                ui->lineEditEnvoi->clear ();
            }
        }
        else 
        {
            ui->textEditEtat->append ("Une erreur de communication est survenue.");
        }
    }
    
    if (phase == 2 && verifAdmin == 1) // Admin 2
    {
        if (ui->lineEditEnvoi->text() != "")
        {
            int num = ui->lineEditEnvoi->text ().toInt ();
            
            if(num >= 1 && num <= 6)
            {                
                if(num == 1)
                {
                    etat = Carte->TrameSecur ();
                }
                
                else if(num == 2)
                {
                    etat = Carte->TrameEnvoiPIN ();
                }
                
                else if(num == 3)
                {
                    etat = Carte->TrameChangePIN ();
                }
                
                else if(num == 4)
                {
                    etat = Carte->TrameClear ();
                }
                
                else if(num == 5)
                {
                    NCARTE = Carte->TrameNcarte ();
                    
                    if (NCARTE == 0)
                    {
                        ui->textEditEtat->append ("La carte entrée est inconnue.");
                    }
                    else
                    {
                        ui->textEditEtat->append ("La carte entrée est la n°" + QString::number(NCARTE) + ".");
                    }
                }
                
                else if(num == 6)
                {
                    if (NCARTE == 2)
                    {
                        cout << "Opétaration impossible sur la carte en User Stage.\n";
                    }
                    else 
                    {
                        etat = Carte->TrameCreateFiles ();
                    }
                }
                
                if ( (num == 1) || (num == 2) || (num == 3) || (num == 4) || (num == 6) )
                {
                    if (etat == 1)
                        ui->textEditEtat->append ("Fonction OK.\n");
                    
                    else 
                        ui->textEditEtat->append ("Echec de la fonction.\n");
                }
                
                
                ui->textEditEtat->append ("\nChoisir l'instruction : \n"
                                          "Actions : 1 - Debuter Session + Authentification | 2 - Envoi Code PIN\n"
                                          "Actions : 3 - Changement Code PIN | 4 - Formatage | 5 - Envoi code ID unique\n"
                                          "Actions : 6 - Création & Lecture de fichiers (à venir)\n"
                                          "Autre - Quitter");
            }
            else 
            {
                ui->textEditEtat->append ("Fin de la transmission.");
                delete Carte; phase = 1; NCARTE = -1; verifAdmin = 0;
                ui->labelInstructs->setText("Voulez-vous lancer le programme en tant qu'administrateur ou en tant qu'utilisateur ?");
            }
        }
    }
    
    
    ui->lineEditEnvoi->clear ();
}

bool Widget::Connexion(bool ADMIN)
{
    Carte = new Detector(ADMIN); // 0 = utilisateur, 1 = admin
    etat = Carte->Communication();
    return etat;
}

bool Widget::EnvoiServeur()
{
    monTcp = new Cpp_http;
    etat = monTcp->Connexion("86.238.113.11", 1234, stringServeur);
    return etat;
}

string Widget::ReceptServeur()
{
    return monTcp->Reception();
}


void Widget::on_pushButtonShutdown_clicked()
{
    if (phase != 1)
    {
        delete Carte; phase = 1;
    }
    this->close();
}

void Widget::on_pushButtonStop_clicked()
{
    if (phase != 1)
    {
        ui->textEditEtat->append ("Transmission interrompue (Stop).");
        delete Carte; phase = 1; NCARTE = -1;
        ui->labelInstructs->setText("Voulez-vous lancer le programme en tant qu'administrateur ou en tant qu'utilisateur ?");
    }
    else 
    {
        ui->textEditEtat->append ("La transmission n'a pas encore commencé.");
    }
}
