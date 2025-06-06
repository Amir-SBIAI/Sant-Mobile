#include "pagemodification.h"
#include "menuadmin.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QSslCertificate>
#include <QSslConfiguration>

PageModification::PageModification(QWidget *parent)
    : QWidget(parent), socket(nullptr), trameEnAttente(""), etatActuel(ATTENTE_SEUILS)
{
    setupUI();
    setupNetworking();
    connectSignals();
}

PageModification::~PageModification()
{
    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
        socket->disconnectFromHost();
    }
}

void PageModification::setupUI()
{
    setWindowTitle("Modification des Seuils");
    setFixedSize(800, 700);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // En-tête
    QLabel *titre = new QLabel("MODIFICATION DES SEUILS");
    titre->setAlignment(Qt::AlignCenter);
    titre->setStyleSheet("font-size: 24px; font-weight: bold; margin: 10px; color: #2c3e50;");
    mainLayout->addWidget(titre);

    // Sélection ID Tutelle
    QHBoxLayout *selectionLayout = new QHBoxLayout();
    selectionLayout->addWidget(new QLabel("ID Tutelle:"));

    selectionID = new QComboBox();
    for (int i = 1; i <= 10; ++i) {
        selectionID->addItem(QString::number(i));
    }
    selectionLayout->addWidget(selectionID);

    boutonChargerSeuils = new QPushButton("Charger les seuils actuels");
    boutonChargerSeuils->setStyleSheet("QPushButton { background-color: #3498db; color: white; padding: 8px; border-radius: 4px; }");
    selectionLayout->addWidget(boutonChargerSeuils);

    mainLayout->addLayout(selectionLayout);
    mainLayout->addSpacing(20);

    // Groupes pour chaque capteur
    createCapteurGroup(mainLayout, "Température ambiante", "temp_ambiante",
                       labelTempAmbiante, tempAmbianteMin, tempAmbianteMax, boutonEnregistrerTempAmbiante);

    createCapteurGroup(mainLayout, "Qualité de l'air", "qualite_lair",
                       labelQualiteAir, qualiteAirMin, qualiteAirMax, boutonEnregistrerQualiteAir);

    createCapteurGroup(mainLayout, "Électrocardiogramme (ECG)", "ecg",
                       labelECG, ecgMin, ecgMax, boutonEnregistrerECG);

    createCapteurGroup(mainLayout, "Température corporelle", "temp_corp",
                       labelTempCorp, tempCorpMin, tempCorpMax, boutonEnregistrerTempCorp);

    createCapteurGroup(mainLayout, "Accéléromètre", "accelerometre",
                       labelAccelerometre, acceleroMin, acceleroMax, boutonEnregistrerAccelero);

    mainLayout->addSpacing(20);

    // Bouton retour
    boutonRetour = new QPushButton("Retour au Menu");
    boutonRetour->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; padding: 10px; border-radius: 4px; }");
    mainLayout->addWidget(boutonRetour);
}

void PageModification::createCapteurGroup(QVBoxLayout *mainLayout, const QString &nomCapteur,
                                          const QString &typeCapteur, QLabel *&labelSeuils,
                                          QLineEdit *&minEdit, QLineEdit *&maxEdit, QPushButton *&bouton)
{
    QGroupBox *group = new QGroupBox(nomCapteur);
    group->setStyleSheet("QGroupBox { font-weight: bold; margin: 5px; padding-top: 15px; }");

    QVBoxLayout *groupLayout = new QVBoxLayout(group);

    // Label pour afficher les seuils actuels
    labelSeuils = new QLabel("Seuils actuels: Non chargés");
    labelSeuils->setStyleSheet("color: #7f8c8d; font-style: italic;");
    groupLayout->addWidget(labelSeuils);

    // Champs de saisie
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(new QLabel("Min:"));
    minEdit = new QLineEdit();
    minEdit->setPlaceholderText("Valeur min");
    inputLayout->addWidget(minEdit);

    inputLayout->addWidget(new QLabel("Max:"));
    maxEdit = new QLineEdit();
    maxEdit->setPlaceholderText("Valeur max");
    inputLayout->addWidget(maxEdit);

    bouton = new QPushButton("Enregistrer");
    bouton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; padding: 5px; border-radius: 3px; }");
    inputLayout->addWidget(bouton);

    groupLayout->addLayout(inputLayout);
    mainLayout->addWidget(group);
}

void PageModification::setupNetworking()
{
    socket = new QSslSocket(this);

    // Configuration SSL
    QFile certFile("/etc/ssl/certs/cert.pem");
    if(certFile.open(QIODevice::ReadOnly)) {
        QSslCertificate cert(&certFile, QSsl::Pem);
        QSslConfiguration conf = socket->sslConfiguration();
        conf.addCaCertificate(cert);
        conf.setPeerVerifyMode(QSslSocket::VerifyNone); // Pour ignorer l'erreur hostname
        socket->setSslConfiguration(conf);
        certFile.close();
        qDebug() << "[SSL] Certificat serveur chargé.";
    } else {
        qDebug() << "[SSL WARNING] Certificat non trouvé, erreurs SSL ignorées";
        socket->ignoreSslErrors();
    }
}

