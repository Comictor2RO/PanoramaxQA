# Probleme intalnite

## 1. Namespace XMP necunoscut pentru pitch si roll

### Simptom

Parserul se oprea inainte sa citeasca metadata EXIF pentru imagini care nu aveau campurile `pitch` si `roll`. Aplicatia afisa eroarea:

```text
No namespace info available for XMP prefix `Camera'
```

### Cauza

Parserul incerca sa caute tag-urile XMP:

```text
Xmp.Camera.Pitch
Xmp.Camera.Roll
```

Namespace-ul `Camera` nu era inregistrat in Exiv2. Crearea cheii XMP arunca o exceptie, iar executia parasea functia inainte de parsarea GPS din EXIF.

### Rezolvare

Parsarea XMP a fost tratata ca optionala si izolata intr-un bloc `try/catch`. Daca namespace-ul sau tag-urile XMP lipsesc, parsarea EXIF continua normal.

Lipsa valorilor `pitch` si `roll` nu trebuie sa invalideze imaginea. Aceste campuri raman la valorile implicite atunci cand nu sunt disponibile.

## 2. Tool-urile traditionale nu expun metadata C2PA/CBOR

### Simptom

`exiv2 -pa` afisa metadata EXIF si XMP, dar nu permitea citirea directa a unor valori precum:

```text
[CBOR] ActionsSoftwareAgentName: BytePlus_ModelArk
```

Detectorul bazat doar pe `Exif.Image.Software` si `Xmp.xmp.CreatorTool` nu putea identifica acest marker.

### Cauza

C2PA foloseste manifesturi si date JUMBF/CBOR pentru provenance. Aceste date nu sunt echivalente cu tag-urile EXIF sau XMP, iar versiunea locala Exiv2 nu oferea un API C++ pentru citirea lor.

### Rezolvare

Proiectul foloseste biblioteca `c2pa-cpp` pentru a deschide manifestul cu `c2pa::Reader::from_asset()`, a citi manifestul ca JSON si a cauta indicatorii C2PA separat de parserul EXIF.

`detectAiMetadata()` ramane un fallback pentru EXIF/XMP. C2PA este tratat intr-un detector separat, deoarece necesita citirea si validarea manifestului, nu doar cautarea unor tag-uri metadata.