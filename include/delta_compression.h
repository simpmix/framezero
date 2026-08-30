#ifndef FRAMEZERO_DELTA_COMPRESSION_H
#define FRAMEZERO_DELTA_COMPRESSION_H

#include "input.h"
#include <cstring>

namespace FrameZero {

// Delta compression for efficient network transmission
class DeltaCompressor {
public:
    // Flags to indicate which fields changed
    enum DeltaFlags : uint8_t {
        FLAG_NONE     = 0,
        FLAG_MOVE_X   = (1 << 0),
        FLAG_MOVE_Y   = (1 << 1),
        FLAG_BUTTONS  = (1 << 2),
        FLAG_ALL      = FLAG_MOVE_X | FLAG_MOVE_Y | FLAG_BUTTONS
    };
    
    struct DeltaInput {
        uint8_t flags;
        int8_t moveX;      // Only if FLAG_MOVE_X set
        int8_t moveY;      // Only if FLAG_MOVE_Y set
        uint8_t buttons;   // Only if FLAG_BUTTONS set
        
        DeltaInput() : flags(FLAG_NONE), moveX(0), moveY(0), buttons(0) {}
    };
    
    Input lastInput;
    bool hasLastInput;
    
    DeltaCompressor() : hasLastInput(false) {}
    
    // Compress current input relative to last
    DeltaInput compress(const Input& current) {
        DeltaInput delta;
        delta.flags = FLAG_NONE;
        
        if (!hasLastInput) {
            // First input - send everything
            delta.flags = FLAG_ALL;
            delta.moveX = current.moveX;
            delta.moveY = current.moveY;
            delta.buttons = current.buttons;
        } else {
            // Send only changed fields
            if (current.moveX != lastInput.moveX) {
                delta.flags |= FLAG_MOVE_X;
                delta.moveX = current.moveX;
            }
            
            if (current.moveY != lastInput.moveY) {
                delta.flags |= FLAG_MOVE_Y;
                delta.moveY = current.moveY;
            }
            
            if (current.buttons != lastInput.buttons) {
                delta.flags |= FLAG_BUTTONS;
                delta.buttons = current.buttons;
            }
        }
        
        lastInput = current;
        hasLastInput = true;
        
        return delta;
    }
    
    // Decompress delta back to full input
    Input decompress(const DeltaInput& delta) {
        Input result;
        
        if (delta.flags & FLAG_MOVE_X) {
            result.moveX = delta.moveX;
        } else if (hasLastInput) {
            result.moveX = lastInput.moveX;
        }
        
        if (delta.flags & FLAG_MOVE_Y) {
            result.moveY = delta.moveY;
        } else if (hasLastInput) {
            result.moveY = lastInput.moveY;
        }
        
        if (delta.flags & FLAG_BUTTONS) {
            result.buttons = delta.buttons;
        } else if (hasLastInput) {
            result.buttons = lastInput.buttons;
        }
        
        lastInput = result;
        hasLastInput = true;
        
        return result;
    }
    
    // Calculate bytes saved by delta compression
    static size_t getDeltaSize(const DeltaInput& delta) {
        size_t size = 1;  // Flags byte always sent
        
        if (delta.flags & FLAG_MOVE_X) size += 1;
        if (delta.flags & FLAG_MOVE_Y) size += 1;
        if (delta.flags & FLAG_BUTTONS) size += 1;
        
        return size;
    }
    
    static size_t getFullInputSize() {
        return 3;  // moveX + moveY + buttons
    }
    
    void reset() {
        hasLastInput = false;
        lastInput = Input();
    }
};

} // namespace FrameZero

#endif // FRAMEZERO_DELTA_COMPRESSION_H
