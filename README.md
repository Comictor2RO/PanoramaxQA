# Panoramax QA

Panoramax QA este o aplicatie C++23 care scaneaza imagini, citeste metadata EXIF/XMP si metadata C2PA si pregateste datele pentru verificari de calitate si generarea unui raport.

## Stare curenta

Implementat:

- parsarea argumentelor CLI cu CLI11;
- scanarea unui folder fara subdirectoare;
- filtrarea extensiilor `.jpg`, `.jpeg`, `.png`, `.tif`, `.tiff` si `.webp`;
- sortarea determinista a imaginilor;
- parsarea GPS din EXIF;
- conversia coordonatelor din grade, minute si secunde in grade zecimale;
- citirea altitudinii, GPSDOP, heading-ului, timestamp-ului si dimensiunilor;
- citirea optionala a unor valori pitch/roll din XMP;
- detectarea unor indicii AI din EXIF/XMP;
- citirea manifestelor C2PA;
- afisarea verdictului AI in modul verbose.
- calcularea brightness-ului si sharpness-ului pentru fiecare imagine;
- calcularea unui blur score normalizat;
- calcularea GPS score;
- generarea issue-urilor si a verdictului QA per imagine.

Inca neimplementat:

- calcularea pixel density;
- integrarea completa a rezultatelor QA in raportul final;
- analiza secventelor;
- raport JSON/CSV;
- teste unitare complete;
- upload-ul catre API-ul Panoramax.

## Dependente

### CLI11

CLI11 este folosit pentru parsarea argumentelor liniei de comanda:

- `--input` / `-i` pentru folderul de intrare;
- `--blur` / `-B` pentru pragul de blur;
- `--brightness` / `-b` pentru pragul de brightness;
- `--jump` / `-j` pentru saltul GPS maxim;
- `--report` / `-r` pentru calea raportului;
- `--upload` / `-u` pentru activarea upload-ului;
- `--token` / `-t` pentru token;
- `--api` / `-a` pentru URL-ul API;
- `--verbose`, `--quiet` si `--version`.

CLI11 valideaza folderul de intrare, valorile numerice si erorile de parsing.

### Exiv2

Exiv2 este folosit pentru metadata EXIF si XMP:

- coordonate GPS;
- referinte GPS `N`, `S`, `E`, `W`;
- altitudine;
- GPSDOP;
- heading;
- timestamp EXIF;
- latime si inaltime;
- unele valori XMP.

Erorile Exiv2 nu trebuie sa opreasca procesarea tuturor imaginilor. O imagine cu metadata lipsa sau invalida este pastrata in rezultate cu valori implicite si `is_valid = false`.

### C2PA

C2PA este folosit pentru citirea si validarea manifestelor de provenance din imagini. Proiectul foloseste biblioteca C++ `c2pa-cpp`, descarcata prin CMake `FetchContent`.

Detectorul C2PA:

1. incearca sa deschida imaginea cu `c2pa::Reader::from_asset()`;
2. ignora imaginile fara manifest C2PA;
3. citeste manifestul ca JSON;
4. cauta provideri si actiuni care indica generare AI;
5. adauga indicatorii gasiti in `ImageMetadata`;
6. combina rezultatul cu fallback-ul EXIF/XMP.

Manifestul C2PA este preferabil unei simple cautari de text, deoarece biblioteca poate verifica binding-ul si validarea manifestului.

### OpenCV

OpenCV este folosit pentru analiza imaginilor:

- incarcarea imaginilor;
- calcularea brightness-ului;
- calcularea sharpness-ului prin variance of Laplacian;
- calcularea blur score-ului normalizat;
- alte metrici QA viitoare.

### nlohmann/json

`nlohmann/json` este folosit pentru citirea JSON-ului returnat de C2PA si va fi folosit ulterior la generarea raportului JSON.

## QA per imagine

Functia `computeQA()` combina imaginea OpenCV cu `ImageMetadata` si produce un rezultat `ImageQA`.

### Brightness

Brightness-ul este media celor trei canale BGR si este comparat cu pragul `--brightness`.
Imaginile sub prag primesc issue-ul `too_dark`.

### Blur si sharpness

Imaginea este convertita in grayscale, apoi se calculeaza variance of Laplacian:

- variance mica indica o imagine blurata;
- variance mare indica o imagine mai clara.

Valoarea este normalizata intr-un `blur_score` intre `0` si `1` si comparata cu pragul `--blur`. Imaginile sub prag primesc issue-ul `too_blurry`.

### GPS score

`gps_score` este `1.0` cand `ImageMetadata::has_valid_gps` este adevarat si `0.0` in caz contrar. Lipsa unui GPS valid adauga issue-ul `invalid_gps`.

### Verdict AI in QA

Verdictul AI este preluat din `ImageMetadata`:

