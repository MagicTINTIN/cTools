#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/videodev2.h>

#define SERVER_IP "192.168.1.10"
#define SERVER_PORT 9000
#define FRAME_SIZE 4096 // Adjust based on the actual frame size

int main() {
    int fd = open("/dev/video2", O_RDWR); // Open the real webcam
    if (fd == -1) {
        perror("Error opening webcam");
        return -1;
    }
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    if (sockfd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);
    
    // Connect to server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    // Start reading and sending frames
    char frame_data[FRAME_SIZE];
    while (1) {
        read(fd, frame_data, FRAME_SIZE); // Read frame from webcam
        send(sockfd, frame_data, FRAME_SIZE, 0); // Send frame to server
    }

    close(fd);
    close(sockfd);
    return 0;
}
