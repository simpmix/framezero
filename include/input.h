#ifndef FRAMEZERO_INPUT_H
#define FRAMEZERO_INPUT_H

#include <cstdint>
#include <cstring>

namespace FrameZero {

// Compact 9-byte input structure for network efficiency
struct Input {
    int8_t moveX;      // -127 to 127 (left/right)
    int8_t moveY;      // -127 to 127 (up/down)
    uint8_t buttons;   // 8 button flags
    
    Input() : moveX(0), moveY(0), buttons(0) {}
    
    bool operator==(const Input& o) const {
        return moveX == o.moveX && moveY == o.moveY && buttons == o.buttons;
    }
    
    bool operator!=(const Input& o) const { return !(*this == o); }
    
    // Serialize to bytes (exactly 3 bytes)
    void serialize(uint8_t* out) const {
        out[0] = static_cast<uint8_t>(moveX + 128);  // Map -127..127 to 1..255
        out[1] = static_cast<uint8_t>(moveY + 128);
        out[2] = buttons;
    }
    
    // Deserialize from bytes
    static Input deserialize(const uint8_t* in) {
        Input inp;
        inp.moveX = static_cast<int8_t>(in[0] - 128);
        inp.moveY = static_cast<int8_t>(in[1] - 128);
        inp.buttons = in[2];
        return inp;
    }
};

// Input queue with prediction support
class InputQueue {
public:
    static constexpr int MAX_SIZE = 128;
    
    Input inputs[MAX_SIZE];
    int head;
    int tail;
    int size;
    
    InputQueue() : head(0), tail(0), size(0) {}
    
    void push(const Input& inp) {
        if (size >= MAX_SIZE) {
            // Drop oldest if full
            head = (head + 1) % MAX_SIZE;
            size--;
        }
        inputs[tail] = inp;
        tail = (tail + 1) % MAX_SIZE;
        size++;
    }
    
    Input peek(int index) const {
        if (index < 0 || index >= size) return Input();
        int idx = (head + index) % MAX_SIZE;
        return inputs[idx];
    }
    
    Input pop() {
        if (size == 0) return Input();
        Input result = inputs[head];
        head = (head + 1) % MAX_SIZE;
        size--;
        return result;
    }
    
    void clear() { head = tail = size = 0; }
    
    int getSize() const { return size; }
    
    bool isEmpty() const { return size == 0; }
};

} // namespace FrameZero

#endif // FRAMEZERO_INPUT_H
