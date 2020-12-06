#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "detector.h"
#include "cpp_http.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT
    
public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
    
    bool Connexion(bool);
    bool EnvoiServeur();
    string ReceptServeur();
    
private slots:
    void on_pushButtonEnvoi_clicked();
    
    void on_pushButtonShutdown_clicked();
    
    void on_pushButtonStop_clicked();
    
    void on_radioButtonAdmin_clicked();
    
    void on_radioButtonUser_clicked();
    
private:
    Ui::Widget *ui;
    
    Detector * Carte;
    Cpp_http * monTcp;
    
    bool verifAdmin, etat;
    int phase, NCARTE;
    string stringServeur, reponseServeur;
};

#endif // WIDGET_H
