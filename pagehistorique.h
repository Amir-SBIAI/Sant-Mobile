#ifndef PAGEHISTORIQUE_H
#define PAGEHISTORIQUE_H

#include <QWidget>
#include <QWebEngineView>
#include <QPushButton>

class PageHistorique : public QWidget
{
    Q_OBJECT

public:
    explicit PageHistorique(QString mode, QWidget *parent = nullptr);
    ~PageHistorique();

private slots:
    void retournerAuMenu();

private:
    QWebEngineView *webView;
    QPushButton *boutonRetour;
    QString modeUtilisateur;
};

#endif // PAGEHISTORIQUE_H
