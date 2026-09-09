// qtphoto — une petite visionneuse pour retoucher ses photos.
//
//   o           choisir un dossier (un petit navigateur, Entrée pour choisir)
//   ← →         photo précédente / suivante
//   Suppr       effacer la photo (avec confirmation ; vers la corbeille)
//   Entrée      ouvrir la photo dans GIMP
//   Ctrl+Entrée ouvrir la photo dans Krita
//   F11         plein écran        Échap / q   quitter
//
//   qtphoto [DOSSIER]           ouvre ce dossier (sinon le dernier visité)
//   qtphoto --sonde [DOSSIER]   dit ce qu'il trouverait, sans fenêtre
#include "Visionneuse.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("qtphoto");
    QApplication::setOrganizationName("vinej");
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QStringList args = app.arguments();
    const bool sonde = args.removeAll("--sonde") > 0;
    const QString dossier = args.size() > 1 ? args.at(1) : QString();

    Visionneuse v;
    v.demarrer(dossier);
    // ⚠ La sonde ne montre rien : elle sert au banc, et à savoir ce que
    //   l'application VERRAIT dans un dossier. Elle sort avant l'écran.
    if (sonde) return v.sonde() ? 0 : 1;

    v.resize(1280, 800);
    v.show();
    return app.exec();
}
