#pragma once

extern "C" {
    void canonicalize(char* input, char* output);
    unsigned int rotl32(unsigned int x, int r);
    unsigned int nextHash(unsigned int prevHash, const unsigned char* bytes, int length);
}