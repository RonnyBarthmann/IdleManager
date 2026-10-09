/*
 * mapsplit - splits a large PNG into tiles of given size
 * Usage: mapsplit <input.png> <output_prefix> <tilesize>
 * Example: mapsplit bigmap.png map 1024
 *          → map_00_00.png, map_00_01.png, ...
 *
 * Memory-efficient: only a few rows are held in RAM at any time.
 * Preserves alpha channel if present.
 *
 * Compile: gcc -O2 -o mapsplit mapsplit.c -lpng -lz -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <stdint.h>

#define MAX_SIDE 65536

typedef struct {
    png_structp png_ptr;
    png_infop   info_ptr;
    FILE       *fp;
    int         width, height;
    int         channels;          /* 3 = RGB, 4 = RGBA */
    int         bit_depth;
    int         color_type;
    png_bytep   row;               /* one decoded row */
} png_reader_t;

static void die(const char *msg) {
    fprintf(stderr, "Error: %s\n", msg);
    exit(1);
}

static void reader_init(png_reader_t *r, const char *filename) {
    r->fp = fopen(filename, "rb");
    if (!r->fp) die("cannot open input file");

    unsigned char sig[8];
    if (fread(sig, 1, 8, r->fp) != 8 || png_sig_cmp(sig, 0, 8))
        die("not a valid PNG file");

    r->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!r->png_ptr) die("png_create_read_struct failed");

    r->info_ptr = png_create_info_struct(r->png_ptr);
    if (!r->info_ptr) die("png_create_info_struct failed");

    if (setjmp(png_jmpbuf(r->png_ptr)))
        die("libpng error while reading");

    png_init_io(r->png_ptr, r->fp);
    png_set_sig_bytes(r->png_ptr, 8);
    png_read_info(r->png_ptr, r->info_ptr);

    r->width      = png_get_image_width(r->png_ptr, r->info_ptr);
    r->height     = png_get_image_height(r->png_ptr, r->info_ptr);
    r->bit_depth  = png_get_bit_depth(r->png_ptr, r->info_ptr);
    r->color_type = png_get_color_type(r->png_ptr, r->info_ptr);

    if (r->width > MAX_SIDE || r->height > MAX_SIDE)
        die("image side length exceeds 65536");

    /* Normalize to 8-bit RGB or RGBA */
    if (r->bit_depth == 16)
        png_set_strip_16(r->png_ptr);

    if (r->color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(r->png_ptr);

    if (r->color_type == PNG_COLOR_TYPE_GRAY && r->bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(r->png_ptr);

    if (png_get_valid(r->png_ptr, r->info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(r->png_ptr);

    if (r->color_type == PNG_COLOR_TYPE_GRAY ||
        r->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(r->png_ptr);

    /* After transforms: either RGB or RGBA */
    png_read_update_info(r->png_ptr, r->info_ptr);
    r->channels = png_get_channels(r->png_ptr, r->info_ptr); /* 3 or 4 */

    r->row = (png_bytep)malloc(png_get_rowbytes(r->png_ptr, r->info_ptr));
    if (!r->row) die("out of memory for row buffer");
}

static void reader_read_row(png_reader_t *r) {
    if (setjmp(png_jmpbuf(r->png_ptr)))
        die("libpng error while reading row");
    png_read_row(r->png_ptr, r->row, NULL);
}

static void reader_finish(png_reader_t *r) {
    if (setjmp(png_jmpbuf(r->png_ptr)))
        die("libpng error while finishing");
    png_read_end(r->png_ptr, NULL);
    png_destroy_read_struct(&r->png_ptr, &r->info_ptr, NULL);
    free(r->row);
    fclose(r->fp);
}

/* ------------------------------------------------------------------ */

typedef struct {
    png_structp png_ptr;
    png_infop   info_ptr;
    FILE       *fp;
    int         channels;
} png_writer_t;

static void writer_init(png_writer_t *w, const char *filename,
                        int width, int height, int channels) {
    w->fp = fopen(filename, "wb");
    if (!w->fp) {
        fprintf(stderr, "cannot create %s\n", filename);
        exit(1);
    }

    w->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!w->png_ptr) die("png_create_write_struct failed");

    w->info_ptr = png_create_info_struct(w->png_ptr);
    if (!w->info_ptr) die("png_create_info_struct failed");

    if (setjmp(png_jmpbuf(w->png_ptr)))
        die("libpng error while writing");

    png_init_io(w->png_ptr, w->fp);

    int color_type = (channels == 4) ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
    png_set_IHDR(w->png_ptr, w->info_ptr, width, height,
                 8, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(w->png_ptr, w->info_ptr);
    w->channels = channels;
}

static void writer_write_row(png_writer_t *w, png_bytep row) {
    if (setjmp(png_jmpbuf(w->png_ptr)))
        die("libpng error while writing row");
    png_write_row(w->png_ptr, row);
}

static void writer_finish(png_writer_t *w) {
    if (setjmp(png_jmpbuf(w->png_ptr)))
        die("libpng error while finishing write");
    png_write_end(w->png_ptr, NULL);
    png_destroy_write_struct(&w->png_ptr, &w->info_ptr);
    fclose(w->fp);
}

/* ------------------------------------------------------------------ */

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input.png> <output_prefix> <tilesize>\n", argv[0]);
        fprintf(stderr, "Example: %s bigmap.png map 1024\n", argv[0]);
        return 1;
    }

    const char *input   = argv[1];
    const char *prefix  = argv[2];
    int tilesize        = atoi(argv[3]);

    if (tilesize <= 0 || tilesize > MAX_SIDE)
        die("tilesize must be between 1 and 65536");

    png_reader_t reader;
    reader_init(&reader, input);

    int w = reader.width;
    int h = reader.height;
    int channels = reader.channels;

    printf("Input: %dx%d, %d channels → tiles of %dx%d\n",
           w, h, channels, tilesize, tilesize);

    int tiles_x = (w + tilesize - 1) / tilesize;
    int tiles_y = (h + tilesize - 1) / tilesize;

    /* We keep one full horizontal strip of tiles in memory (tilesize rows) */
    size_t row_bytes = (size_t)w * channels;
    png_bytep *strip = malloc(tilesize * sizeof(png_bytep));
    if (!strip) die("out of memory");

    for (int i = 0; i < tilesize; i++) {
        strip[i] = malloc(row_bytes);
        if (!strip[i]) die("out of memory for strip");
    }

    char filename[1024];
    png_writer_t *writers = malloc(tiles_x * sizeof(png_writer_t));
    if (!writers) die("out of memory");

    int y = 0;
    while (y < h) {
        int rows_in_strip = (y + tilesize <= h) ? tilesize : (h - y);

        /* Read the next strip of rows */
        for (int r = 0; r < rows_in_strip; r++) {
            reader_read_row(&reader);
            memcpy(strip[r], reader.row, row_bytes);
        }

        /* Open writers for this horizontal band of tiles */
        for (int tx = 0; tx < tiles_x; tx++) {
            int tile_w = (tx * tilesize + tilesize <= w)
                         ? tilesize : (w - tx * tilesize);
            int tile_h = rows_in_strip;

            snprintf(filename, sizeof(filename), "%s_%02d_%02d.png",
                     prefix, tx, y / tilesize);

            writer_init(&writers[tx], filename, tile_w, tile_h, channels);
        }

        /* Write the strip into the corresponding tiles */
        for (int r = 0; r < rows_in_strip; r++) {
            for (int tx = 0; tx < tiles_x; tx++) {
                int tile_w = (tx * tilesize + tilesize <= w)
                             ? tilesize : (w - tx * tilesize);
                png_bytep src = strip[r] + (size_t)tx * tilesize * channels;
                writer_write_row(&writers[tx], src);
            }
        }

        /* Close this band of tiles */
        for (int tx = 0; tx < tiles_x; tx++)
            writer_finish(&writers[tx]);

        y += rows_in_strip;
        printf("\rProcessed %d / %d rows", y, h);
        fflush(stdout);
    }
    printf("\nDone. Created %d × %d = %d tiles.\n", tiles_y, tiles_x, tiles_x * tiles_y);

    /* Cleanup */
    for (int i = 0; i < tilesize; i++)
        free(strip[i]);
    free(strip);
    free(writers);
    reader_finish(&reader);

    return 0;
}