void PageModification::connectSignals()
{
    connect(boutonChargerSeuils, &QPushButton::clicked, this, &PageModification::chargerSeuils);
    connect(boutonEnregistrerTempAmbiante, &QPushButton::clicked, this, &PageModification::enregistrerSeuilTempAmbiante);
    connect(boutonEnregistrerQualiteAir, &QPushButton::clicked, this, &PageModification::enregistrerSeuilQualiteAir);
    connect(boutonEnregistrerECG, &QPushButton::clicked, this, &PageModification::enregistrerSeuilECG);
    connect(boutonEnregistrerTempCorp, &QPushButton::clicked, this, &PageModification::enregistrerSeuilTempCorp);
    connect(boutonEnregistrerAccelero, &QPushButton::clicked, this, &PageModification::enregistrerSeuilAccelero);
    connect(boutonRetour, &QPushButton::clicked, this, &PageModification::retournerAuMenu);

    // Signaux SSL
    connect(socket, &QSslSocket::encrypted, this, &PageModification::onSslHandshakeDone);
    connect(socket, &QSslSocket::readyRead, this, &PageModification::traiterReponseServeur);
    connect(socket, QOverload<const QList<QSslError>&>::of(&QSslSocket::sslErrors),
            this, &PageModification::onSslErrors);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &PageModification::onSocketError);
}

bool PageModification::connectToServerSSL()
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        return true;
    }

    qDebug() << "[SSL] Connexion au serveur 192.168.15.104:5008...";
    socket->connectToHostEncrypted("192.168.15.104", 5008);

    return socket->waitForConnected(5000);
}

void PageModification::envoyerTrame(const QString &trame)
{
    if (!connectToServerSSL()) {
        QMessageBox::critical(this, "Erreur", "Impossible de se connecter au serveur");
        return;
    }

    if (socket->isEncrypted()) {
        qDebug() << "[TRAME ENVOYÉE]:" << trame;
        socket->write(trame.toUtf8());
        socket->flush();
    } else {
        trameEnAttente = trame;
        qDebug() << "[SSL] Trame en attente du handshake:" << trame;
    }
}

void PageModification::chargerSeuils()
{
    reinitialiserAffichage();
    etatActuel = ATTENTE_SEUILS;

    QString idTutelle = selectionID->currentText();
    QString trame = QString("SEUIL;12345;%1;$").arg(idTutelle);

    qDebug() << "[DEMANDE SEUILS] Envoi pour ID tutelle:" << idTutelle;
    envoyerTrame(trame);
}

void PageModification::enregistrerSeuilTempAmbiante()
{
    enregistrerSeuil("temp_ambiante", tempAmbianteMin->text(), tempAmbianteMax->text());
}

void PageModification::enregistrerSeuilQualiteAir()
{
    enregistrerSeuil("qualite_lair", qualiteAirMin->text(), qualiteAirMax->text());
}

void PageModification::enregistrerSeuilECG()
{
    enregistrerSeuil("ecg", ecgMin->text(), ecgMax->text());
}

void PageModification::enregistrerSeuilTempCorp()
{
    enregistrerSeuil("temp_corp", tempCorpMin->text(), tempCorpMax->text());
}

void PageModification::enregistrerSeuilAccelero()
{
    enregistrerSeuil("accelerometre", acceleroMin->text(), acceleroMax->text());
}

void PageModification::enregistrerSeuil(const QString &typeCapteur, const QString &seuilMin, const QString &seuilMax)
{
    if (seuilMin.isEmpty() || seuilMax.isEmpty()) {
        QMessageBox::warning(this, "Erreur", "Veuillez remplir les valeurs min et max pour " + getNomCapteur(typeCapteur));
        return;
    }

    etatActuel = ATTENTE_MODIFICATION;

    QString idTutelle = selectionID->currentText();
    QString trame = QString("MODIFSEUIL;%1;%2;%3;%4;$")
                        .arg(idTutelle)
                        .arg(typeCapteur)
                        .arg(seuilMin)
                        .arg(seuilMax);

    qDebug() << "[MODIFICATION SEUIL] Type:" << typeCapteur << "Min:" << seuilMin << "Max:" << seuilMax;
    envoyerTrame(trame);
}

