#include "Visionneuse.h"
#include "ChoixDossier.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QSettings>
#include <QTextStream>

// ── construction ─────────────────────────────────────────────────────────

Visionneuse::Visionneuse(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("qtphoto");
    // Comme un fond d'écran : noir autour, la photo au centre.
    QPalette p = palette();
    p.setColor(QPalette::Window, Qt::black);
    setPalette(p);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(320, 240);
}

void Visionneuse::demarrer(const QString& dossier)
{
    QString d = dossier;
    if (d.isEmpty()) d = QSettings().value("dernierDossier").toString();
    if (d.isEmpty() || !QDir(d).exists()) d = QDir::homePath() + "/images";
    if (!QDir(d).exists()) d = QDir::homePath();
    ouvrirDossier(d);
}

// Les formats que CE Qt sait lire — pas une liste écrite d'avance.
QStringList Visionneuse::motifsSupportes()
{
    QStringList motifs;
    for (const QByteArray& f : QImageReader::supportedImageFormats()) {
        const QString ext = QString::fromLatin1(f).toLower();
        if (ext == "pdf" || ext == "svg" || ext == "svgz" || ext == "ico" || ext == "cur") continue;   // pas des photos
        motifs << ("*." + ext) << ("*." + ext.toUpper());
    }
    return motifs;
}

void Visionneuse::ouvrirDossier(const QString& dossier)
{
    QDir dir(dossier);
    if (!dir.exists()) { dire("Dossier introuvable : " + dossier); return; }
    m_dossier = dir.absolutePath();
    m_photos  = dir.entryList(motifsSupportes(), QDir::Files | QDir::Readable,
                              QDir::Name | QDir::IgnoreCase);
    m_photos.removeDuplicates();         // *.jpg et *.JPG peuvent se recouper
    m_index = m_photos.isEmpty() ? -1 : 0;
    QSettings().setValue("dernierDossier", m_dossier);
    m_message.clear();
    charger();
}

// ── la photo ──────────────────────────────────────────────────────────────

QString Visionneuse::cheminCourant() const
{
    if (m_index < 0 || m_index >= m_photos.size()) return {};
    return m_dossier + "/" + m_photos.at(m_index);
}

void Visionneuse::charger()
{
    m_original = QPixmap();
    m_ajustee  = QPixmap();
    const QString chemin = cheminCourant();
    if (!chemin.isEmpty()) {
        QImageReader lecteur(chemin);
        // 🔴 L'orientation EXIF : sans elle, les photos de téléphone
        //   arrivent couchées.
        lecteur.setAutoTransform(true);
        const QImage img = lecteur.read();
        if (!img.isNull()) m_original = QPixmap::fromImage(img);
        else m_message = "Illisible : " + lecteur.errorString();
    }
    setWindowTitle(chemin.isEmpty()
        ? QString("qtphoto — %1").arg(m_dossier)
        : QString("qtphoto — %1  (%2/%3)").arg(m_photos.at(m_index))
                                           .arg(m_index + 1).arg(m_photos.size()));
    update();
}

void Visionneuse::aller(int delta)
{
    if (m_photos.isEmpty()) return;
    const int n = m_photos.size();
    m_index = ((m_index + delta) % n + n) % n;        // boucle aux deux bouts
    m_message.clear();
    charger();
}

void Visionneuse::dire(const QString& message)
{
    m_message = message;
    update();
}

// ── le dessin : la photo ajustée à la fenêtre, centrée, sur noir ──────────

void Visionneuse::resizeEvent(QResizeEvent*) { m_ajustee = QPixmap(); }

void Visionneuse::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), Qt::black);

    if (!m_original.isNull()) {
        // ⭐ Une seule mise à l'échelle par taille de fenêtre : redessiner
        //   ne recalcule rien.
        if (m_ajustee.isNull() || m_tailleAjustee != size()) {
            m_ajustee = m_original.scaled(size(), Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);
            m_tailleAjustee = size();
        }
        const QPoint coin((width() - m_ajustee.width()) / 2,
                          (height() - m_ajustee.height()) / 2);
        p.drawPixmap(coin, m_ajustee);
    }

    // Le bandeau du bas : nom, rang, dimensions — ou l'aide quand c'est vide.
    QString texte;
    if (m_index >= 0)
        texte = QString("%1   ·   %2 / %3   ·   %4 × %5")
                    .arg(m_photos.at(m_index)).arg(m_index + 1).arg(m_photos.size())
                    .arg(m_original.width()).arg(m_original.height());
    else
        texte = QString("Aucune photo dans %1   —   o : choisir un dossier").arg(m_dossier);
    if (!m_message.isEmpty()) texte += "      " + m_message;

    QFont f = p.font(); f.setPointSize(11); p.setFont(f);
    const int h = p.fontMetrics().height() + 12;
    p.fillRect(0, height() - h, width(), h, QColor(0, 0, 0, 160));
    p.setPen(Qt::white);
    p.drawText(QRect(12, height() - h, width() - 24, h), Qt::AlignVCenter | Qt::AlignLeft,
               p.fontMetrics().elidedText(texte, Qt::ElideMiddle, width() - 24));

    if (m_index < 0) {
        p.setPen(QColor(200, 200, 200));
        f.setPointSize(16); p.setFont(f);
        p.drawText(rect().adjusted(0, 0, 0, -h), Qt::AlignCenter,
            "o  choisir un dossier          ←  →  parcourir\n"
            "Entrée  ouvrir dans GIMP        Suppr  effacer        F11  plein écran");
    }
}

