#include "pageaffichage.h"
#include "menuuser1.h"
#include "menuuser2.h"
#include "menuadmin.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QTcpSocket>
#include <QTime>
#include <QStackedWidget>

PageAffichage::PageAffichage(QString mode, QWidget *parent)
    : QWidget(parent), modeUtilisateur(mode), tempsRestant(0), boutonUrgenceActive(false)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Titre
    QLabel *titre = new QLabel("Affichage des constantes physiologiques et environnementales", this);
    titre->setStyleSheet("font-size: 20px; font-weight: bold; padding: 10px;");
    titre->setAlignment(Qt::AlignCenter);
    layout->addWidget(titre);

    // Labels pour constantes physiologiques
    labelPhysio = new QLabel("📊 **Constantes Physiologiques**", this);
    labelPhysio->setStyleSheet("font-size: 18px; font-weight: bold;");
    layout->addWidget(labelPhysio);

    labelTempCorp = new QLabel("Température corporelle : -- °C", this);
    labelBouton = new QLabel("Bouton : --", this);
    labelECG = new QLabel("Électrocardiogramme : -- BPM", this);
    labelAccelero = new QLabel("Détecteur de chute : -- ", this);
    labelEtatConscience = new QLabel("État de conscience : --", this);

    layout->addWidget(labelTempCorp);
    layout->addWidget(labelBouton);
    layout->addWidget(labelECG);
    layout->addWidget(labelAccelero);
    layout->addWidget(labelEtatConscience);

    // Labels pour constantes environnementales
    labelEnviron = new QLabel("🌍 **Constantes Environnementales**", this);
    labelEnviron->setStyleSheet("font-size: 18px; font-weight: bold;");
    layout->addWidget(labelEnviron);

    labelQualiteAir = new QLabel("Qualité de l'air : -- PPM", this);
    labelTempAmbiante = new QLabel("Température ambiante : -- °C", this);

    layout->addWidget(labelQualiteAir);
    layout->addWidget(labelTempAmbiante);

    // Bouton "Retour"
    boutonRetour = new QPushButton("Retour", this);
    boutonRetour->setStyleSheet("padding: 10px; font-size: 16px;");
    layout->addWidget(boutonRetour);

    setLayout(layout);
    setWindowTitle("Affichage des données en temps réel");
    resize(500, 400);

    // Connexion du bouton retour
    connect(boutonRetour, &QPushButton::clicked, this, &PageAffichage::retournerAuMenu);

    // Initialisation du timer pour le compte à rebours
    compteAReboursTimer = new QTimer(this);
    connect(compteAReboursTimer, &QTimer::timeout, this, &PageAffichage::mettreAJourCompteARebours);

    // Connexion au serveur TCP/IP
    socket = new QTcpSocket(this);
    socket->connectToHost("192.168.15.104", 5001);

    connect(socket, &QTcpSocket::readyRead, this, &PageAffichage::recevoirDonneesCapteurs);
    connect(socket, &QTcpSocket::errorOccurred, this, [](QAbstractSocket::SocketError error){
        qDebug() << "Erreur de connexion :" << error;
    });
}

PageAffichage::~PageAffichage()
{
    if (compteAReboursTimer->isActive()) {
        compteAReboursTimer->stop();
    }
    delete compteAReboursTimer;
    socket->disconnectFromHost();
}

void PageAffichage::recevoirDonneesCapteurs()
{
    QByteArray data = socket->readAll();
    QString valeurs = QString::fromUtf8(data).trimmed();
    qDebug() << "[TRAME REÇUE] :" << valeurs;

    QStringList listeValeurs = valeurs.split(";");

    if (listeValeurs.size() > 0) {
        if (listeValeurs[0] == "CP" && listeValeurs.size() >= 7) {
            labelTempCorp->setText("Température corporelle : " + listeValeurs[1] + " °C");

            // Traitement spécial pour le bouton avec compte à rebours
            if (listeValeurs[2] == "1" && !boutonUrgenceActive) {
                // Bouton d'urgence activé - démarrer le compte à rebours
                demarrerCompteARebours();
            }
            else if (listeValeurs[2] == "0" && !boutonUrgenceActive) {
                // Bouton non pressé et pas de compte à rebours en cours
                labelBouton->setText("Bouton : OK");
                labelBouton->setStyleSheet("color: green;");
            }

            labelECG->setText("Électrocardiogramme : " + listeValeurs[3] + " BPM");

            // Traitement spécial pour le détecteur de chute
            if (listeValeurs[4] == "0") {
                labelAccelero->setText("Détecteur de chute : OK");
                labelAccelero->setStyleSheet("color: green;");
            } else if (listeValeurs[4] == "1") {
                labelAccelero->setText("Détecteur de chute : CHUTE POTENTIELLE");
                labelAccelero->setStyleSheet("color: red; font-weight: bold;");
            } else {
                labelAccelero->setText("Détecteur de chute : " + listeValeurs[4]);
                labelAccelero->setStyleSheet("");
            }

            labelEtatConscience->setText("État de conscience : " + listeValeurs[5]);
        }
        else if (listeValeurs[0] == "CE" && listeValeurs.size() >= 3) {
            labelQualiteAir->setText("Qualité de l'air : " + listeValeurs[1] + " PPM");
            labelTempAmbiante->setText("Température ambiante : " + listeValeurs[2] + " °C");
        }
    }
}

void PageAffichage::demarrerCompteARebours()
{
    // Arrêter le timer s'il est déjà en cours
    if (compteAReboursTimer->isActive()) {
        compteAReboursTimer->stop();
    }

    // Réinitialiser le temps restant
    tempsRestant = 10;
    boutonUrgenceActive = true;

    // Mettre à jour immédiatement l'affichage
    labelBouton->setText("Bouton : URGENCE (" + QString::number(tempsRestant) + "s)");
    labelBouton->setStyleSheet("color: red; font-weight: bold;");

    // Démarrer le timer (1 seconde d'intervalle)
    compteAReboursTimer->start(1000);
}

void PageAffichage::arreterCompteARebours()
{
    compteAReboursTimer->stop();
    boutonUrgenceActive = false;
    labelBouton->setText("Bouton : OK");
    labelBouton->setStyleSheet("color: green;");
}

void PageAffichage::mettreAJourCompteARebours()
{
    tempsRestant--;

    if (tempsRestant > 0) {
        labelBouton->setText("Bouton : URGENCE (" + QString::number(tempsRestant) + "s)");
    } else {
        // Compte à rebours terminé
        arreterCompteARebours();
    }
}

void PageAffichage::retournerAuMenu()
{
    qDebug() << "Retour au menu principal...";

    if (modeUtilisateur == "admin") {
        MenuAdmin *menu = new MenuAdmin();
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->show();
    } else if (modeUtilisateur == "user1") {
        MenuUser1 *menu = new MenuUser1();
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->show();
    } else if (modeUtilisateur == "user2") {
        MenuUser2 *menu = new MenuUser2();
        menu->setAttribute(Qt::WA_DeleteOnClose);
        menu->show();
    }

    this->close();
}
