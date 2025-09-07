Francesco Falcon Sm3201408

# Raytracer – Parte 1 (C)

Raytracer in C (ray casting di sfere) con OpenMP e output PPM (P6) scritto via memory mapping.

Questo progetto implementa esattamente quanto richiesto dal compito (PDF/screenshot):
- Linguaggio C (C99), aritmetica in precisione singola (`float`).
- Parser testuale del file di scena nel formato indicato.
- Ray casting di sole sfere; colore pieno dell’oggetto intersecato o background.
- Parallelizzazione con OpenMP (`#pragma omp parallel for`).
- Scrittura dell’immagine nel formato PPM binario (P6) tramite memoria mappata.
- Build tramite `Makefile`.

## Quick start
- Se hai già compilato: su Windows apri PowerShell nella root del progetto e lancia `./RUN.ps1 -Width 320 -Height 180` per generare un PPM e (se disponibile) un PNG nelle cartelle `ppms/` e `renders/`.
- Altrimenti, segui la sezione "Compilazione" (MSYS2/MinGW su Windows, oppure WSL/Ubuntu) e poi esegui.

Indice
- Requisiti del compito e copertura
- Struttura del progetto
- Formato del file di scena
- Modello geometrico e intersezioni
- Formato immagine PPM (P6)
- Compilazione (Windows MSYS2/MinGW, WSL/Ubuntu)
- Esecuzione e esempi
- Script di supporto (facoltativi)
- Gestione errori e risorse
- Verifica rapida (smoke test) e qualità
- Troubleshooting
- Note per la consegna

## Requisiti del compito e copertura
- C99 e `float` per tutta la matematica. 
- Parser scena con `fscanf` e formato richiesto. 
- Ray casting sfere, nessuna illuminazione/ombre avanzate. 
- Parallelizzazione con OpenMP. 
- Salvataggio PPM (P6) via memoria mappata. 
- Makefile fornito. 

## Struttura del progetto
Contenuti del .zip:
- `main.c` — Entry point: parsing CLI, caricamento scena, render parallelo, scrittura PPM.
- `scene.h/.c` — Tipi (viewport, sphere, scene) e funzioni: parser, intersezione raggio–sfera, colore pixel.
- `ppm.h/.c` — Writer PPM (P6) con memoria mappata (Windows/Posix).
- `Makefile` — Regole di build (`-fopenmp`).

Contenuti aggiuntivi della repo:
- `scenes/` — File di scena di esempio (incluso `scene_pdf_example.txt`).
- `ppms/` — Output PPM generati (runtime; non richiesti in consegna).
- `renders/` — PNG ricavati dai PPM (runtime; opzionale).
- `scripts/` — Script PowerShell di supporto (opzionali per la consegna).

## Formato del file di scena
Righe testuali con i seguenti tag (ordine libero, spazi come separatori):
- `VP <x> <y> <z>` — Dimensioni viewport e distanza dal pinhole (camera in origine).
- `BG <R> <G> <B>` — Colore di background (0–255 per canale).
- `OBJ_N <n>` — Numero di sfere che seguono.
- `S <cx> <cy> <cz> <r> <R> <G> <B>` — Sfera: centro, raggio, colore (0–255).

Esempio (come nel PDF):

VP  1.777 1 1
BG 255 255 255
OBJ_N 16
S 0.2 1 4 0.2 200 0 0
S 0.2 1.5 5 0.4 150 0 0
S 0.2 2 6 0.6 100 0 0
S 0.2 2.5 7 0.8 50 0 0
S 0 0 3 0.2 127 0 0
S 0 0 10 1 255 0 0
S 2 -2 8 0.5 0 200 0
S 1 -3 8 0.5 0 180 0
S 0 -4 8 0.5 0 160 0
S -1 -5 8 0.5 0 140 0
S 1 0 7 0.25 0 0 255
S 1 0 5 0.25 0 0 225
S 1 0 3 0.25 0 0 195
S 1 0 1 0.25 0 0 165
S -35 0 100 20 127 127 127
S -100 0 10000 3000 255 255 0

La repository include `scenes/scene_pdf_example.txt` con il contenuto coerente.

## Modello geometrico e intersezioni (riassunto)
- Camera in `(0,0,0)`.
- Viewport: rettangolo centrato sullo z, perpendicolare all’asse z, di dimensioni `vp.x`×`vp.y` alla distanza `vp.z`.
- Per ogni pixel `(i,j)` di un’immagine `width×height`:
  - Punto sul viewport: `x = ((i+0.5) - width/2)/width * vp.x`;
    `y = -(((j+0.5) - height/2)/height) * vp.y`;
    `z = vp.z`.
  - Direzione del raggio: normalizzare `(x,y,z)`.
