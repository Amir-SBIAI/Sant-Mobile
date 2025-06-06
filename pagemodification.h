#ifndef PAGEMODIFICATION_H
#define PAGEMODIFICATION_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QSslSocket>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSslError>

class PageModification : public QWidget
{
    Q_OBJECT

public:
    explicit PageModification(QWidget *parent = nullptr);
    ~PageModification();

private slots:
    void chargerSeuils();
    void enregistrerSeuilTempAmbiante();
    void enregistrerSeuilQualiteAir();
    void enregistrerSeuilECG();
    void enregistrerSeuilTempCorp();
    void enregistrerSeuilAccelero();
    void traiterReponseServeur();
    void onSslHandshakeDone();
    void onSslErrors(const QList<QSslError> &errors);
    void onSocketError(QAbstractSocket::SocketError socketError);
    void retournerAuMenu();

private:
    void setupUI();
    void setupNetworking();
    void connectSignals();
    void createCapteurGroup(QVBoxLayout *mainLayout, const QString &nomCapteur,
                            const QString &typeCapteur, QLabel *&labelSeuils,
                            QLineEdit *&minEdit, QLineEdit *&maxEdit, QPushButton *&bouton);
    void afficherSeuils(const QString &typeCapteur, const QString &seuilMin, const QString &seuilMax);
    void reinitialiserAffichage();
    bool connectToServerSSL();
    void envoyerTrame(const QString &trame);
    void enregistrerSeuil(const QString &typeCapteur, const QString &seuilMin, const QString &seuilMax);
    QString getNomCapteur(const QString &typeCapteur);

    // Interface utilisateur
    QComboBox *selectionID;
    QPushButton *boutonChargerSeuils;
    QPushButton *boutonRetour;

    // Température ambiante
    QLabel *labelTempAmbiante;
    QLineEdit *tempAmbianteMin;
    QLineEdit *tempAmbianteMax;
    QPushButton *boutonEnregistrerTempAmbiante;

    // Qualité de l'air
    QLabel *labelQualiteAir;
    QLineEdit *qualiteAirMin;
    QLineEdit *qualiteAirMax;
    QPushButton *boutonEnregistrerQualiteAir;

    // ECG
    QLabel *labelECG;
    QLineEdit *ecgMin;
    QLineEdit *ecgMax;
    QPushButton *boutonEnregistrerECG;

    // Température corporelle
    QLabel *labelTempCorp;
    QLineEdit *tempCorpMin;
    QLineEdit *tempCorpMax;
    QPushButton *boutonEnregistrerTempCorp;

    // Accéléromètre
    QLabel *labelAccelerometre;
    QLineEdit *acceleroMin;
    QLineEdit *acceleroMax;
    QPushButton *boutonEnregistrerAccelero;

    // Réseau
    QSslSocket *socket;
    QByteArray buffer;
    QString trameEnAttente;

    enum EtatConnexion {
        ATTENTE_SEUILS,
        ATTENTE_MODIFICATION
    };
    EtatConnexion etatActuel;
};

#endif // PAGEMODIFICATION_H
