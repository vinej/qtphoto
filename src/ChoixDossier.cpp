#include "ChoixDossier.h"

#include <QImageReader>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

namespace {
constexpr int ROLE_CHEMIN = Qt::UserRole;      // le chemin complet
constexpr int ROLE_ICI    = Qt::UserRole + 1;  // vrai pour « ce dossier »
}

ChoixDossier::ChoixDossier(const QString& depart, QWidget* parent)
    : QDialog(parent)
    , m_chemin(new QLabel(this))
    , m_liste(new QListWidget(this))
{
    setWindowTitle("Choisir un dossier");
    resize(640, 520);

    auto* aide = new QLabel(
        "<b>Entrée</b> choisir   ·   <b>→</b> ou <b>Espace</b> descendre   ·   "
        "<b>←</b> remonter   ·   <b>Échap</b> annuler   ·   une lettre : sauter au nom", this);
    aide->setStyleSheet("color: gray;");
    m_chemin->setStyleSheet("font-weight: bold;");

    auto* v = new QVBoxLayout(this);
    v->addWidget(m_chemin);
    v->addWidget(m_liste, 1);
    v->addWidget(aide);

    m_liste->installEventFilter(this);
    connect(m_liste, &QListWidget::itemActivated, this, &ChoixDossier::choisir);

    QDir d(depart);
    remplir(d.exists() ? d.absolutePath() : QDir::homePath());
}

int ChoixDossier::nombreDePhotos(const QDir& d)
{
    QStringList motifs;
    for (const QByteArray& f : QImageReader::supportedImageFormats()) {
        const QString ext = QString::fromLatin1(f).toLower();
        if (ext == "pdf" || ext == "svg" || ext == "svgz" || ext == "ico" || ext == "cur") continue;
        motifs << ("*." + ext) << ("*." + ext.toUpper());
    }
    return d.entryList(motifs, QDir::Files).size();
}

void ChoixDossier::remplir(const QString& chemin)
{
    m_courant = QDir(chemin);
    m_chemin->setText(m_courant.absolutePath());
    m_liste->clear();

    // « ce dossier » d'abord : Entrée dessus le choisit tel quel.
    auto* ici = new QListWidgetItem(
        QString("▶  ce dossier   (%1 photos)").arg(nombreDePhotos(m_courant)), m_liste);
    ici->setData(ROLE_CHEMIN, m_courant.absolutePath());
    ici->setData(ROLE_ICI, true);

    if (!m_courant.isRoot()) {
        auto* haut = new QListWidgetItem("..", m_liste);
        QDir parent = m_courant; parent.cdUp();
        haut->setData(ROLE_CHEMIN, parent.absolutePath());
    }

    // ⚠ Les liens vers des dossiers sont gardés : ~/images/SharedPictures en est un.
    const auto sous = m_courant.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot,
                                              QDir::Name | QDir::IgnoreCase);
    for (const QFileInfo& fi : sous) {
        const int n = nombreDePhotos(QDir(fi.absoluteFilePath()));
        auto* it = new QListWidgetItem(
            n > 0 ? QString("%1   (%2 photos)").arg(fi.fileName()).arg(n) : fi.fileName(), m_liste);
        it->setData(ROLE_CHEMIN, fi.absoluteFilePath());
        if (n == 0) it->setForeground(Qt::gray);
    }
    m_liste->setCurrentRow(0);
    m_liste->setFocus();
}

void ChoixDossier::descendre(QListWidgetItem* it)
{
    if (!it || it->data(ROLE_ICI).toBool()) return;
    remplir(it->data(ROLE_CHEMIN).toString());
}

void ChoixDossier::monter()
{
    if (m_courant.isRoot()) return;
    QDir parent = m_courant; parent.cdUp();
    const QString ancien = m_courant.dirName();
    remplir(parent.absolutePath());
    // Le curseur revient sur le dossier qu'on vient de quitter.
    for (int i = 0; i < m_liste->count(); ++i)
        if (QFileInfo(m_liste->item(i)->data(ROLE_CHEMIN).toString()).fileName() == ancien) {
            m_liste->setCurrentRow(i); break;
        }
}

void ChoixDossier::choisir(QListWidgetItem* it)
{
    if (!it) return;
    if (it->text() == "..") { monter(); return; }     // Entrée sur « .. » remonte
    m_choisi = it->data(ROLE_CHEMIN).toString();
    accept();
}

bool ChoixDossier::eventFilter(QObject* o, QEvent* e)
{
    if (o == m_liste && e->type() == QEvent::KeyPress) {
        auto* k = static_cast<QKeyEvent*>(e);
        switch (k->key()) {
        case Qt::Key_Return: case Qt::Key_Enter: choisir(m_liste->currentItem());   return true;
        case Qt::Key_Right:  case Qt::Key_Space: descendre(m_liste->currentItem()); return true;
        case Qt::Key_Left:   case Qt::Key_Backspace: monter();                     return true;
        case Qt::Key_Escape: reject();                                             return true;
        default: break;
        }
    }
    return QDialog::eventFilter(o, e);
}
