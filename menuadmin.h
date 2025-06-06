#ifndef MENUADMIN_H
#define MENUADMIN_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTcpSocket>
#include <QTimer>

class MenuAdmin : public QWidget
{
    Q_OBJECT

public:
    explicit MenuAdmin(QWidget *parent = nullptr);
    ~MenuAdmin();

private slots:
    void ouvrirPageModification();
    void ouvrirPageAffichage();
    void ouvrirPageHistorique();
    void deconnecterUtilisateur();
    void envoyerAlerte();
    void verifierReponse();

private:
    QPushButton *boutonModification;
    QPushButton *boutonAffichage;
    QPushButton *boutonHistorique;
    QPushButton *boutonDeconnexion;
    QPushButton *boutonAlerte; // Bouton d'alerte rouge ajouté
    QLabel *logo;
    QTcpSocket *socket;
    QTimer *timer;

    QString genererCodeAleatoire();
};

#endif // MENUADMIN_H
