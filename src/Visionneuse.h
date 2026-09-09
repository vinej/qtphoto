#pragma once

#include <QPixmap>
#include <QStringList>
#include <QWidget>

class Visionneuse : public QWidget {
    Q_OBJECT
public:
    explicit Visionneuse(QWidget* parent = nullptr);

    // Ouvre DOSSIER, ou le dernier visité, ou ~/images.
    void demarrer(const QString& dossier);
    void ouvrirDossier(const QString& dossier);
    // Écrit sur la sortie standard ce qui serait affiché. Vrai s'il y a des photos.
    bool sonde() const;

protected:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    void charger();
    void aller(int delta);
    void choisirDossier();
    void supprimer();
    void ouvrirDans(const QString& app);   // « gimp », « krita »…
    void dire(const QString& message);
    QString cheminCourant() const;
    static QStringList motifsSupportes();

    QString     m_dossier;
    QStringList m_photos;
    int         m_index = -1;
    QPixmap     m_original;      // la photo telle que lue (orientation EXIF appliquée)
    QPixmap     m_ajustee;       // la même, à la taille de la fenêtre
    QSize       m_tailleAjustee;
    QString     m_message;       // un mot en bas, effacé au prochain geste
};