- `confirmed` adauga `ai_generated` si respinge imaginea;
- `likely` adauga `likely_ai_generated` si respinge imaginea;
- `not_detected` nu adauga un issue si accepta verificarea AI;
- `unknown` adauga `ai_status_unknown`, dar nu respinge automat imaginea.

Verdictul final `passed` este adevarat numai cand blur-ul, brightness-ul, GPS-ul si politica AI sunt acceptabile.

## Detectarea imaginilor generate cu AI

Detectarea actuala este bazata pe indicii de metadata si provenance. Nu este un detector vizual si nu poate demonstra ca o imagine este reala doar pentru ca nu are indicii AI.

### Indicii EXIF/XMP

Detectorul cauta valori in:

- `Exif.Image.Software`;
- `Xmp.xmp.CreatorTool`.

Printre markerii cautati se afla:

- Midjourney;
- Stable Diffusion;
- DALL-E;
- Adobe Firefly;
- Generative Fill;
- ComfyUI;
- Automatic1111;
- Leonardo AI;
- Ideogram;
- BytePlus ModelArk.

Un marker gasit in EXIF sau XMP produce verdictul `likely`.

### Indicii C2PA

Pentru manifestele C2PA sunt cautate valori precum:

- provider-ul software;
- `BytePlus_ModelArk`;
- actiuni de creare sau generare;
- texte care indica generare.

Daca exista un provider sau o actiune AI, indicatorul este pastrat in `ai_indicators`.

C2PA este mai puternic decat EXIF/XMP atunci cand manifestul este valid si semnatura poate fi verificata. In aplicatie, un manifest fara erori de validare poate produce verdictul `confirmed`, iar un manifest cu erori de validare produce `likely`.

## Verdicturi AI

Verdictul este stocat in:

```cpp
AiVerdict ai_verdict;
```

si poate avea una dintre valorile urmatoare.

### `confirmed`

Manifestul C2PA indica generare AI si nu raporteaza erori de validare.

Acesta este cel mai puternic verdict disponibil in implementarea curenta. El depinde de validarea facuta de biblioteca C2PA.

### `likely`

Au fost gasiti indicatori expliciti AI in EXIF, XMP sau intr-un manifest C2PA care are probleme de validare.

Exemple:

- `Software = Stable Diffusion`;
- `CreatorTool = Midjourney`;
- provider C2PA cunoscut;
- actiune C2PA de generare cu validare incompleta.

Acest verdict este probabil, dar metadata poate fi modificata sau falsificata.

### `not_detected`

Nu a fost gasit niciun indicator AI, iar imaginea contine cel putin doua campuri coerente de camera, de exemplu:

- `Make`;
- `Model`;
- `ExposureTime`;
- `FNumber`;
- `LensModel`.

Acest verdict inseamna doar ca generarea AI nu a fost detectata. Nu este o dovada absoluta ca imaginea este reala.

### `unknown`

Nu exista suficiente informatii pentru o concluzie:

- metadata lipsa;
- imagine exportata fara EXIF/XMP/C2PA;
- eroare la citirea manifestului;
- imagine fara campuri coerente de camera si fara indicatori AI.

O imagine AI ale carei metadata au fost sterse va ajunge, de regula, la `unknown`.

## Confidence si indicatori

Rezultatul este pastrat in `ImageMetadata`:

```cpp
AiVerdict ai_verdict = AiVerdict::unknown;
double ai_confidence = 0.0;
std::vector<std::string> ai_indicators;
```

Valorile curente sunt orientative:

- `1.0` pentru `confirmed`;
- `0.9` pentru `likely`;
- `0.6` pentru `not_detected`;
- `0.0` pentru `unknown`.

Aceste valori nu reprezinta probabilitati statistice calibrate.

## Build

```bash
cmake -S . -B build
cmake --build build --parallel
```

C2PA este descarcat prin `FetchContent`, deci prima configurare necesita acces la internet.

## Utilizare

Afisare help:

```bash
./build/panoramax_qa --help
```

Afisare versiune:

```bash
./build/panoramax_qa --version
```

Procesare folder:

```bash
./build/panoramax_qa \
    --input ./test_folder \
    --verbose
```

Exemplu de output:

```text
Images found: 1
./test_folder/image.jpg | valid: true | ai_verdict: likely | ai_confidence: 0.9
```

Pentru inspectarea metadata unei imagini:

```bash
exiv2 -pa imagine.jpg
```

## Limitari

- Lipsa metadata nu demonstreaza ca imaginea nu este generata cu AI.
- EXIF si XMP pot fi sterse sau modificate.
- Un provider poate folosi un nume care nu exista in lista curenta.
- Detectarea C2PA depinde de manifest, semnatura si suportul formatului.
- CBOR este procesat prin C2PA atunci cand face parte dintr-un manifest suportat; Exiv2 nu este folosit pentru citirea manifestelor C2PA.
- `pitch` si `roll` nu au tag-uri EXIF universale.
