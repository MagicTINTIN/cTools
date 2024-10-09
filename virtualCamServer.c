#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/videodev2.h>

#define SERVER_PORT 9000
#define FRAME_SIZE 4096 // Adjust based on the actual frame size

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in serv_addr, client_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Bind failed");
        return -1;
    }

    // Listen for connections
    listen(sockfd, 1);
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = accept(sockfd, (struct sockaddr*)&client_addr, &addr_len); // Accept connection

    // Open the virtual webcam (created with v4l2loopback)
    int virtual_fd = open("/dev/video10", O_RDWR);
    if (virtual_fd == -1) {
        perror("Error opening virtual webcam");
        return -1;
    }

    // Receive and write frames to the virtual webcam
    char frame_data[FRAME_SIZE];
    while (1) {
        recv(client_fd, frame_data, FRAME_SIZE, 0); // Receive frame from client
        write(virtual_fd, frame_data, FRAME_SIZE);  // Write to virtual webcam
    }

    close(client_fd);
    close(virtual_fd);
    return 0;
}
