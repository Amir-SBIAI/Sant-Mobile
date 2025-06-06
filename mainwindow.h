#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSslSocket>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void connectToServer();
    void onSslHandshakeDone();
    void onReadyRead();
    void onSslErrors(const QList<QSslError> &errors);
    void onSocketError(QAbstractSocket::SocketError socketError);
    void toggleMotDePasse();

private:
    QSslSocket *socket;
    QLineEdit *champIdentifiant;
    QLineEdit *champMotDePasse;
    QPushButton *boutonConnexion;
    QCheckBox *caseAfficherMotDePasse;
    QLabel *titre;
    QLabel *logo;
};

#endif // MAINWINDOW_H
