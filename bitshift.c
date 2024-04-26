#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
uint8_t bitShiftRGB(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t red = r >> 5;  // 3 bits for red
    uint8_t green = g >> 5;  // 3 bits for green
    uint8_t blue = b >> 6;  // 2 bits for blue
    
    return (red << 5) | (green << 2) | blue;
}

int main() {
    uint8_t twnk9Pack[96] =  {255,205,0,  255,205,0,  255,205,0,  255,205,0,
                           255,205,0,  255,205,0,  255,205,0,  255,205,0, 
                           255,205,0,  255,205,0,  255,205,0,  255,205,0,
                           255,205,0,  255,205,0,  255,205,0,  255,205,0,
                           255,205,0,  255,205,0,  255,205,0,  255,205,0,
                           255,205,0,  255,205,0,  255,205,0,  255,205,0,
                           255,205,0,  255,205,0,  255,205,0,  255,205,0,
                           255,205,0,  255,205,0,  255,205,0,  255,205,0};

    // Calculate the number of RGB triples (each triple contains 3 elements)
    int num_triples = sizeof(twnk9Pack) / (3 * sizeof(uint8_t));
    
    // Bit shift each RGB triple into one byte
    for (int i = 0; i < num_triples; i++) {
        uint8_t r = twnk9Pack[i * 3];
        uint8_t g = twnk9Pack[i * 3 + 1];
        uint8_t b = twnk9Pack[i * 3 + 2];
        uint8_t result = bitShiftRGB(r, g, b);
        printf("%d,", result);
    }

    return 0;
}

