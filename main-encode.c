#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int unicode_to_utf8(uint32_t code, uint8_t result[5]){
    int len;
    if (code < 128) {
        len = 1;
        result[0] = code;
    } else if (code < 2048) {
        len = 2;
        result[1] = (128 + code % 64);
        code /= 64;
        result[0] = (128 + 64 + code);
    } else if (code < 65536) {
        len = 3;
        result[2] = (128 + code % 64);
        code /= 64;
        result[1] = (128 + code % 64);
        code /= 64;
        result[0] = (128 + 64 + 32 + code);
    } else if (code < 1112064) {
        len = 4;
        result[3] = (128 + code % 64);
        code /= 64;
        result[2] = (128 + code % 64);
        code /= 64;
        result[1] = (128 + code % 64);
        code /= 64;
        result[0] = (128 + 64 + 32 + 16 + code);
    } else len = 0;

    result[len] = 0;
    return len;
}

uint32_t w1251_to_unicode(uint8_t sym){
    uint32_t res = 0;
    uint32_t from_80_to_BF[] = {
        0x402, 0x403, 0x201A, 0x453, 0x201E, 0x2026, 0x2020, 0x2021,
        0x20AC, 0x2030, 0x409, 0x2039, 0x40A, 0x40C, 0x40B, 0x40F,

        0x452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
        0x0, 0x2122, 0x459, 0x203A, 0x45A, 0x45C, 0x45B, 0x45F,

        0xA0, 0x40E, 0x45E, 0x408, 0xA4, 0x490, 0xA6, 0xA7,
        0x401, 0xA9, 0x404, 0xAB, 0xAC, 0xAD, 0xAE, 0x407,

        0xB0, 0xB1, 0x406, 0x456, 0x491, 0xB5, 0xB6, 0xB7,
        0x451, 0x2116, 0x454, 0xBB, 0x458, 0x405, 0x455, 0x457
    };

    if(sym >= 0xC0)
        res = sym - 0xC0 + 0x410;
    else if(sym < 0x80)
        res = sym;
    else
        res = from_80_to_BF[sym - 0x80];
    return res;

}

uint32_t koi8_to_unicode(uint8_t sym){
    uint32_t from_80 [] = {
        0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518, 0x251C, 0x2524,
        0x252C, 0x2534, 0x253C, 0x2580, 0x2584, 0x2588, 0x258C, 0x2590,

        0x2591, 0x2592, 0x2593, 0x2320, 0x25A0, 0x2219, 0x221A, 0x2248,
        0x2264, 0x2265, 0xA0, 0x2321, 0xB0, 0xB2, 0xB7, 0xF7,

        0x2550, 0x2551, 0x2552, 0x451, 0x2553, 0x2554, 0x2555, 0x2556,
        0x2557, 0x2558, 0x2559, 0x255A, 0x255B, 0x255C, 0x255D, 0x255E,

        0x255F, 0x2560, 0x2561, 0x401, 0x2562, 0x2563, 0x2564, 0x2565,
        0x2566, 0x2567, 0x2568, 0x2569, 0x256A, 0x256B, 0x256C, 0xA9,

        0x44E, 0x430, 0x431, 0x446, 0x434, 0x435, 0x444, 0x433,
        0x445, 0x438, 0x439, 0x43A, 0x43B, 0x43C, 0x43D, 0x43E,

        0x43F, 0x44F, 0x440, 0x441, 0x442, 0x443, 0x436, 0x432,
        0x44C, 0x44B, 0x437, 0x448, 0x44D, 0x449, 0x447, 0x44A,

        0x42E, 0x410, 0x411, 0x426, 0x414, 0x415, 0x424, 0x413,
        0x425, 0x418, 0x419, 0x41A, 0x41B, 0x41C, 0x41D, 0x41E,

        0x41F, 0x42F, 0x420, 0x421, 0x422, 0x423, 0x416, 0x412,
        0x42C, 0x42B, 0x417, 0x428, 0x42D, 0x429, 0x427, 0x42A
    };

    uint32_t res;
    if(sym < 0x80)
        res = sym;
    else
        res = from_80[sym - 0x80];
    return res;
}

uint32_t iso_to_unicode(uint8_t sym){
    uint32_t res = 0;
    if(sym <= 0xA0 || sym == 0xAD)
        res += sym;
    else if (sym == 0xF0)
        res = 0x2116;
    else if (sym == 0xFD)
        res = 0xA7;
    else
        res = sym - 0xA0 + 0x400;
    return res;
}

void print_bin(uint8_t num){
    uint8_t comp = 128;
    while(comp){
        if(comp & num)
            printf("1");
        else
            printf("0");
        comp /= 2;
    }
}

enum CODING{
    KOI8_R,
    WINDOWS_1251,
    ISO_8859_5
};

int main()
{
    printf("Print name of file to change encoding\n");
    printf("Print Ctrl+C to exit\n");
    while(1){
        char filename[256];
        char resultname[256];
        enum CODING type_code;
        int buf;


        printf("Input name of coded file: ");
        scanf("%s", filename);

        FILE* my_file = fopen(filename, "rb");
        if(my_file == NULL){
            perror("Error occured while opening the file\n");
            continue;
        }

        printf("Input name of file to write in UTF-8: ");
        scanf("%s", resultname);

        FILE* my_output = fopen(resultname, "wb");
        if(my_output == NULL){
            perror("Error occured while creating the file to write answer\n");
            continue;
        }

        while(1){
            printf("Input number of the type of coding: \n");
            printf("KOI8-R is 1, Windows-1251 is 2, ISO-8859-5 is 3 : ");
            scanf("%d", &buf);
            if(buf < 1 || buf > 3){
                perror("Wrong coding input\n");
                continue;
            } else if(buf == 1) {
                type_code = KOI8_R;
                break;
            } else if(buf == 2) {
                type_code = WINDOWS_1251;
                break;
            }
            else if(buf == 3) {
                type_code = ISO_8859_5;
                break;
            }
        }

        const uint8_t prefix[] = {0xEF, 0xBB, 0xBF};
        fwrite(prefix, sizeof(prefix[0]), 3, my_output);


        while(1){
            buf = getc(my_file);
            if(buf == EOF)
                break;

            int len;
            uint8_t convert[5];

            uint32_t uni;
            if(type_code == KOI8_R)
                uni = koi8_to_unicode((uint8_t)buf);
            else if(type_code == WINDOWS_1251)
                uni = w1251_to_unicode((uint8_t)buf);
            else if(type_code == ISO_8859_5)
                uni = iso_to_unicode((uint8_t)buf);

            len = unicode_to_utf8(uni, convert);

            if(len)
                fwrite(convert, sizeof(uint8_t), len, my_output);
        }
        printf("The file is converted\n");
        fclose(my_output);
        fclose(my_file);
    }
   return 0;
}
