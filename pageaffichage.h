#ifndef PAGEAFFICHAGE_H
#define PAGEAFFICHAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTcpSocket>
#include <QTimer>

class PageAffichage : public QWidget
{
    Q_OBJECT

public:
    explicit PageAffichage(QString mode, QWidget *parent = nullptr);
    ~PageAffichage();

private slots:
    void recevoirDonneesCapteurs();
    void retournerAuMenu();
    void mettreAJourCompteARebours();

private:
    QString modeUtilisateur;
    QTcpSocket *socket;

    QLabel *labelPhysio;
    QLabel *labelTempCorp;
    QLabel *labelBouton;
    QLabel *labelECG;
    QLabel *labelAccelero;
    QLabel *labelEtatConscience;
    QLabel *labelEnviron;
    QLabel *labelQualiteAir;
    QLabel *labelTempAmbiante;
    QPushButton *boutonRetour;

    QTimer *compteAReboursTimer;
    int tempsRestant;
    bool boutonUrgenceActive;

    void demarrerCompteARebours();
    void arreterCompteARebours();
};

#endif // PAGEAFFICHAGE_H
