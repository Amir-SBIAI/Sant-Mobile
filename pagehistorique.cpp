#include "pagehistorique.h"
#include "menuadmin.h"
#include "menuuser1.h"
#include "menuuser2.h"
#include <QVBoxLayout>
#include <QDebug>

PageHistorique::PageHistorique(QString mode, QWidget *parent)
    : QWidget(parent), modeUtilisateur(mode)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Création de la vue web pour afficher la page
    webView = new QWebEngineView(this);
    webView->load(QUrl("http://192.168.15.104:5003/mesures"));
    layout->addWidget(webView);

    // Bouton "Retour"
    boutonRetour = new QPushButton("Retour", this);
    boutonRetour->setStyleSheet("padding: 10px; font-size: 16px;");
    layout->addWidget(boutonRetour);

    setLayout(layout);
    setWindowTitle("Historique des mesures");
    resize(800, 600);

    // Connexion du bouton retour
    connect(boutonRetour, &QPushButton::clicked, this, &PageHistorique::retournerAuMenu);
}

PageHistorique::~PageHistorique() {
    delete webView;
}

void PageHistorique::retournerAuMenu()
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


