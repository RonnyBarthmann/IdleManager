#include "sdl_wrapper.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ---------------------------------------------------------------- */
/* interner Zustand                                                  */
/* ---------------------------------------------------------------- */

typedef struct {
    SDL_Texture *tex;
    SDL_Surface *surface; /* NULL bei CreateImage()-Bildern (reine Render-Ziele);
                              bei LoadImage() die CPU-Kopie für schnelles Bulk-Lesen. */
    int w, h;
} Image;

static SDL_Window   *window   = NULL;
static SDL_Renderer *renderer = NULL;

static Image *images    = NULL;   /* images[0] entspricht Bild-ID 1 */
static int    imageCount = 0;
static int    imageCapacity = 0;

static int quitFlag    = 0;

/* Maustasten-Zustand, getrennt pro Taste. Index: 0=links, 1=rechts,
 * 2=mittig (Scrollrad). buttonIsDown ist der fortlaufende
 * "gehalten"-Zustand, die beiden Flags werden bei jedem
 * FlipBuffers() zurückgesetzt. */
static int buttonIsDown[3]       = { 0, 0, 0 };
static int buttonPressedFlag[3]  = { 0, 0, 0 };
static int buttonReleasedFlag[3] = { 0, 0, 0 };

static int firstKey    = 0;

/* ---------------------------------------------------------------- */
/* Hilfsfunktionen                                                   */
/* ---------------------------------------------------------------- */

static int StoreImage(SDL_Texture *tex, SDL_Surface *surface)
{
    if (!tex) return 0;

    if (imageCount >= imageCapacity) {
        int newCapacity = (imageCapacity == 0) ? 8 : imageCapacity * 2;
        Image *newArr = realloc(images, (size_t)newCapacity * sizeof(Image));
        if (!newArr) {
            SDL_DestroyTexture(tex);
            if (surface) SDL_FreeSurface(surface);
            return 0;
        }
        images = newArr;
        imageCapacity = newCapacity;
    }

    int w = 0, h = 0;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    images[imageCount].tex     = tex;
    images[imageCount].surface = surface;
    images[imageCount].w       = w;
    images[imageCount].h       = h;
    imageCount++;

    return imageCount; /* IDs beginnen bei 1 */
}

static SDL_Texture *GetTexture(int imageID)
{
    if (imageID <= 0 || imageID > imageCount) return NULL;
    return images[imageID - 1].tex;
}

/* liefert die aktuelle Größe der Zeichenfläche (in Pixeln); wird live
 * abgefragt, damit sie bei Größenänderungen immer aktuell ist. */
static void GetCurrentWindowSize(int *w, int *h)
{
    SDL_GetRendererOutputSize(renderer, w, h);
}

/* ---------------------------------------------------------------- */
/* Initialisierung / Beendigung                                      */
/* ---------------------------------------------------------------- */

int SDL_Wrapper_Init(int width, int height, const char *title)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 1;
    }

    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imgFlags) & imgFlags) == 0) {
        /* SDL_image konnte nicht vollständig initialisiert werden;
         * wir machen trotzdem weiter, LoadImage() kann dann für
         * manche Formate fehlschlagen. */
    }

    window = SDL_CreateWindow(title,
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               width, height,
                               SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1,
                                   SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_DestroyWindow(window);
        window = NULL;
        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_StartTextInput();

    quitFlag = 0;
    for (int i = 0; i < 3; i++) {
        buttonIsDown[i]       = 0;
        buttonPressedFlag[i]  = 0;
        buttonReleasedFlag[i] = 0;
    }
    firstKey = 0;

    return 0;
}

void SDL_Wrapper_Quit(void)
{
    for (int i = 0; i < imageCount; i++) {
        if (images[i].tex) SDL_DestroyTexture(images[i].tex);
        if (images[i].surface) SDL_FreeSurface(images[i].surface);
    }
    free(images);
    images = NULL;
    imageCount = 0;
    imageCapacity = 0;

    if (renderer) { SDL_DestroyRenderer(renderer); renderer = NULL; }
    if (window)   { SDL_DestroyWindow(window);     window   = NULL; }

    IMG_Quit();
    SDL_Quit();
}

/* ---------------------------------------------------------------- */
/* Bilder                                                             */
/* ---------------------------------------------------------------- */

