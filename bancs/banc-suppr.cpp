// Le banc de la touche Suppr — le seul geste qui DÉTRUIT.
//
// Sans fenêtre (offscreen), sur un dossier jetable : trois copies d'une
// photo, on efface celle du milieu, on vérifie qu'elle est dans la
// CORBEILLE (pas perdue), que la liste a bougé, et que ← → bouclent.
// 🔴 Un banc doit pouvoir ÉCHOUER : chaque étape compare à un attendu.
#include "../src/Visionneuse.h"

#include <QApplication>
#include <QAbstractButton>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QKeyEvent>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimer>
#include <cstdio>

static int g_rouges = 0;
static void verifie(const char* quoi, bool ok)
{ std::printf("  [%s] %s\n", ok ? "VERT " : "ROUGE", quoi); if (!ok) ++g_rouges; }

static void pompe(int ms)
{ QElapsedTimer t; t.start(); while (t.elapsed() < ms) QCoreApplication::processEvents(QEventLoop::AllEvents, 20); }

static void touche(QWidget* w, int k)
{ QKeyEvent e(QEvent::KeyPress, k, Qt::NoModifier); QCoreApplication::sendEvent(w, &e); }

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("qtphoto-banc");
    QApplication::setOrganizationName("vinej");

    // ⚠ Les dialogues ouvrent un event loop IMBRIQUÉ : seul un vrai QTimer
    //   y répond (la leçon du banc de qt_finance).
    QMessageBox::StandardButton reponse = QMessageBox::Yes;
    QTimer huissier;
    QObject::connect(&huissier, &QTimer::timeout, [&] {
        if (auto* mb = qobject_cast<QMessageBox*>(QApplication::activeModalWidget()))
            if (auto* b = mb->button(reponse)) b->click();
    });
    huissier.start(30);

    const QString source = argc > 1 ? argv[1] : QString();
    if (source.isEmpty() || !QFile::exists(source)) { std::printf("usage : banc-suppr PHOTO.jpg\n"); return 2; }

    // 🔴 DANS SON FOYER, pas sous /tmp : la corbeille freedesktop est PAR
    //   VOLUME. Sous /tmp (un autre système de fichiers), Qt aurait rangé
    //   b.jpg dans /tmp/.Trash-1000 — et le banc, qui regardait le foyer,
    //   a rougi alors que rien n'était perdu. Ses photos sont sous ~/images :
    //   c'est ce chemin-là qu'on éprouve.
    QTemporaryDir bac(QDir::homePath() + "/.qtphoto-banc-XXXXXX");
    if (!bac.isValid()) { std::printf("ROUGE : pas de dossier jetable dans le foyer\n"); return 2; }
    for (const char* n : {"a.jpg", "b.jpg", "c.jpg"}) QFile::copy(source, bac.filePath(n));

    Visionneuse v;
    v.ouvrirDossier(bac.path());
    pompe(100);

    std::printf("== 1. trois photos, la première est a.jpg ==\n");
    verifie("le titre porte (1/3)", v.windowTitle().contains("(1/3)"));

    std::printf("== 2. → puis ← : on boucle ==\n");
    touche(&v, Qt::Key_Right); verifie("→ donne 2/3", v.windowTitle().contains("(2/3)"));
    touche(&v, Qt::Key_Left);  touche(&v, Qt::Key_Left);
    verifie("← ← depuis la 2e boucle à la 3e", v.windowTitle().contains("(3/3)"));
    touche(&v, Qt::Key_Right); verifie("→ depuis la 3e revient à la 1re", v.windowTitle().contains("(1/3)"));

    std::printf("== 3. Suppr sur b.jpg, on répond OUI ==\n");
    touche(&v, Qt::Key_Right);
    reponse = QMessageBox::Yes;
    touche(&v, Qt::Key_Delete); pompe(300);
    verifie("b.jpg n'est plus dans le dossier", !QFile::exists(bac.filePath("b.jpg")));
    const QString corbeille = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/Trash/files";
    bool dansCorbeille = false;
    for (const QString& f : QDir(corbeille).entryList(QDir::Files))
        if (f.startsWith("b") && f.endsWith(".jpg")) dansCorbeille = true;
    verifie("…mais il est dans la CORBEILLE, pas perdu", dansCorbeille);
    verifie("la liste est passée à 2 et montre c.jpg", v.windowTitle().contains("c.jpg") && v.windowTitle().contains("(2/2)"));

    std::printf("== 4. Suppr, on répond NON : rien ne bouge ==\n");
    reponse = QMessageBox::No;
    touche(&v, Qt::Key_Delete); pompe(300);
    verifie("c.jpg est toujours là", QFile::exists(bac.filePath("c.jpg")));
    verifie("toujours 2 photos", v.windowTitle().contains("(2/2)"));

    // Ménage : ce que le banc a mis à la corbeille, il l'en retire.
    for (const QString& f : QDir(corbeille).entryList({"b*.jpg"}, QDir::Files)) {
        QFile::remove(corbeille + "/" + f);
        QFile::remove(QDir(corbeille).absolutePath() + "/../info/" + f + ".trashinfo");
    }

    std::printf("\n%s (%d rouge(s))\n", g_rouges ? "== ROUGE ==" : "== TOUT VERT ==", g_rouges);
    return g_rouges ? 1 : 0;
}
