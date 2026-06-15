//
// Created by MeowWow520 on 2026/6/15.
// dotbin.c — 生成 AT27C64 EPROM 二进制图案文件
//
// 用法:
//   dotbin.exe                 生成 pattern.bin 并打印 hex
//   dotbin.exe -o output.bin   指定输出文件名
//   dotbin.exe -s              终端预览所有图案
//   dotbin.exe -p 0            预览第 0 幅图案
//
// 输出: IMAGES 数组中的 uint64_t 数据以 LE 字节序展开，
//       每 8 字节为一幅 8×8 图案（共 32 幅，256 字节）。
//       AT27C64 地址: A0-A2 选行, A3-A7 选图案 (0-31).

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPLIT_U64_TO_BYTES_LE(val, out_arr) \
    do { \
        uint64_t _tmp = (val); \
        (out_arr)[0] = (uint8_t)(_tmp); \
        (out_arr)[1] = (uint8_t)(_tmp >> 8); \
        (out_arr)[2] = (uint8_t)(_tmp >> 16); \
        (out_arr)[3] = (uint8_t)(_tmp >> 24); \
        (out_arr)[4] = (uint8_t)(_tmp >> 32); \
        (out_arr)[5] = (uint8_t)(_tmp >> 40); \
        (out_arr)[6] = (uint8_t)(_tmp >> 48); \
        (out_arr)[7] = (uint8_t)(_tmp >> 56); \
    } while(0)

#define BYTES_PER_IMAGE  8
#define MAX_IMAGES       32
#define TOTAL_BYTES      (MAX_IMAGES * BYTES_PER_IMAGE)

const uint64_t IMAGES[] = {
    0x0e00040000ffffff,
    0x0704000000ffffff,
    0x0e00080000ffffff,
    0x0e00080000ffffff,
    0x1c00001000ffffff,
    0x3800000020ffffff,
    0x7000004000dfffff,
    0x7000004000dfffff,
    0x7000800000dfffff,
    0x7040000000dfffff,
    0x7000200000dfffff,
    0x7000200000dfffff,
    0x7000001000dfffff,
    0x3800000008dfffff,
    0x3800000400d7ffff,
    0x3800000400d7ffff,
    0x1c00020000d7ffff,
    0x0e01000000d7ffff,
    0x0701000000d7ffff,
    0x0700010000d7ffff,
    0x0700010000d7ffff,
    0x0700000100d7ffff,
    0x0700000001d7ffff,
    0x0700000001d6ffff,
    0x0700000100d6ffff,
    0x0700010000d6ffff,
    0x0701000000d6ffff,
    0x0e01000000d6ffff,
    0x0e00020000d6ffff,
    0x0e00000400d6ffff,
    0x0e00000008d6ffff,
    0x1c00000010c6ffff
};

const int IMAGES_LEN = sizeof(IMAGES) / sizeof(IMAGES[0]);

static void split_all(uint8_t *buf)
{
    memset(buf, 0, TOTAL_BYTES);
    for (int i = 0; i < IMAGES_LEN; i++) {
        SPLIT_U64_TO_BYTES_LE(IMAGES[i], &buf[i * BYTES_PER_IMAGE]);
    }
}

static void write_bin(const char *path, uint8_t *buf)
{
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "error: open '%s' failed\n", path);
        exit(1);
    }
    size_t written = fwrite(buf, 1, TOTAL_BYTES, fp);
    fclose(fp);
    printf("written: %s  (%zu bytes, %d images)\n", path, written, IMAGES_LEN);
}

static void print_hex(uint8_t *buf)
{
    for (int i = 0; i < TOTAL_BYTES; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % BYTES_PER_IMAGE == 0) printf("\n");
    }
}

static void draw_image(uint8_t *buf, int idx)
{
    if (idx < 0 || idx >= IMAGES_LEN) {
        printf("image index %d out of range (0-%d)\n", idx, IMAGES_LEN - 1);
        return;
    }
    uint8_t *rows = &buf[idx * BYTES_PER_IMAGE];
    printf("\n  image %d\n", idx);
    printf("  -----------------\n");
    for (int r = 0; r < 8; r++) {
        printf("  |");
        for (int c = 7; c >= 0; c--) {
            printf("%s", (rows[r] >> c) & 1 ? "# " : ". ");
        }
        printf("|\n");
    }
    printf("  -----------------\n");
    printf("  hex: ");
    for (int r = 0; r < 8; r++) {
        printf("0x%02x ", rows[r]);
    }
    printf("\n");
}

static void draw_all(uint8_t *buf)
{
    for (int i = 0; i < IMAGES_LEN; i++) {
        draw_image(buf, i);
    }
}

int main(int argc, char *argv[])
{
    const char *output = "pattern.bin";
    int preview = 0;
    int preview_idx = -1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0) {
            preview = 1;
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            preview = 1;
            preview_idx = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            puts("dotbin: generate AT27C64 EPROM .bin pattern file\n"
                 "  dotbin.exe                write pattern.bin + print hex\n"
                 "  dotbin.exe -o file.bin    specify output filename\n"
                 "  dotbin.exe -s             preview all images in terminal\n"
                 "  dotbin.exe -p N           preview image N (0-31)\n");
            return 0;
        }
    }

    uint8_t images_buf[TOTAL_BYTES];
    split_all(images_buf);

    if (preview) {
        if (preview_idx >= 0) {
            draw_image(images_buf, preview_idx);
        } else {
            draw_all(images_buf);
        }
        return 0;
    }

    write_bin(output, images_buf);
    printf("hex dump:\n");
    print_hex(images_buf);
    return 0;
}
