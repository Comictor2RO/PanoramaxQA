# Panoramax QA — Milestones

## Obiectiv

Panoramax QA este o aplicație C++ care analizează imagini, extrage metadata EXIF/XMP, calculează metrici de calitate, detectează probleme la nivel de secvență și generează un raport JSON. Într-o etapă ulterioară, aplicația va putea încărca imaginile acceptate către API-ul Panoramax.

## Implementat

### Structura inițială

- Proiectul folosește CMake.
- Compilatorul C++ este configurat cu standardul C++23.
- Există directoare separate pentru cod sursă și headere.
- Executabilul proiectului se numește `panoramax_qa`.
- Build-ul funcționează în folderul separat `build/`.

Structura actuală relevantă:

```text
panoramax_qa/
├── CMakeLists.txt
├── includes/
│   ├── cli/
│   │   └── args.hpp
│   └── image/
│       ├── metadata.hpp
│       └── exif_parser.hpp
├── src/
│   ├── main.cpp
│   ├── cli/
│   │   └── args.cpp
│   └── image/
│       └── exif_parser.cpp
└── build/
```

### CMake

- CMake detectează OpenCV.
- CMake detectează nlohmann/json.
- CLI11 este inclus și link-uit prin target-ul `CLI11::CLI11`.
- Exiv2 este declarat ca dependență.
- `src/main.cpp`, `src/cli/args.cpp` și `src/image/exif_parser.cpp` sunt incluse în executabil.
- Directorul `includes/` este disponibil pentru includerea headerelor proiectului.
- Proiectul se configurează și se compilează cu succes.

Comenzi de build:

```bash
cmake -S . -B build
cmake --build build
```

Executabilul se rulează cu:

```bash
./build/panoramax_qa
```

### CLI parsing

Este definită structura `CliArgs` în `includes/cli/args.hpp`.

Câmpurile existente sunt:

- `input_file` — folderul cu imaginile.
- `threshold_blur` — pragul pentru blur.
- `threshold_brightness` — pragul pentru brightness.
- `max_gps_jump_meters` — saltul GPS maxim acceptat.
- `report_path` — calea raportului JSON.
- `do_upload` — activează sau dezactivează upload-ul.
- `token_path` — calea către token-ul Panoramax.
- `api_base_url` — URL-ul de bază al API-ului Panoramax.

`parseArgs()` folosește CLI11 și implementează:

- opțiunea obligatorie `--input` / `-i`;
- verificarea existenței folderului cu `CLI::ExistingDirectory`;
- opțiunea `--blur` / `-B` cu `CLI::PositiveNumber`;
- opțiunea `--brightness` / `-b` cu intervalul `0–255`;
- opțiunea `--jump` / `-j` cu `CLI::PositiveNumber`;
- opțiunea `--report` / `-r`;
- flag-ul `--upload` / `-u`;
- opțiunea `--token` / `-t`;
- opțiunea `--api` / `-a`;
- afișarea automată a help-ului și a erorilor de parsing.

Exemplu de rulare:

```bash
./build/panoramax_qa \
    --input ./images \
    --blur 0.4 \
    --brightness 40 \
    --jump 100 \
    --report result.json
```

### Modelul pentru metadata

Este definită structura `ImageMetadata` în `includes/image/metadata.hpp`.

Structura conține câmpuri pentru:

- calea imaginii;
- latitudine;
- longitudine;
- altitudine;
- precizie GPS;
- heading;
- pitch;
- roll;
- timestamp;
- lățime și înălțime;
- starea `is_valid`.

### Începutul EXIF parsing-ului

Este declarată funcția:

```cpp
ImageMetadata parseExif(const std::string& image_path);
```

Funcția este destinată să:

- deschidă o imagine cu Exiv2;
- citească metadata prin `readMetadata()`;
- acceseze `ExifData`;
- caute tag-urile GPS;
- marcheze metadata ca invalidă dacă informațiile esențiale lipsesc;
- gestioneze erorile Exiv2 fără să oprească întreaga aplicație.

## Neimplementat încă

- Scanarea tuturor imaginilor din folder.
- Filtrarea fișierelor după extensii (`.jpg`, `.jpeg`, `.png` etc.).
- Conversia completă a coordonatelor GPS în grade zecimale.
- Citirea referințelor GPS `N/S/E/W`.
- Citirea altitudinii.
- Citirea GPSDOP sau a erorii de poziționare.
- Citirea heading-ului, pitch-ului și roll-ului.
- Parsarea timestamp-ului EXIF.
- Citirea rezoluției imaginii din OpenCV sau EXIF.
- Structura `ImageQA`.
- Calcularea blurului.
- Calcularea brightness-ului.
- Calcularea pixel density.
- Calcularea scorului GPS.
- Detectarea problemelor individuale ale imaginilor.
- Analiza secvențelor.
- Detectarea salturilor GPS.
- Verificarea coerenței heading-ului.
- Detectarea imaginilor duplicate.
- Generarea raportului JSON.
- Generarea raportului CSV.
- Clientul HTTP Panoramax.
- Autentificarea prin token.
- Upload-ul imaginilor.
- Progress bar-ul și statisticile din terminal.
- Testele unitare.
- Documentația finală și exemplele complete de utilizare.

