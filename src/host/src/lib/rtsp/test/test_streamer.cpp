#include "streamer.h"
#include <iostream>

namespace RTP {
class H264Streamer : public Streamer {
public:
    H264Streamer() : Streamer(0x15, 0x27272727) , _counter(0) {}
    virtual uint32_t fetch_timestamp(uint8_t* frame) {
        return ++_counter;
    }
private:
    uint32_t    _counter;
};

}


int main() {
    std::cout << sizeof(uint8_t) << "\n";
    const int frame_size = 5000;
    char frame[frame_size];
    for (int i = 0; i < frame_size; i++) {
        frame[i] = i & 0x00ff;
    }
    RTP::H264Streamer streamer;
    uint8_t* ptr = (uint8_t*) &frame[24];
    streamer.add_client("192.168.1.99", 5678);
    // send small frame
    streamer.send_frame(ptr, 1000);
    for (int i = 0; i < frame_size; i++) {
        frame[i] = i & 0x00ff;
    }
    // send large frame
    streamer.send_frame(ptr, frame_size - 100);
    return 0;
}