- Intersezione raggio–sfera: risolvere la quadratica; si sceglie il `t>0` più piccolo (più vicino).

## Formato immagine PPM (P6)
Header testuale:

P6
<width> <height>
255

Segue un blocco binario di `width*height*3` byte (RGB, 8 bit ciascuno) in ordine riga-per-riga, dall’alto verso il basso.

## Compilazione
### Windows (MSYS2/MinGW)
1) Installazione MSYS2 (una tantum):

winget install -e --id MSYS2.MSYS2 --accept-package-agreements --accept-source-agreements

2) Toolchain (una tantum):

"C:\\msys64\\usr\\bin\\bash.exe" -lc "pacman -Sy --noconfirm && pacman -S --needed --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-libwinpthread-git"

3) Build del progetto:

"C:\\msys64\\usr\\bin\\bash.exe" -lc "cd /c/Users/susid/OneDrive/Desktop/skuola/Raytracer && PATH=/mingw64/bin:$PATH mingw32-make -j"

Artefatto: `raytracer` (eseguibile nella root del progetto; su Windows il file sarà `raytracer.exe`).
Pulizia: "C:\\msys64\\usr\\bin\\bash.exe" -lc "cd /c/Users/susid/OneDrive/Desktop/skuola/Raytracer && mingw32-make clean"

### WSL/Ubuntu (alternativa)

sudo apt update && sudo apt install -y build-essential libomp-dev
make -j
make clean  # (opzionale) pulisce i .o e l'eseguibile

## Esecuzione ed esempi
Sintassi:

raytracer <scene.txt> <out.ppm> <width> <height>

Esempio (MSYS2, usa la scena inclusa):

"C:\\msys64\\usr\\bin\\bash.exe" -lc "cd /c/Users/susid/OneDrive/Desktop/skuola/Raytracer && PATH=/mingw64/bin:$PATH ./raytracer ./scenes/scene_pdf_example.txt ./ppms/example.ppm 1280 720"

Esecuzione rapida (PowerShell) con lo script incluso:

./RUN.ps1 -Width 640 -Height 360

Esecuzione diretta (PowerShell, senza script):

```powershell
mkdir .\ppms -Force
.\raytracer.exe .\scenes\scene_pdf_example.txt .\ppms\example.ppm 640 360
```

Nota: se dovessero mancare DLL, aggiungere `C:\msys64\mingw64\bin` al PATH oppure eseguire dalla shell MSYS2.

Conversione opzionale a PNG (ImageMagick):

"C:\\Program Files\\ImageMagick-7.1.1-Q16-HDRI\\magick.exe" .\\ppms\\example.ppm .\\renders\\example.png

## Script di supporto (trovati nella repo)
- `RUN.ps1` — Runner rapido: usa automaticamente `scripts/render.ps1` quando presente, crea le cartelle output.
- `scripts/render.ps1` — Render di una scena nelle cartelle corrette (`ppms/`, `renders/`).
- `scripts/render-all.ps1` — Render batch di tutte le scene in `scenes/`.
- `scripts/cleanup-scenes.ps1` — Riordina eventuali file scena finiti per errore nella root.
- `scripts/package.ps1` — Crea ZIP di consegna (minimo richiesto o con README).

Esempio rapido (PowerShell):

./scripts/render.ps1 -ScenePath ./scenes/scene_pdf_example.txt -Width 640 -Height 360

## Gestione errori e risorse
- Input non valido o file scena illeggibile → messaggio su stderr e uscita con codice > 0.
- Allocazioni fallite → messaggio su stderr; rilascio risorse; uscita sicura.
- La scrittura PPM mappata chiude sempre view/handle/file (Windows) o unmap/sync (Posix).

## Verifica rapida (smoke test) e qualità
- Build: `make` senza warning di regole duplicate; linking con OpenMP.
- Smoke test: render 320×180 produce un PPM valido in `ppms/`.
- Lint/Typecheck: non applicabile; codice C con warning minimi.
- Test manuale: apertura PPM con un visualizzatore o conversione con ImageMagick.

## Troubleshooting
- OpenMP non trovato (Windows): assicurarsi di usare `mingw32-make` dentro MSYS2 con `PATH=/mingw64/bin`.
- Eseguibile non parte fuori da MSYS2: l’eseguibile è MinGW; eseguirlo dalla shell MSYS2 o assicurarsi che le DLL richieste siano nel PATH.
- PPM non visualizzabile: usare un viewer che supporti P6 o convertire con ImageMagick.

.
