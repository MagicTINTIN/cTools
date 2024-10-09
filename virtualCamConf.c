#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <string.h>

int configure_virtual_webcam(const char* device) {
    // Open the virtual webcam device (e.g., /dev/video4)
    int fd = open(device, O_RDWR);
    if (fd == -1) {
        perror("Error opening virtual webcam");
        return -1;
    }

    struct v4l2_capability cap;
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
        perror("Error querying capabilities");
        close(fd);
        return -1;
    }

    printf("Driver: %s\n", cap.driver);
    printf("Card: %s\n", cap.card); // This will show the device name (e.g., "Dummy video")

    // Set format (e.g., MJPEG, YUYV, etc.)
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix.width = 640;
    fmt.fmt.pix.height = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV; // Set the pixel format
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("Error setting format");
        close(fd);
        return -1;
    }

    printf("Format set: %dx%d, %c%c%c%c\n",
           fmt.fmt.pix.width, fmt.fmt.pix.height,
           fmt.fmt.pix.pixelformat & 0xFF, (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
           (fmt.fmt.pix.pixelformat >> 16) & 0xFF, (fmt.fmt.pix.pixelformat >> 24) & 0xFF);

    // Configure frame rate (optional)
    struct v4l2_streamparm parm;
    memset(&parm, 0, sizeof(parm));
    parm.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    parm.parm.output.timeperframe.numerator = 1;
    parm.parm.output.timeperframe.denominator = 30; // 30 FPS

    if (ioctl(fd, VIDIOC_S_PARM, &parm) == -1) {
        perror("Error setting frame rate");
        close(fd);
        return -1;
    }

    printf("Frame rate set to %d FPS\n", parm.parm.output.timeperframe.denominator);

    close(fd);
    return 0;
}

int main() {
    // Example: Configure /dev/video4
    configure_virtual_webcam("/dev/video4");
    return 0;
}
