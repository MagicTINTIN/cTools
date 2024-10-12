#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <asm-generic/socket.h>

#define WIDTH 640
#define HEIGHT 480
#define PORT 8080
#define BUFFER_COUNT 4

struct buffer {
    void *start;
    size_t length;
};

struct webcam {
    int fd;
    struct buffer *buffers;
    size_t buffer_count;
    pthread_t thread;
    int streaming;
};

static int xioctl(int fd, int request, void *arg) {
    int r;
    do {
        r = ioctl(fd, request, arg);
    } while (-1 == r && EINTR == errno);
    return r;
}

void webcam_close(struct webcam *w) {
    for (size_t i = 0; i < w->buffer_count; i++) {
        munmap(w->buffers[i].start, w->buffers[i].length);
    }
    free(w->buffers);
    close(w->fd);
}

struct webcam *webcam_open(const char *dev_name) {
    struct webcam *w = malloc(sizeof(struct webcam));
    if (!w) {
        perror("Failed to allocate memory");
        return NULL;
    }
    
    w->fd = open(dev_name, O_RDWR);
    if (w->fd == -1) {
        perror("Cannot open device");
        free(w);
        return NULL;
    }

    struct v4l2_capability cap;
    if (xioctl(w->fd, VIDIOC_QUERYCAP, &cap) == -1) {
        perror("Cannot query device capabilities");
        free(w);
        return NULL;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "Device is not a video capture device\n");
        free(w);
        return NULL;
    }

    struct v4l2_format fmt;
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = WIDTH;
    fmt.fmt.pix.height = HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (xioctl(w->fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("Failed to set format");
        free(w);
        return NULL;
    }

    struct v4l2_requestbuffers req;
    req.count = BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (xioctl(w->fd, VIDIOC_REQBUFS, &req) == -1) {
        perror("Failed to request buffers");
        free(w);
        return NULL;
    }

    w->buffers = calloc(req.count, sizeof(struct buffer));
    for (size_t i = 0; i < req.count; i++) {
        struct v4l2_buffer buf;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (xioctl(w->fd, VIDIOC_QUERYBUF, &buf) == -1) {
            perror("Failed to query buffer");
            free(w->buffers);
            free(w);
            return NULL;
        }

        w->buffers[i].length = buf.length;
        w->buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, w->fd, buf.m.offset);

        if (MAP_FAILED == w->buffers[i].start) {
            perror("Failed to mmap buffer");
            free(w->buffers);
            free(w);
            return NULL;
        }
    }

    return w;
}

void webcam_stream(struct webcam *w, int server_fd) {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    for (size_t i = 0; i < w->buffer_count; i++) {
        struct v4l2_buffer buf;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        xioctl(w->fd, VIDIOC_QBUF, &buf);
    }

    xioctl(w->fd, VIDIOC_STREAMON, &type);

    while (1) {
        struct v4l2_buffer buf;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        
        xioctl(w->fd, VIDIOC_DQBUF, &buf);
        
        // Send buffer over socket
        send(server_fd, w->buffers[buf.index].start, buf.bytesused, 0);
        
        xioctl(w->fd, VIDIOC_QBUF, &buf);
    }

    xioctl(w->fd, VIDIOC_STREAMOFF, &type);
}

int main() {
    struct sockaddr_in server_addr;
    int server_fd, client_fd;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("localhost");
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if ((client_fd = accept(server_fd, NULL, NULL)) < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    struct webcam *w = webcam_open("/dev/video2");
    if (!w) {
        close(client_fd);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    webcam_stream(w, client_fd);

    webcam_close(w);
    close(client_fd);
    close(server_fd);

    return 0;
}
