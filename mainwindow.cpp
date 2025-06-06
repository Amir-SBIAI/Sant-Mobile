#include "mainwindow.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QPixmap>
#include <QHBoxLayout>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QFile>
#include <QDir>

#include "menuadmin.h"
#include "menuuser1.h"
#include "menuuser2.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), socket(new QSslSocket(this))
{
    // Configuration de l'interface
    titre = new QLabel("Santé Mobile", this);
    titre->setStyleSheet("font-size: 24px; font-weight: bold; padding: 20px;");

    logo = new QLabel(this);
    QPixmap pixmap("/home/sbiai/Images/LOGO.png");
    logo->setPixmap(pixmap.scaled(50, 50, Qt::KeepAspectRatio));
    logo->setFixedSize(50, 50);

    QHBoxLayout *titreLayout = new QHBoxLayout();
    titreLayout->addWidget(titre);
    titreLayout->addWidget(logo);

    QWidget *titreWidget = new QWidget(this);
    titreWidget->setLayout(titreLayout);

    champIdentifiant = new QLineEdit(this);
    champIdentifiant->setPlaceholderText("Identifiant");
    champIdentifiant->setStyleSheet("padding: 8px; border-radius: 5px; border: 1px solid gray;");

    champMotDePasse = new QLineEdit(this);
    champMotDePasse->setPlaceholderText("Mot de passe");
    champMotDePasse->setEchoMode(QLineEdit::Password);
    champMotDePasse->setStyleSheet("padding: 8px; border-radius: 5px; border: 1px solid gray;");

    caseAfficherMotDePasse = new QCheckBox("Afficher le mot de passe", this);
    connect(caseAfficherMotDePasse, &QCheckBox::stateChanged, this, &MainWindow::toggleMotDePasse);

    QVBoxLayout *motDePasseLayout = new QVBoxLayout();
    motDePasseLayout->addWidget(champMotDePasse);
    motDePasseLayout->addWidget(caseAfficherMotDePasse);

    QWidget *motDePasseWidget = new QWidget(this);
    motDePasseWidget->setLayout(motDePasseLayout);

    boutonConnexion = new QPushButton("Se connecter", this);
    boutonConnexion->setStyleSheet("padding: 10px; border-radius: 5px; background-color: blue; color: white;");

    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(titreWidget);
    layout->addWidget(champIdentifiant);
    layout->addWidget(motDePasseWidget);
    layout->addWidget(boutonConnexion);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    connect(boutonConnexion, &QPushButton::clicked, this, &MainWindow::connectToServer);
    connect(socket, &QSslSocket::encrypted, this, &MainWindow::onSslHandshakeDone);
    connect(socket, &QSslSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(socket,
            static_cast<void (QSslSocket::*)(const QList<QSslError>&)>(&QSslSocket::sslErrors),
            this, &MainWindow::onSslErrors);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &MainWindow::onSocketError);
}

MainWindow::~MainWindow()
{
    if (socket) socket->disconnectFromHost();
}

void MainWindow::toggleMotDePasse()
{
    champMotDePasse->setEchoMode(caseAfficherMotDePasse->isChecked() ? QLineEdit::Normal : QLineEdit::Password);
}

void MainWindow::connectToServer()
{
    QString identifiant = champIdentifiant->text();
    QString motDePasse = champMotDePasse->text();
    QString code_source = "43715";
    QString trame = QString("DDC;%1;%2;%3;$")
                        .arg(code_source)
                        .arg(identifiant)
                        .arg(motDePasse);

    // Charge le certificat du serveur
    QFile certFile("/etc/ssl/certs/cert.pem");
    if(certFile.open(QIODevice::ReadOnly)) {
        QSslCertificate cert(&certFile, QSsl::Pem);
        QSslConfiguration conf = socket->sslConfiguration();
        conf.addCaCertificate(cert);
        socket->setSslConfiguration(conf);
        certFile.close();
        qDebug() << "[LOG] Certificat serveur chargé.";
    } else {
        qDebug() << "[WARN] Pas de certificat chargé, on ignore erreurs SSL (dev ONLY)";
        socket->ignoreSslErrors();
    }

    qDebug() << "[LOG] Connexion (SSL/TLS) au serveur...";
    socket->connectToHostEncrypted("192.168.15.104", 5008);

    // Stocke la trame pour l'envoyer après handshake
    socket->setProperty("trame_a_envoyer", trame);
}

void MainWindow::onSslHandshakeDone()
{
    qDebug() << "[LOG] SSL handshake ok !";
    QString trame = socket->property("trame_a_envoyer").toString();
    socket->write(trame.toUtf8());
    socket->flush();
    qDebug() << "[LOG] Trame envoyée:" << trame;
}

void MainWindow::onReadyRead()
{
    QByteArray data = socket->readAll();
    QString reponse = QString::fromUtf8(data);
    qDebug() << "[LOG] Reçu:" << reponse;

    // Traitement de la réponse du serveur
    if (reponse.startsWith("ACC")) {
        qDebug() << "Connexion réussie, ouverture du menu...";

        QWidget *nouvellePage = nullptr;
        QString identifiant = champIdentifiant->text();

        if (identifiant.toLower() == "admin") {
            nouvellePage = new MenuAdmin();
        } else if (identifiant.toLower() == "niel.otto") {
            nouvellePage = new MenuUser1();
        } else if (identifiant.toLower() == "kader.mastif") {
            nouvellePage = new MenuUser2();
        } else {
            qDebug() << "Erreur : Aucun menu trouvé pour cet identifiant ->" << identifiant;
            return;
        }

        nouvellePage->setAttribute(Qt::WA_DeleteOnClose);
        nouvellePage->show();
        this->close();
    }
}

void MainWindow::onSslErrors(const QList<QSslError> &errors)
{
    qDebug() << "[SSL ERROR] ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓";
    for (const QSslError &err : errors)
        qDebug() << "  -" << err.errorString();

    // Pour le DEV SEULEMENT (évite l'échec handshake)
    socket->ignoreSslErrors();
    // En prod, il faudra corriger le certificat !
}

void MainWindow::onSocketError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "[ERR Socket]" << socketError << socket->errorString();
}
