# qtphoto

Une petite visionneuse Qt 6 pour **retoucher ses photos** : on parcourt un
dossier au clavier, la photo emplit la fenêtre comme un fond d'écran, et
`Entrée` l'ouvre dans GIMP.

| touche | geste |
|---|---|
| `o` | choisir un dossier — un petit navigateur : `Entrée` choisit, `→`/`Espace` descend, `←` remonte |
| `←` `→` | photo précédente / suivante (boucle aux deux bouts) |
| `Suppr` | effacer la photo, **avec confirmation** — vers la corbeille ; si elle refuse, on redemande avant d'effacer pour de bon |
| `Entrée` | ouvrir la photo dans **GIMP** |
| `F11` | plein écran · `Échap` / `q` quitter |

Le dernier dossier visité est retenu. Les photos de téléphone sont
redressées (orientation EXIF).

## Bâtir

    cmake --preset linux-release
    cmake --build --preset linux-release --parallel

Qt 6 (Widgets) seulement. Le préréglage vise `/opt/qt-6.11.2` ; adapter
`CMakePresets.json` ailleurs. `qtphoto --sonde [DOSSIER]` dit ce que
l'application verrait dans un dossier, sans ouvrir de fenêtre.

## GIMP

`Entrée` appelle `/usr/bin/jyvux-gimp <photo>` s'il existe — sur JYVUX,
GIMP vit dans un conteneur podman et ce lanceur sait le joindre, même déjà
ouvert. Ailleurs : `gimp <photo>` dans le `PATH`.

## Formats

Ceux que **votre** Qt sait lire (`QImageReader::supportedImageFormats`),
demandés à l'exécution — pas une liste figée. Sur JYVUX : JPEG, PNG, GIF,
BMP (pas de TIFF, WebP ni HEIC dans les greffons de ce Qt).

MIT — Jean-Yves Vinet, 2026.
