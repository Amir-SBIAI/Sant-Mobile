#ifndef MENUUSER2_H
#define MENUUSER2_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTcpSocket>
#include <QTimer>

class MenuUser2 : public QWidget
{
    Q_OBJECT

public:
    explicit MenuUser2(QWidget *parent = nullptr);
    ~MenuUser2();

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

#endif // MENUUSER2_H
