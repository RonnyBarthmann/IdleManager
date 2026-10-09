#ifndef SDL_WRAPPER_H
#define SDL_WRAPPER_H

/* ---- Initialisierung / Beendigung (wird in der Grunddatei aufgerufen) ---- */

/* Öffnet das Fenster und initialisiert SDL.
 * Rückgabe: 0 bei Erfolg, ungleich 0 bei Fehler. */
int SDL_Wrapper_Init(int width, int height, const char *title);

/* Räumt SDL sauber wieder auf (Fenster schließen, Speicher freigeben, ...). */
void SDL_Wrapper_Quit(void);

/* ---- Bilder ---- */

/* Lädt ein Bild von der Festplatte (png, jpg, ...).
 * Rückgabe: Bild-ID (>0) bei Erfolg, 0 bei Fehler. */
int LoadImage(const char *file);

/* Erstellt ein leeres, transparentes Bild in der angegebenen Größe.
 * Rückgabe: Bild-ID (>0) bei Erfolg, 0 bei Fehler. */
int CreateImage(int width, int height);

/* ---- Zeichnen ---- */

/* Legt fest, auf welches Bild ab sofort gezeichnet wird.
 * imageID == 0 bedeutet: Zeichnen direkt auf das Fenster. */
void DrawOutput(int imageID);

/* Zeichnet das Bild mit der ID "imageID" bei (x,y) gestreckt auf
 * die Größe (width,height) auf das aktuell mit DrawOutput()
 * gesetzte Ziel. */
void DrawImage(int imageID, int x, int y, int width, int height);

/* Überträgt das gezeichnete Bild auf den Bildschirm (wartet dabei
 * ggf. auf vsync) und wertet anschließend die Eingabe-Ereignisse
 * (Maus/Tastatur) für den nächsten Frame neu aus. */
void FlipBuffers(void);

/* Wie DrawImage(), zeichnet das Bild aber um seinen Mittelpunkt
 * rotiert und skaliert:
 *  - (x,y) ist der Mittelpunkt des gezeichneten Bildes (nicht die
 *    obere linke Ecke).
 *  - angleDegree dreht im Uhrzeigersinn (0 = normal, 90 = nach
 *    rechts gedreht, 270 = nach links gedreht). Werte außerhalb von
 *    0-360 sind erlaubt und werden automatisch normalisiert.
 *  - scale skaliert die Originalgröße des Bildes (1.0 = Originalgröße,
 *    2.0 = doppelt so groß, 0.5 = halb so groß). */
void DrawImageRotated(int imageID, int x, int y, double angleDegree, double scale);

/* Wie DrawImageRotated(), addiert aber vorher einen Offset (x2,y2)
 * zu (x,y), der seinerseits um angleDegree "mitrotiert" wird.
 * Beispiel: bei angleDegree=0 landet (x2,y2)=(0,-100) einfach bei
 * (x, y-100). Bei angleDegree=90 landet derselbe Offset bei
 * (x+100, y). Nützlich z.B. um einen "Geschützturm" versetzt vom
 * Drehpunkt eines Objekts zu zeichnen. */
void DrawImageRotatedOffset(int imageID, int x, int y, int x2, int y2,
                             double angleDegree, double scale);

/* ---- Pixelzugriff ---- */

/* Farbwerte werden als int im Format 0xRRGGBBAA kodiert (siehe RGB()
 * und RGBA()) - das ist das Format, das Plot() erwartet und Point()
 * zurückgibt. */

/* Setzt den Pixel bei (x,y) auf der aktuellen Ausgabe (siehe
 * DrawOutput()) auf die Farbe c (inkl. Alpha-Kanal, der Pixel wird
 * also exakt überschrieben, nicht "verblendet"). */
void Plot(int x, int y, int c);

/* Liefert den Farbwert des Pixels bei (x,y) auf der aktuellen
 * Ausgabe zurück (0xRRGGBBAA), oder 0 falls (x,y) außerhalb liegt.
 * Hinweis: relativ langsam (GPU-Readback pro Aufruf) - für sehr
 * viele Pixel auf einmal GetImagePixels()/GetImageGreenChannel()
 * benutzen. */
int Point(int x, int y);

