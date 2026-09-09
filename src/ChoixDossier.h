#pragma once

#include <QDialog>
#include <QDir>

class QLabel;
class QListWidget;
class QListWidgetItem;

// Un petit navigateur de dossiers, au clavier :
//   Entrée   choisir le dossier sous le curseur (ou celui-ci)
//   → Espace descendre dedans        ← Retour   remonter
//   Échap    annuler                 une lettre : sauter au nom
class ChoixDossier : public QDialog {
    Q_OBJECT
public:
    explicit ChoixDossier(const QString& depart, QWidget* parent = nullptr);
    QString dossier() const { return m_choisi; }

protected:
    bool eventFilter(QObject*, QEvent*) override;

private:
    void remplir(const QString& chemin);
    void descendre(QListWidgetItem*);
    void monter();
    void choisir(QListWidgetItem*);
    static int nombreDePhotos(const QDir&);

    QDir         m_courant;
    QString      m_choisi;
    QLabel*      m_chemin;
    QListWidget* m_liste;
};
