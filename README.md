Francesco Falcon Sm3201408

# Raytracer – Parte 1 (C)

Ray casting di sfere in C con OpenMP e output PPM (P6) scritto via memory mapping.

## Build

- Windows (MSYS2/MinGW):
  - pacman: mingw-w64-x86_64-gcc, mingw-w64-x86_64-make
  - PATH=/mingw64/bin:$PATH
  - mingw32-make

- Linux:
  - sudo apt install build-essential libomp-dev
  - make

## Uso

./raytracer <scene.txt> <out.ppm> <width> <height>

Formato scena: VP x y z; BG R G B; OBJ_N n; S cx cy cz r R G B (ripetuto n volte).
Francesco Falcon Sm3201408

# Raytracer – Parte 1 (C)

Implementazione del progetto di "Programmazione avanzata e parallela – Parte 1 (C)". Il programma effettua il rendering (ray casting) di sole sfere e salva l’immagine nel formato PPM (P6). La generazione dei pixel è parallelizzata con OpenMP. Il salvataggio è implementato con memoria mappata (mmap su POSIX, CreateFileMapping/MapViewOfFile su Windows).

## Requisiti del compito e copertura

- Linguaggio C, tipo floating point: `float` (f32). ✅
- Struttura a più file: `main.c`, `scene.h/.c`, `ppm.h/.c`. ✅
- Parsing file scena con `fscanf` nel formato indicato. ✅
- Ray casting: un raggio per pixel, intersezione raggio-sfera, colore dell’intersezione più vicina, background altrimenti. ✅
- Parallelizzazione con OpenMP (`#pragma omp parallel for`). ✅
- Scrittura immagine PPM P6 tramite memoria mappata. ✅
- Gestione errori con uscita ordinata e rilascio risorse. ✅
- Commenti esaustivi su funzioni e file (nome, cognome, matricola da completare). ✅
- Compilabile con `make` (Makefile fornito). ✅

## Struttura del progetto

- `main.c` – entrypoint: CLI, caricamento scena, rendering parallelo, salvataggio PPM.
- `scene.h/.c` – tipi e funzioni per viewport, sfere, scena; parsing; ray-sphere; calcolo colore per pixel.
- `ppm.h/.c` – salvataggio PPM (P6) tramite mapping.
- `Makefile` – build con GCC/Clang + OpenMP.
- `scenes/` – file di scena di esempio (facoltativi per la consegna).
- `ppms/` – PPM generati (output runtime).
- `renders/` – PNG derivati (output runtime, opzionale).
- `scripts/` – utility PowerShell per rendering/organizzazione (facoltativo).

## Formato del file di scena

Il file è testuale, con le seguenti sezioni in qualunque riga separate da spazi:

- `VP <x> <y> <z>`: dimensioni viewport (larghezza `x`, altezza `y`) e distanza `z` dalla camera.
- `BG <R> <G> <B>`: colore di background (u_int8 per canale, 0–255).
- `OBJ_N <n>`: numero di sfere che seguono.
- `S <cx> <cy> <cz> <r> <R> <G> <B>`: definizione di una sfera con centro `(cx,cy,cz)`, raggio `r` e colore RGB (u_int8).

Esempio (quello del PDF):

```
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
```

## Modello geometrico (riassunto)

- Camera in `(0,0,0)`.
- Viewport: rettangolo perpendicolare all’asse `z`, centrato, con dimensioni `vp.x` e `vp.y` posto a distanza `vp.z`.
- Per ogni pixel `(i,j)` di un’immagine `width×height`, si genera un vettore direzione:
  - `x = ((i + 0.5) - width/2) / width * vp.x`
  - `y = -(((j + 0.5) - height/2) / height) * vp.y` (segno meno perché l’asse Y dell’immagine cresce verso il basso)
  - `z = vp.z`
  - Si normalizza `(x,y,z)` e si lancia il raggio `V` dalla camera.
- Intersezione raggio-sfera: si risolve la quadratica e si prende `t>0` più piccolo.

## Formato PPM (P6)

Header testuale:

```
P6
<width> <height>
255
```

Segue un blocco binario di `width*height*3` byte con i pixel in ordine riga per riga (alto→basso, sinistra→destra), canali `R,G,B` (8 bit ciascuno).

## Compilazione

### Windows (MSYS2/MinGW)

1) Installa MSYS2 (una sola volta):
```powershell
winget install -e --id MSYS2.MSYS2 --accept-package-agreements --accept-source-agreements
```
2) Installa toolchain (una sola volta):
```powershell
C:\msys64\usr\bin\bash.exe -lc "pacman -Sy --noconfirm && pacman -S --needed --noconfirm mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-libwinpthread-git"
```
3) Compila:
```powershell
C:\msys64\usr\bin\bash.exe -lc "cd /c/Users/susid/OneDrive/Desktop/skuola/Raytracer && PATH=/mingw64/bin:$PATH mingw32-make"
```

### WSL/Ubuntu (alternativa)
```bash
sudo apt update && sudo apt install -y build-essential libomp-dev
make
```

## Esecuzione

CLI del programma:
```text
raytracer <scene.txt> <out.ppm> <width> <height>
```

Esempio (MSYS2):
```powershell
C:\msys64\usr\bin\bash.exe -lc "cd /c/Users/susid/OneDrive/Desktop/skuola/Raytracer && PATH=/mingw64/bin:/usr/bin:$PATH ./raytracer ./scene_pdf_example.txt ./ppms/example.ppm 1280 720"
```

Conversione rapida a PNG (opzionale, ImageMagick):
```powershell
& "C:\Program Files\ImageMagick-7.1.2-Q16-HDRI\magick.exe" .\ppms\example.ppm .\renders\example.png
```

## Script di supporto (facoltativi)

- `scripts/render.ps1` – rende una scena e salva direttamente in `ppms/` e `renders/`:
```powershell
.\n\scripts\render.ps1 -ScenePath .\scenes\scene_pdf_example.txt -Width 1280 -Height 720
```
- `scripts/render-all.ps1` – rende tutte le scene in `scenes/`:
```powershell
.
\scripts\render-all.ps1
```

## Gestione errori e risorse

- Input non valido, file scena non leggibile o non ben formato → messaggio su stderr e uscita con codice > 0.
- Allocazioni fallite → messaggio su stderr, rilascio risorse e uscita.
- In tutti i percorsi di errore vengono rilasciate le risorse (buffer immagine, array di sfere, handle/file mapping).

## Assunzioni/limitazioni (coerenti al PDF)

- Tutte le sfere sono davanti alla camera e non interamente coincidenti.
- Nessuna gestione di materiali/illuminazione/ombre: colore pieno dell’oggetto intersecato o background.
- Camera fissa in `(0,0,0)`; viewport fisso perpendicolare a `z`.

## Note per la consegna

- Compilare l’intestazione in ogni file (`Nome`, `Cognome`, `Matricola`).
- Consegnare i soli sorgenti e `Makefile` richiesti. Le cartelle `scenes/`, `ppms/`, `renders/`, `scripts/` sono utili per i test ma non necessarie se il docente richiede solo i minimi file.
- Per una consegna pulita:
```powershell
# Rimuovi artefatti di build
mingw32-make clean
Remove-Item .\ppms -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item .\renders -Recurse -Force -ErrorAction SilentlyContinue
```

## Contatti

Inserire in testa ai file: Nome, Cognome, Matricola. Per eventuali problemi di build su Windows, usare MSYS2/MinGW come indicato sopra.
