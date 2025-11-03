#include <string.h>
#include "BlockchainDLL.h"

#define SEED 0x5A

extern "C" {

    __declspec(dllexport) void canonicalize(char* input, char* output) {
        strcpy(output, input);
        int len = strlen(output);
        if (len > 0 && output[len-1] == '\n') {
            output[len-1] = '\0';
        }
    }

    __declspec(dllexport) unsigned int rotl32(unsigned int x, int r) {
        return ((x << r) | (x >> (32 - r))) & 0xFFFFFFFF;
    }

    __declspec(dllexport) unsigned int nextHash(unsigned int prevHash, const unsigned char* bytes, int length) {
        unsigned int h = prevHash & 0xFFFFFFFF;
        int i;
        
        for (i = 0; i < length; i++) {
            h = h ^ bytes[i];
            h = rotl32(h, 5);
            h = (h * 0x01000193) & 0xFFFFFFFF;
        }
        
        h = h ^ length;
        return h;
    }

}