#include <stdlib.h>
#include <string.h>
#include "file_source.h"


void callback(uint32_t stream_id, uint8_t* frame, uint32_t size, uint32_t timestamp) {
    char buffer[size + 10];
    memcpy(buffer, frame, size);
    buffer[size] = 0;
    printf("%6d: %p %d: %s\n", timestamp, frame, size, buffer);
}

int main(int argc, char* argv[]) {
    int fps = 2;
    int buffsize = 333;
    const char* filename = "stream_test.txt";
    int clock = 90000;
    bool threads = true;
    const char* usage = "Usage: test_file_source [-F fps] [-f filename] [-c clock] [-b buffsize] [-T]";
    int c;
    while ( (c = getopt(argc, argv, "F:f:b:c:Th")) != -1)
        switch (c) {
            case 'F': fps      = strtol(optarg, 0, 0); break;
            case 'b': buffsize = strtol(optarg, 0, 0); break;
            case 'c': clock    = strtol(optarg, 0, 0); break;
            case 'f': filename = optarg;               break;
            case 'T': threads  = false;                break;
            case 'h':
            default:  printf("%s\n", usage); exit(1);
        }
    RTSP::FileSource* file_source = RTSP::FileSource::create(filename, 0, fps, clock, buffsize);
    if (!threads)
        file_source->start_thread(); // this won't return
    file_source->play();
    sleep(10);
    file_source->teardown();
    printf("Done!\n");
    exit(0);
}

