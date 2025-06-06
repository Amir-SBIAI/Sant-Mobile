#ifndef MENUUSER1_H
#define MENUUSER1_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTcpSocket>
#include <QTimer>

class MenuUser1 : public QWidget
{
    Q_OBJECT

public:
    explicit MenuUser1(QWidget *parent = nullptr);
    ~MenuUser1();

private slots:
    void ouvrirPageAffichage();
    void ouvrirPageHistorique();
    void deconnecterUtilisateur();
    void envoyerAlerte();
    void verifierReponse();

private:
    QLabel *logo;
    QPushButton *boutonAffichage;
    QPushButton *boutonHistorique;
    QPushButton *boutonDeconnexion;
    QPushButton *boutonAlerte; // Bouton d'alerte rouge ajouté
    QTcpSocket *socket;
    QTimer *timer;

    QString genererCodeAleatoire();
};

#endif // MENUUSER1_H