int LoadImage(const char *file)
{
    /* Als Surface laden (statt direkt als Textur), damit wir die
     * Pixel danach auch ohne GPU-Readback im RAM zur Verfügung haben
     * (siehe GetImagePixels()/GetImageGreenChannel()). */
    SDL_Surface *loaded = IMG_Load(file);
    if (!loaded) return 0;

    /* Auf ein festes Format bringen, damit wir beim Auslesen genau
     * wissen, wie die Bytes im Speicher liegen (R,G,B,A pro Pixel). */
    SDL_Surface *surface = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(loaded);
    if (!surface) return 0;

    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surface);
    if (!tex) {
        SDL_FreeSurface(surface);
        return 0;
    }

    return StoreImage(tex, surface);
}

int CreateImage(int width, int height)
{
    if (width <= 0 || height <= 0) return 0;

    SDL_Texture *tex = SDL_CreateTexture(renderer,
                                          SDL_PIXELFORMAT_RGBA8888,
                                          SDL_TEXTUREACCESS_TARGET,
                                          width, height);
    if (!tex) return 0;

    /* neues Bild transparent leeren */
    SDL_Texture *prevTarget = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, tex);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, prevTarget);

    /* kein CPU-Puffer: CreateImage()-Bilder sind Render-Ziele und
     * ändern sich laufend - GetImagePixels() liefert dafür NULL. */
    return StoreImage(tex, NULL);
}

/* ---------------------------------------------------------------- */
/* Zeichnen                                                           */
/* ---------------------------------------------------------------- */

void DrawOutput(int imageID)
{
    if (imageID == 0) {
        SDL_SetRenderTarget(renderer, NULL);
        return;
    }
    SDL_Texture *tex = GetTexture(imageID);
    if (tex) {
        SDL_SetRenderTarget(renderer, tex);
    }
}

void DrawImage(int imageID, int x, int y, int width, int height)
{
    SDL_Texture *tex = GetTexture(imageID);
    if (!tex) return;

    SDL_Rect dst = { x, y, width, height };
    SDL_RenderCopy(renderer, tex, NULL, &dst);
}

void DrawImageRotated(int imageID, int x, int y, double angleDegree, double scale)
{
    if (imageID <= 0 || imageID > imageCount) return;
    SDL_Texture *tex = images[imageID - 1].tex;
    if (!tex) return;

    /* Winkel auf den Bereich [0, 360) normalisieren, auch für
     * Werte weit außerhalb von 0-360 (positiv wie negativ). */
    double angle = fmod(angleDegree, 360.0);
    if (angle < 0.0) angle += 360.0;

    int srcW = images[imageID - 1].w;
    int srcH = images[imageID - 1].h;

    int dstW = (int)(srcW * scale);
    int dstH = (int)(srcH * scale);

    /* (x,y) ist der Mittelpunkt -> obere linke Ecke entsprechend
     * verschieben. Das Zentrum für die Rotation (center = NULL)
     * entspricht dann automatisch der Mitte dieses Rechtecks, also
     * genau (x,y). */
    SDL_Rect dst = { x - dstW / 2, y - dstH / 2, dstW, dstH };

    SDL_RenderCopyEx(renderer, tex, NULL, &dst, angle, NULL, SDL_FLIP_NONE);
}

void DrawImageRotatedOffset(int imageID, int x, int y, int x2, int y2,
                             double angleDegree, double scale)
{
    double rad  = angleDegree * M_PI / 180.0;
    double cosA = cos(rad);
    double sinA = sin(rad);

    /* (x2,y2) um denselben Winkel wie das Bild rotieren */
    double rotatedX = (double)x2 * cosA - (double)y2 * sinA;
    double rotatedY = (double)x2 * sinA + (double)y2 * cosA;

    int newX = x + (int)lround(rotatedX);
    int newY = y + (int)lround(rotatedY);

    DrawImageRotated(imageID, newX, newY, angleDegree, scale);
}

/* ---------------------------------------------------------------- */
/* Pixelzugriff                                                       */
/* ---------------------------------------------------------------- */

int RGB(int r, int g, int b)
{
    return RGBA(r, g, b, 255);
}

int RGBA(int r, int g, int b, int alpha)
{
    return ((r & 0xFF) << 24) | ((g & 0xFF) << 16) | ((b & 0xFF) << 8) | (alpha & 0xFF);
}