/* Baut aus r,g,b (0-255) einen Farbwert wie von Plot() erwartet /
 * von Point() geliefert. Alpha wird dabei auf 255 (undurchsichtig)
 * gesetzt. */
int RGB(int r, int g, int b);

/* Wie RGB(), mit zusätzlichem Alpha-Kanal (0-255). */
int RGBA(int r, int g, int b, int alpha);

/* Extrahiert den Rot-/Grün-/Blau-Anteil (0-255) aus einer Farbe, wie
 * sie z.B. von Point() oder RGB()/RGBA() geliefert wird. */
int Red(int c);
int Green(int c);
int Blue(int c);

/* ---- Schnelles Bulk-Auslesen ----
 *
 * Point() ist für einzelne Pixel gedacht - jeder Aufruf synchronisiert
 * mit der GPU und ist bei sehr vielen Pixeln (z.B. beim Einlesen einer
 * Level-Grafik) entsprechend langsam. Für solche Fälle: einmalig das
 * komplette Bild in ein Array im RAM kopieren und danach direkt
 * indizieren (result[y*width+x]). Funktioniert nur für Bilder, die
 * mit LoadImage() geladen wurden (nicht für mit CreateImage()
 * erzeugte Render-Ziele) - liefert sonst NULL. Das zurückgegebene
 * Array gehört dem Aufrufer und muss mit free() freigegeben werden.
 */

/* Kopiert das komplette Bild als Farbwerte (0xRRGGBBAA, siehe RGB())
 * in ein neu allokiertes Array (Zeile für Zeile, result[y*w+x]).
 * outWidth/outHeight (falls != NULL) werden mit der Bildgröße
 * gefüllt. NULL bei Fehler. */
int *GetImagePixels(int imageID, int *outWidth, int *outHeight);

/* Wie GetImagePixels(), liefert aber nur den Grün-Kanal (0-255) pro
 * Pixel als char-Array - braucht nur ein Viertel des Speichers und
 * ist noch etwas schneller, falls du wirklich nur den Grün-Kanal
 * brauchst (z.B. für eine Höhenkarte). */
unsigned char *GetImageGreenChannel(int imageID, int *outWidth, int *outHeight);

/* ---- Eingabe ---- */

/* X-Position der Maus relativ zum Fenster, oder -1 falls außerhalb. */
int GetMouseX(void);

/* Y-Position der Maus relativ zum Fenster, oder -1 falls außerhalb. */
int GetMouseY(void);

/* button wählt aus, welche Maustaste betrachtet wird:
 *   0 = egal (irgendeine Taste)
 *   1 = nur linke Maustaste
 *   2 = nur rechte Maustaste
 *   3 = nur mittlere Maustaste (Scrollrad) */

/* Ungleich 0, falls die betrachtete(n) Taste(n) zwischen den
 * letzten beiden Aufrufen von FlipBuffers() gedrückt wurde(n)
 * (Übergang von "nicht gedrückt" zu "gedrückt", steigende Flanke). */
int GetMousePressed(int button);

/* Ungleich 0, falls die betrachtete(n) Taste(n) zwischen den
 * letzten beiden Aufrufen von FlipBuffers() losgelassen wurde(n)
 * (Übergang von "gedrückt" zu "nicht gedrückt", fallende Flanke). */
int GetMouseReleased(int button);

/* Ungleich 0, falls die betrachtete(n) Taste(n) beim letzten
 * FlipBuffers() gedrückt war(en)/ist(sind) - unabhängig davon, ob
 * sie schon vorher gedrückt war(en) (also ob gerade "gehalten"
 * wird). */
int GetMouseIsPressed(int button);

/* ASCII-Wert der ersten "Zeichen-Taste" (inkl. Großschreibung durch
 * Shift), die seit dem letzten FlipBuffers() gedrückt wurde.
 * Enter = 10, ESC = 27. Alle anderen Sondertasten werden ignoriert.
 * 0, falls keine solche Taste gedrückt wurde. */
int GetKey(void);

/* ---- Fenstergröße ---- */

int WindowWidth(void);
int WindowHeight(void);

/* Ungleich 0, sobald der Benutzer das Fenster schließen möchte
 * (z.B. über den Schließen-Button). Praktisch für die
 * Abbruchbedingung der Hauptschleife. */
int ShouldQuit(void);

#endif /* SDL_WRAPPER_H */
