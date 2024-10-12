#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <errno.h>

#define SERVER_PORT 12345
#define BUFFER_SIZE 65507

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    int vfd;  // Virtual webcam file descriptor

    // Open UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }


    // Bind to server port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;


    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Open the virtual webcam (created by v4l2loopback)
    vfd = open("/dev/video10", O_RDWR);
    if (vfd == -1) {
        perror("Failed to open virtual webcam");
        exit(EXIT_FAILURE);
    }


    socklen_t len = sizeof(client_addr);
    ssize_t n;

    // Set the format for the virtual webcam
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix.width = 640;
    fmt.fmt.pix.height = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;


    if (ioctl(vfd, VIDIOC_S_FMT, &fmt) < 0) {
        perror("Could not set format for virtual webcam");
        close(vfd);
        exit(EXIT_FAILURE);
    }


    while (1) {
        // Receive frame from the client
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_WAITALL,
                     (struct sockaddr *)&client_addr, &len);



        if (n == -1) {
            perror("Failed to receive frame data");
            continue;
        }



        // Write the received data to the virtual webcam
        if (write(vfd, buffer, n) == -1) {
            perror("Failed to write frame to virtual webcam");
        }



    }


    close(vfd);
    close(sockfd);
    return 0;
}