int Red(int c)
{
    return (c >> 24) & 0xFF;
}

int Green(int c)
{
    return (c >> 16) & 0xFF;
}

int Blue(int c)
{
    return (c >> 8) & 0xFF;
}

/* interner Helfer, analog zu Red()/Green()/Blue(), aber nicht Teil
 * der öffentlichen API, da nicht angefordert. */
static int ExtractAlpha(int c)
{
    return c & 0xFF;
}

void Plot(int x, int y, int c)
{
    SDL_BlendMode prevMode;
    SDL_GetRenderDrawBlendMode(renderer, &prevMode);

    /* Farbe (inkl. Alpha) exakt schreiben statt zu verblenden, damit
     * Point() danach wieder denselben Wert liefert. */
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, (Uint8)Red(c), (Uint8)Green(c), (Uint8)Blue(c), (Uint8)ExtractAlpha(c));
    SDL_RenderDrawPoint(renderer, x, y);

    SDL_SetRenderDrawBlendMode(renderer, prevMode);
}

int Point(int x, int y)
{
    Uint8 pixel[4] = { 0, 0, 0, 0 };
    SDL_Rect rect = { x, y, 1, 1 };

    /* SDL_PIXELFORMAT_RGBA32 liegt unabhängig von der Endianness der
     * Plattform immer als R,G,B,A im Speicher vor. */
    if (SDL_RenderReadPixels(renderer, &rect, SDL_PIXELFORMAT_RGBA32, pixel, 4) != 0) {
        return 0;
    }

    return RGBA(pixel[0], pixel[1], pixel[2], pixel[3]);
}

int *GetImagePixels(int imageID, int *outWidth, int *outHeight)
{
    if (imageID <= 0 || imageID > imageCount) return NULL;

    SDL_Surface *surf = images[imageID - 1].surface;
    if (!surf) return NULL; /* z.B. CreateImage()-Bild ohne CPU-Puffer */

    int w = surf->w;
    int h = surf->h;

    int *result = malloc((size_t)w * (size_t)h * sizeof(int));
    if (!result) return NULL;

    SDL_LockSurface(surf);
    const Uint8 *pixels = (const Uint8 *)surf->pixels;
    int pitch = surf->pitch;

    for (int y = 0; y < h; y++) {
        const Uint8 *row = pixels + (size_t)y * pitch;
        for (int x = 0; x < w; x++) {
            const Uint8 *p = row + x * 4; /* RGBA32 -> 4 Byte/Pixel */
            result[(size_t)y * w + x] = RGBA(p[0], p[1], p[2], p[3]);
        }
    }
    SDL_UnlockSurface(surf);

    if (outWidth)  *outWidth  = w;
    if (outHeight) *outHeight = h;
    return result;
}

unsigned char *GetImageGreenChannel(int imageID, int *outWidth, int *outHeight)
{
    if (imageID <= 0 || imageID > imageCount) return NULL;

    SDL_Surface *surf = images[imageID - 1].surface;
    if (!surf) return NULL;

    int w = surf->w;
    int h = surf->h;

    unsigned char *result = malloc((size_t)w * (size_t)h);
    if (!result) return NULL;

    SDL_LockSurface(surf);
    const Uint8 *pixels = (const Uint8 *)surf->pixels;
    int pitch = surf->pitch;

    for (int y = 0; y < h; y++) {
        const Uint8 *row = pixels + (size_t)y * pitch;
        for (int x = 0; x < w; x++) {
            result[(size_t)y * w + x] = row[x * 4 + 1]; /* Grün-Kanal */
        }
    }
    SDL_UnlockSurface(surf);

    if (outWidth)  *outWidth  = w;
    if (outHeight) *outHeight = h;
    return result;
}

/* Wandelt SDL-Keycode + Modifier in den gewünschten ASCII-Wert um,
 * oder liefert 0, wenn die Taste ignoriert werden soll. */
static int TranslateKey(SDL_Keycode sym, Uint16 mod)
{
    if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) return 10;
    if (sym == SDLK_ESCAPE) return 27;

    /* druckbare Zeichen kommen normalerweise über SDL_TEXTINPUT rein,
     * hier fangen wir nur die o.g. Sonderfälle ab. */
    (void)mod;
    return 0;
}

