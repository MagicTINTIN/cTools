#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "webcam.h"

#define SERVER_IP "192.168.22.96"
#define SERVER_PORT 12345
#define BUFFER_SIZE 65507 // UDP max size

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    webcam_t *w = webcam_open("/dev/video0");
    buffer_t frame;
    frame.start = NULL;
    frame.length = 0;

    // Open UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    webcam_resize(w, 640, 480);
    webcam_stream(w, true);

    while (1) {
        webcam_grab(w, &frame);

        if (frame.length > 0) {
            // printf("Captured frame of length: %zu\n", frame.length);

            // Send frame via UDP (chunking if necessary)
            size_t offset = 0;
            while (offset < frame.length) {
                size_t chunk_size = (frame.length - offset > BUFFER_SIZE) ? BUFFER_SIZE : frame.length - offset;
                ssize_t sent_bytes = sendto(sockfd, frame.start + offset, chunk_size, 0,
                                            (const struct sockaddr *) &server_addr, sizeof(server_addr));
                if (sent_bytes == -1) {
                    perror("Failed to send frame data");
                    break;
                }
                offset += sent_bytes;
            }
        }
    }

    // Cleanup
    webcam_stream(w, false);
    webcam_close(w);
    close(sockfd);
    return 0;
}
