#include "menuuser1.h"
#include "pageaffichage.h"
#include "pagehistorique.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QDebug>
#include <QRandomGenerator>
#include <QMessageBox>

MenuUser1::MenuUser1(QWidget *parent)
    : QWidget(parent)
{
    // Création du titre
    QLabel *titre = new QLabel("Menu Surveillance Niel.Otto", this);
    titre->setStyleSheet("font-size: 22px; font-weight: bold; padding-left: 15px;");

    // Ajout du logo
    logo = new QLabel(this);
    QPixmap pixmap("/home/sbiai/Images/LOGO.png");
    logo->setPixmap(pixmap.scaled(50, 50, Qt::KeepAspectRatio));
    logo->setFixedSize(50, 50);

    // Disposition du logo et du titre sur la même ligne
    QHBoxLayout *logoLayout = new QHBoxLayout();
    logoLayout->addWidget(logo);
    logoLayout->addWidget(titre);
    logoLayout->addStretch();

    // Création des boutons
    boutonAffichage = new QPushButton("Afficher les mesures physiologiques et environnementales", this);
    boutonHistorique = new QPushButton("Afficher l'historique des mesures", this);

    boutonAffichage->setStyleSheet("padding: 15px; font-size: 16px;");
    boutonHistorique->setStyleSheet("padding: 15px; font-size: 16px;");

    // Bouton d'alerte rouge
    boutonAlerte = new QPushButton("⚠️ Vérification de l'état de conscience", this);
    boutonAlerte->setStyleSheet("padding: 15px; font-size: 16px; background-color: red; color: white; font-weight: bold;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(logoLayout);
    layout->addWidget(boutonAffichage);
    layout->addWidget(boutonHistorique);
    layout->addWidget(boutonAlerte);

    setLayout(layout);
    setWindowTitle("Menu Utilisateur 1");
    resize(500, 400);

    connect(boutonAffichage, &QPushButton::clicked, this, &MenuUser1::ouvrirPageAffichage);
    connect(boutonHistorique, &QPushButton::clicked, this, &MenuUser1::ouvrirPageHistorique);
    connect(boutonAlerte, &QPushButton::clicked, this, &MenuUser1::envoyerAlerte);

    // Ajout du bouton "Déconnexion"
    boutonDeconnexion = new QPushButton("Déconnexion", this);
    boutonDeconnexion->setStyleSheet("padding: 12px; font-size: 16px; background-color: red; color: white; font-weight: bold;");
    layout->addWidget(boutonDeconnexion);
    connect(boutonDeconnexion, &QPushButton::clicked, this, &MenuUser1::deconnecterUtilisateur);

    // Initialisation du socket et du timer
    socket = new QTcpSocket(this);
    timer = new QTimer(this);
    timer->setInterval(10000);  // 10 secondes pour attendre une réponse

    connect(timer, &QTimer::timeout, this, &MenuUser1::verifierReponse);
    connect(socket, &QTcpSocket::readyRead, this, &MenuUser1::verifierReponse);
}

MenuUser1::~MenuUser1()
{
    socket->disconnectFromHost();
    delete socket;
    delete timer;
}

void MenuUser1::envoyerAlerte()
{
    QString codeAleatoire = genererCodeAleatoire();
    QString trame = "V;" + codeAleatoire + ";[1];$";

    qDebug() << "Envoi de la trame d'alerte :" << trame;

    boutonAlerte->setEnabled(false);  // Désactive le bouton après l'envoi

    socket->connectToHost("192.168.15.104", 5001);
    if (socket->waitForConnected(3000)) {
        socket->write(trame.toUtf8());
        socket->waitForBytesWritten(3000);

        timer->start();  // Démarrer le timer pour attendre la réponse
    }
}

void MenuUser1::verifierReponse()
{
    QByteArray data = socket->readAll();
    QString valeurs = QString::fromUtf8(data).trimmed();

    if (!valeurs.isEmpty()) {
        qDebug() << "[Réponse reçue] :" << valeurs;
        timer->stop();  // Arrêter le timer
        boutonAlerte->setEnabled(true);  // Réactiver le bouton
    } else {
        qDebug() << "Alerte : aucune réponse reçue après 10 secondes !";
        QMessageBox::warning(this, "Alerte", "⚠️ Aucun signal reçu après 10 secondes !");
        boutonAlerte->setEnabled(true);  // Réactiver le bouton en cas d'alerte
    }
}

void MenuUser1::ouvrirPageAffichage()
{
    qDebug() << "Ouverture de la page d'affichage des mesures...";
    PageAffichage *page = new PageAffichage("user1");
    page->setAttribute(Qt::WA_DeleteOnClose);
    page->show();
    this->close();

}


void MenuUser1::ouvrirPageHistorique()
{
    qDebug() << "Ouverture de la page d'historique des mesures...";
    PageHistorique *page = new PageHistorique("user1");
    page->setAttribute(Qt::WA_DeleteOnClose);
    page->show();
    this->close();
}

void MenuUser1::deconnecterUtilisateur()
{
    qDebug() << "Déconnexion de l'utilisateur...";
    MainWindow *fenetreConnexion = new MainWindow();
    fenetreConnexion->setAttribute(Qt::WA_DeleteOnClose);
    fenetreConnexion->show();
    this->close();
}


QString MenuUser1::genererCodeAleatoire()
{
    return QString::number(QRandomGenerator::global()->bounded(10000, 99999));
}