/* Wandelt einen SDL-Maustasten-Code in unseren internen Array-Index
 * (0=links, 1=rechts, 2=mitte) um, oder -1 für nicht unterstützte
 * Tasten (z.B. die zusätzlichen "X1"/"X2"-Maustasten). */
static int ButtonToIndex(Uint8 sdlButton)
{
    switch (sdlButton) {
        case SDL_BUTTON_LEFT:   return 0;
        case SDL_BUTTON_RIGHT:  return 1;
        case SDL_BUTTON_MIDDLE: return 2;
        default:                return -1;
    }
}

void FlipBuffers(void)
{
    SDL_RenderPresent(renderer);

    /* Eingabezustand für den neuen Frame zurücksetzen (buttonIsDown
     * bleibt bestehen, da es den "gehalten"-Zustand fortlaufend
     * beschreibt und nicht pro Frame zurückgesetzt wird). */
    for (int i = 0; i < 3; i++) {
        buttonPressedFlag[i]  = 0;
        buttonReleasedFlag[i] = 0;
    }
    firstKey  = 0;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                quitFlag = 1;
                break;

            case SDL_MOUSEBUTTONDOWN: {
                int idx = ButtonToIndex(event.button.button);
                if (idx >= 0) {
                    if (!buttonIsDown[idx]) buttonPressedFlag[idx] = 1;
                    buttonIsDown[idx] = 1;
                }
                break;
            }

            case SDL_MOUSEBUTTONUP: {
                int idx = ButtonToIndex(event.button.button);
                if (idx >= 0) {
                    if (buttonIsDown[idx]) buttonReleasedFlag[idx] = 1;
                    buttonIsDown[idx] = 0;
                }
                break;
            }

            case SDL_KEYDOWN: {
                if (firstKey == 0) {
                    int k = TranslateKey(event.key.keysym.sym, event.key.keysym.mod);
                    if (k != 0) firstKey = k;
                }
                break;
            }

            case SDL_TEXTINPUT: {
                if (firstKey == 0) {
                    unsigned char c = (unsigned char)event.text.text[0];
                    if (c >= 32 && c < 127) {
                        firstKey = c;
                    }
                }
                break;
            }

            default:
                break;
        }
    }
}

/* ---------------------------------------------------------------- */
/* Eingabe                                                            */
/* ---------------------------------------------------------------- */

int GetMouseX(void)
{
    if (SDL_GetMouseFocus() != window) return -1;

    int x, y, w, h;
    SDL_GetMouseState(&x, &y);
    GetCurrentWindowSize(&w, &h);
    if (x < 0 || y < 0 || x >= w || y >= h) return -1;
    return x;
}

int GetMouseY(void)
{
    if (SDL_GetMouseFocus() != window) return -1;

    int x, y, w, h;
    SDL_GetMouseState(&x, &y);
    GetCurrentWindowSize(&w, &h);
    if (x < 0 || y < 0 || x >= w || y >= h) return -1;
    return y;
}

int GetMousePressed(int button)
{
    if (button == 0) {
        return buttonPressedFlag[0] || buttonPressedFlag[1] || buttonPressedFlag[2];
    }
    if (button >= 1 && button <= 3) {
        return buttonPressedFlag[button - 1];
    }
    return 0; /* ungültiger button-Wert */
}

int GetMouseReleased(int button)
{
    if (button == 0) {
        return buttonReleasedFlag[0] || buttonReleasedFlag[1] || buttonReleasedFlag[2];
    }
    if (button >= 1 && button <= 3) {
        return buttonReleasedFlag[button - 1];
    }
    return 0;
}

int GetMouseIsPressed(int button)
{
    if (button == 0) {
        return buttonIsDown[0] || buttonIsDown[1] || buttonIsDown[2];
    }
    if (button >= 1 && button <= 3) {
        return buttonIsDown[button - 1];
    }
    return 0;
}

int GetKey(void)
{
    return firstKey;
}

/* ---------------------------------------------------------------- */
/* Fenstergröße                                                       */
/* ---------------------------------------------------------------- */

int WindowWidth(void)
{
    int w, h;
    GetCurrentWindowSize(&w, &h);
    return w;
}

int WindowHeight(void)
{
    int w, h;
    GetCurrentWindowSize(&w, &h);
    return h;
}

int ShouldQuit(void)
{
    return quitFlag;
}