// ── les touches ───────────────────────────────────────────────────────────

void Visionneuse::keyPressEvent(QKeyEvent* e)
{
    switch (e->key()) {
    case Qt::Key_Left:  case Qt::Key_Up:   case Qt::Key_PageUp:   case Qt::Key_Backspace:
        aller(-1); break;
    case Qt::Key_Right: case Qt::Key_Down: case Qt::Key_PageDown: case Qt::Key_Space:
        aller(+1); break;
    case Qt::Key_Home: if (!m_photos.isEmpty()) { m_index = 0; charger(); } break;
    case Qt::Key_End:  if (!m_photos.isEmpty()) { m_index = m_photos.size() - 1; charger(); } break;
    case Qt::Key_O:      choisirDossier(); break;
    case Qt::Key_Delete: supprimer(); break;
    case Qt::Key_Return: case Qt::Key_Enter: ouvrirDansGimp(); break;
    case Qt::Key_F11: case Qt::Key_F:
        isFullScreen() ? showNormal() : showFullScreen(); break;
    case Qt::Key_Escape:
        if (isFullScreen()) showNormal(); else close(); break;
    case Qt::Key_Q: close(); break;
    default: QWidget::keyPressEvent(e);
    }
}

void Visionneuse::choisirDossier()
{
    ChoixDossier dlg(m_dossier, this);
    if (dlg.exec() == QDialog::Accepted) ouvrirDossier(dlg.dossier());
}

// ── effacer : vers la CORBEILLE, et jamais sans demander ─────────────────

void Visionneuse::supprimer()
{
    const QString chemin = cheminCourant();
    if (chemin.isEmpty()) return;
    const QString nom = m_photos.at(m_index);

    if (QMessageBox::question(this, "Effacer la photo",
            QString("Mettre « %1 » à la corbeille ?").arg(nom),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;

    QFile f(chemin);
    bool parti = f.moveToTrash();
    if (!parti) {
        // ⚠ La corbeille peut refuser (un autre volume, un montage NTFS…).
        //   On le DIT, et on redemande : effacer pour de bon est un autre geste.
        if (QMessageBox::warning(this, "Corbeille impossible",
                QString("La corbeille a refusé « %1 » (%2).\n\nL'effacer DÉFINITIVEMENT ?")
                    .arg(nom, f.errorString()),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
            parti = f.remove();
    }
    if (!parti) { dire("Pas effacée : " + f.errorString()); return; }

    m_photos.removeAt(m_index);
    if (m_photos.isEmpty()) m_index = -1;
    else if (m_index >= m_photos.size()) m_index = m_photos.size() - 1;
    m_message = "Effacée : " + nom;
    charger();
}

// ── GIMP : par le lanceur de JYVUX, qui sait où GIMP vit (podman) ────────

void Visionneuse::ouvrirDansGimp()
{
    const QString chemin = cheminCourant();
    if (chemin.isEmpty()) return;
    // ⭐ On ne sait pas ici comment GIMP est installé — et on n'a pas à le
    //   savoir : /usr/bin/jyvux-gimp le sait (conteneur, écran, rôle).
    //   Repli : un « gimp » dans le PATH.
    const QString lanceur = QFile::exists("/usr/bin/jyvux-gimp") ? "/usr/bin/jyvux-gimp" : "gimp";
    if (QProcess::startDetached(lanceur, {chemin}))
        dire("→ GIMP");
    else
        QMessageBox::warning(this, "GIMP", "Impossible de lancer " + lanceur);
}

// ── la sonde ──────────────────────────────────────────────────────────────

bool Visionneuse::sonde() const
{
    QTextStream out(stdout);
    out << "dossier : " << m_dossier << "\n"
        << "formats : " << motifsSupportes().filter(QRegularExpression("^\\*\\.[a-z]")).join(' ') << "\n"
        << "photos  : " << m_photos.size() << "\n";
    if (!m_photos.isEmpty()) {
        out << "premiere: " << m_photos.first() << "  " << m_original.width() << "x" << m_original.height() << "\n"
            << "derniere: " << m_photos.last() << "\n";
    }
    out.flush();
    return !m_photos.isEmpty();
}
