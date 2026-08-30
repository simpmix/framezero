#include <iostream>
#include <cstdint>

static const int32_t CORDIC_ATAN[16] = {
    51471, 30385, 16054, 8149, 4090, 2047, 1023, 511, 255, 127, 63, 31, 15, 7, 3, 1
};
static const int32_t CORDIC_K = 39811;

int main() {
    int32_t x = CORDIC_K;
    int32_t y = 0;
    int32_t z = 0;

    for (int i = 0; i < 16; i++) {
        int32_t d = z < 0 ? -1 : 1;
        int32_t tx = x - d * (y >> i);
        int32_t ty = y + d * (x >> i);
        int32_t tz = z - d * CORDIC_ATAN[i];
        x = tx;
        y = ty;
        z = tz;
    }
    std::cout << "x (cos): " << x << "\n";
    std::cout << "y (sin): " << y << "\n";
    return 0;
}
