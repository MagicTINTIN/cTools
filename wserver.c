#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480
#define BUFFER_SIZE (FRAME_WIDTH * FRAME_HEIGHT * 3) // Assuming RGB

int main() {
    // Step 1: Set up a TCP server to receive frames
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                    (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Step 2: Open the virtual webcam
    int webcam_fd = open("/dev/video10", O_WRONLY);
    if (webcam_fd < 0) {
        perror("Opening virtual webcam");
        return -1;
    }

    // Step 3: Set video format for virtual webcam
    struct v4l2_format format;
    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    format.fmt.pix.width = FRAME_WIDTH;
    format.fmt.pix.height = FRAME_HEIGHT;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
    format.fmt.pix.field = V4L2_FIELD_NONE;
    
    if (ioctl(webcam_fd, VIDIOC_S_FMT, &format) < 0) {
        perror("Setting format on virtual webcam");
        return -1;
    }

    // Step 4: Receive frames from the network and write to virtual webcam
    ssize_t bytes_received;
    while ((bytes_received = recv(new_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        // Write the received frame to the virtual webcam
        if (write(webcam_fd, buffer, BUFFER_SIZE) < 0) {
            perror("Writing to virtual webcam");
            break;
        }
    }

    // Step 5: Clean up
    close(new_socket);
    close(server_fd);
    close(webcam_fd);
    
    return 0;
}