void PageModification::traiterReponseServeur()
{
    buffer += socket->readAll();
    qDebug() << "[DONNÉES REÇUES]:" << buffer;

    // Traiter toutes les trames complètes dans le buffer
    while (buffer.contains('$')) {
        int finTrame = buffer.indexOf('$');
        QString trame = buffer.left(finTrame);
        buffer.remove(0, finTrame + 1);

        qDebug() << "[TRAME COMPLÈTE REÇUE]:" << trame;

        QStringList parties = trame.split(';');

        if (etatActuel == ATTENTE_SEUILS) {
            // Traitement des seuils reçus
            if (parties.size() >= 3 && parties[0] == "REP") {
                if (parties.size() >= 5) {
                    QString typeCapteur = parties[2];
                    QString seuilMin = parties[3];
                    QString seuilMax = parties[4];

                    qDebug() << "[SEUILS REÇUS] Capteur:" << typeCapteur << "Min:" << seuilMin << "Max:" << seuilMax;
                    afficherSeuils(typeCapteur, seuilMin, seuilMax);
                }
            } else {
                qDebug() << "[SEUILS] Réponse inattendue:" << trame;
                QMessageBox::information(this, "Réponse serveur", "Réponse: " + trame);
            }

        } else if (etatActuel == ATTENTE_MODIFICATION) {
            // Confirmation de modification
            if (parties.size() >= 1) {
                if (parties[0] == "OK") {
                    qDebug() << "[MODIF] Modification réussie";
                    QMessageBox::information(this, "Succès", "Seuils modifiés avec succès !");

                    // Recharger les seuils automatiquement
                    chargerSeuils();

                } else {
                    qDebug() << "[MODIF] Réponse:" << trame;
                    QMessageBox::information(this, "Réponse serveur", "Réponse: " + trame);
                }
            }
        }
    }
}

void PageModification::afficherSeuils(const QString &typeCapteur, const QString &seuilMin, const QString &seuilMax)
{
    QString texte = QString("Seuils actuels: Min = %1, Max = %2").arg(seuilMin).arg(seuilMax);
    QString style = "color: #27ae60; font-weight: bold;";

    if (typeCapteur == "temp_ambiante") {
        labelTempAmbiante->setText(texte);
        labelTempAmbiante->setStyleSheet(style);
    } else if (typeCapteur == "qualite_lair") {
        labelQualiteAir->setText(texte);
        labelQualiteAir->setStyleSheet(style);
    } else if (typeCapteur == "ecg") {
        labelECG->setText(texte);
        labelECG->setStyleSheet(style);
    } else if (typeCapteur == "temp_corp") {
        labelTempCorp->setText(texte);
        labelTempCorp->setStyleSheet(style);
    } else if (typeCapteur == "accelerometre") {
        labelAccelerometre->setText(texte);
        labelAccelerometre->setStyleSheet(style);
    }
}

void PageModification::reinitialiserAffichage()
{
    QString styleChargement = "color: #f39c12; font-style: italic;";

    labelTempAmbiante->setText("Chargement des seuils...");
    labelQualiteAir->setText("Chargement des seuils...");
    labelECG->setText("Chargement des seuils...");
    labelTempCorp->setText("Chargement des seuils...");
    labelAccelerometre->setText("Chargement des seuils...");

    labelTempAmbiante->setStyleSheet(styleChargement);
    labelQualiteAir->setStyleSheet(styleChargement);
    labelECG->setStyleSheet(styleChargement);
    labelTempCorp->setStyleSheet(styleChargement);
    labelAccelerometre->setStyleSheet(styleChargement);
}

void PageModification::onSslHandshakeDone()
{
    qDebug() << "[SSL] Handshake SSL terminé avec succès !";

    if (!trameEnAttente.isEmpty()) {
        qDebug() << "[SSL] Envoi de la trame en attente:" << trameEnAttente;
        socket->write(trameEnAttente.toUtf8());
        socket->flush();
        trameEnAttente.clear();
    }
}

void PageModification::onSslErrors(const QList<QSslError> &errors)
{
    qDebug() << "[SSL ERREUR] Erreurs SSL détectées:";
    for (const QSslError &err : errors) {
        qDebug() << "  -" << err.errorString();
    }

    // Pour le développement uniquement
    socket->ignoreSslErrors();
    qDebug() << "[SSL WARNING] Erreurs SSL ignorées (développement)";
}

void PageModification::onSocketError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "[ERREUR SOCKET]" << socketError << socket->errorString();

    // Ne pas afficher d'erreur pour RemoteHostClosedError car ça peut être normal
    if (socketError != QAbstractSocket::RemoteHostClosedError) {
        QMessageBox::critical(this, "Erreur de connexion",
                              "Erreur de connexion: " + socket->errorString());
    } else {
        qDebug() << "[INFO] Connexion fermée par le serveur (normal après traitement)";
    }
}

QString PageModification::getNomCapteur(const QString &typeCapteur)
{
    if (typeCapteur == "temp_ambiante") return "Température ambiante";
    if (typeCapteur == "qualite_lair") return "Qualité de l'air";
    if (typeCapteur == "ecg") return "Électrocardiogramme";
    if (typeCapteur == "temp_corp") return "Température corporelle";
    if (typeCapteur == "accelerometre") return "Accéléromètre";
    return typeCapteur;
}

void PageModification::retournerAuMenu()
{
    qDebug() << "Retour au menu principal...";
    MenuAdmin *menu = new MenuAdmin();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->show();
    this->close();
}