## Pașii următori

### Milestone 1 — Finalizarea CLI-ului

1. Verifică faptul că `args.cpp` este inclus în `add_executable()`.
2. Compilează proiectul.
3. Rulează `--help`.
4. Testează lipsa argumentului `--input`.
5. Testează un folder inexistent.
6. Testează valori invalide pentru blur, brightness și GPS jump.
7. Testează afișarea valorilor parse-uite în `main.cpp`.

Comenzi:

```bash
cmake -S . -B build
cmake --build build
./build/panoramax_qa --help
```

### Milestone 2 — EXIF parsing pentru o singură imagine

1. Implementează deschiderea imaginii cu `Exiv2::ImageFactory::open()`.
2. Apelează `readMetadata()`.
3. Citește `ExifData`.
4. Verifică existența tag-urilor GPS.
5. Implementează conversia grade/minute/secunde în grade zecimale.
6. Aplică semnul corect pentru `N`, `S`, `E` și `W`.
7. Citește altitudinea și GPSDOP.
8. Citește heading-ul folosind fallback între `GPSImgDirection` și `GPSTrack`.
9. Citește timestamp-ul și dimensiunile.
10. Testează imagini cu metadata completă și imagini fără GPS.

### Milestone 3 — Scanarea folderului

1. Folosește `std::filesystem` pentru parcurgerea folderului.
2. Acceptă doar extensii de imagine suportate.
3. Sortează imaginile într-o ordine deterministă.
4. Apelează `parseExif()` pentru fiecare imagine.
5. Păstrează rezultatele într-un `std::vector<ImageMetadata>`.
6. Raportează numărul de imagini procesate și erorile.

Exemplu de structură:

```cpp
std::vector<ImageMetadata> images;
```

### Milestone 4 — QA per imagine

1. Creează `includes/qa/image_qa.hpp`.
2. Definește structura `ImageQA`.
3. Implementează încărcarea imaginii cu OpenCV.
4. Calculează brightness-ul mediu.
5. Calculează variance of Laplacian pentru sharpness.
6. Convertește sharpness-ul într-un scor de blur clar definit.
7. Calculează scorul GPS.
8. Adaugă issue-uri pentru problemele detectate.
9. Leagă pragurile din `CliArgs` de verdictul QA.

### Milestone 5 — QA la nivel de secvență

1. Sortează imaginile după timestamp.
2. Compară imaginile consecutive.
3. Implementează distanța Haversine.
4. Detectează salturile GPS.
5. Calculează diferența de heading.
6. Detectează heading-uri incoerente.
7. Detectează posibile duplicate.
8. Stochează rezultatele într-o structură `SequenceQA`.

### Milestone 6 — Raport JSON

1. Creează `includes/report/generator.hpp`.
2. Creează `src/report/generator.cpp`.
3. Definește structura raportului.
4. Adaugă sumarul global.
5. Adaugă lista imaginilor și issue-urile lor.
6. Adaugă rezultatele secvențelor.
7. Scrie raportul folosind nlohmann/json.
8. Gestionează erorile de scriere în fișier.
9. Testează output-ul cu un parser JSON extern.

### Milestone 7 — Integrarea API-ului Panoramax

1. Instalează și configurează cpr.
2. Creează `PanoramaxClient`.
3. Implementează citirea token-ului.
4. Implementează construirea endpoint-urilor.
5. Implementează crearea unui upload set.
6. Implementează upload-ul unui fișier.
7. Implementează finalizarea upload set-ului.
8. Implementează verificarea statusului.
9. Încarcă doar imaginile care au trecut QA.
10. Gestionează timeout-uri, statusuri HTTP și retry-uri.

### Milestone 8 — Integrarea finală

1. Leagă toate modulele în `main.cpp`.
2. Procesează folderul complet.
3. Afișează progresul.
4. Afișează statistici finale.
5. Generează raportul indiferent dacă unele imagini eșuează.
6. Activează upload-ul doar dacă este prezent `--upload`.
7. Adaugă mesaje de eroare clare.
8. Testează pe seturi mici și mari de imagini.
9. Adaugă teste unitare pentru GPS, blur și Haversine.
10. Actualizează README-ul.

## Criteriul de MVP

Proiectul poate fi considerat MVP atunci când poate:

1. Primi un folder prin CLI.
2. Parcurge imaginile din folder.
3. Extrage GPS și timestamp din EXIF.
4. Calculează blur și brightness.
5. Marchează imaginile cu probleme.
6. Generează `report.json`.

Upload-ul Panoramax este o funcționalitate suplimentară și nu trebuie să blocheze demonstrația MVP-ului.

## Comenzi uzuale

Reconfigurare completă:

```bash
rm -rf build
cmake -S . -B build
```

Build incremental:

```bash
cmake --build build
```

Build cu mai multe thread-uri:

```bash
cmake --build build --parallel
```

Rulare help:

```bash
./build/panoramax_qa --help
```

Rulare minimală:

```bash
./build/panoramax_qa --input ./images
```
