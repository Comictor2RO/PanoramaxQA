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