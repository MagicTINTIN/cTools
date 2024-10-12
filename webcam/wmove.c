#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <linux/videodev2.h>
#include <sys/ioctl.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

/**
 * Buffer structure
 */
typedef struct buffer {
    uint8_t *start;
    size_t  length;
} buffer_t;

/**
 * Webcam structure
 */
typedef struct webcam {
    char            *name;
    int             fd;
    buffer_t        *buffers;
    uint8_t         nbuffers;

    buffer_t        frame;
    pthread_t       thread;
    pthread_mutex_t mtx_frame;

    uint16_t        width;
    uint16_t        height;
    uint8_t         colorspace;

    char            formats[16][5];
    bool            streaming;
} webcam_t;

/**
 * Keeping tabs on opened webcam devices
 */
static webcam_t *_w[16] = {
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL
};

static int _ioctl(int fh, int request, void *arg)
{
    int r;

    do
    {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

struct webcam *webcam_capture_open(const char *dev)
{
    struct stat st;

    struct v4l2_capability cap;
    struct v4l2_format fmt;

    uint16_t min;

    int fd;
    struct webcam *w;

    // Check if the device path exists
    if (-1 == stat(dev, &st)) {
        fprintf(stderr, "Cannot identify '%s': %d, %s\n",
                dev, errno, strerror(errno));
        return NULL;
    }

    // Should be a character device
    if (!S_ISCHR(st.st_mode)) {
        fprintf(stderr, "%s is no device\n", dev);
        return NULL;
    }

    // Create a file descriptor
    fd = open(dev, O_RDWR | O_NONBLOCK, 0);
    if (-1 == fd) {
        fprintf(stderr, "Cannot open'%s': %d, %s\n",
                dev, errno, strerror(errno));
        return NULL;
    }

    // Query the webcam capabilities
    if (-1 == _ioctl(fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s is no V4L2 device\n", dev);
            return NULL;
        } else {
            fprintf(stderr, "%s: could not fetch video capabilities\n", dev);
            return NULL;
        }
    }

    // Needs to be a capturing device
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "%s is no video capture device\n", dev);
        return NULL;
    }

    // Prepare webcam structure
    w = calloc(1, sizeof(struct webcam));
    w->fd = fd;
    w->name = strdup(dev);
    w->frame.start = NULL;
    w->frame.length = 0;
    pthread_mutex_init(&w->mtx_frame, NULL);

    // Initialize buffers
    w->nbuffers = 0;
    w->buffers = NULL;

    // Store webcam in _w
    int i = 0;
    for(; i < 16; i++) {
        if (_w[i] == NULL) {
            _w[i] = w;
            break;
        }
    }

    // Request supported formats
    struct v4l2_fmtdesc fmtdesc;
    uint32_t idx = 0;
    char *pixelformat = calloc(5, sizeof(char));
    for(;;) {
        fmtdesc.index = idx;
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (-1 == _ioctl(w->fd, VIDIOC_ENUM_FMT, &fmtdesc)) break;

        memset(w->formats[idx], 0, 5);
        memcpy(&w->formats[idx][0], &fmtdesc.pixelformat, 4);
        fprintf(stderr, "%s: Found format: %s - %s\n", w->name, w->formats[idx], fmtdesc.description);
        idx++;
    }

    return w;
}

void webcam_resize(webcam_t *w, uint16_t width, uint16_t height)
{
    uint32_t i;
    struct v4l2_format fmt;
    struct v4l2_buffer buf;

    // Use YUYV as default for now
    CLEAR(fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.colorspace = V4L2_COLORSPACE_REC709;
    fprintf(stderr, "%s: requesting image format %ux%u\n", w->name, width, height);
    _ioctl(w->fd, VIDIOC_S_FMT, &fmt);

    // Storing result
    w->width = fmt.fmt.pix.width;
    w->height = fmt.fmt.pix.height;
    w->colorspace = fmt.fmt.pix.colorspace;

    char *pixelformat = calloc(5, sizeof(char));
    memcpy(pixelformat, &fmt.fmt.pix.pixelformat, 4);
    fprintf(stderr, "%s: set image format to %ux%u using %s\n", w->name, w->width, w->height, pixelformat);

    // Buffers have been created before, so clear them
    if (NULL != w->buffers) {
        for (i = 0; i < w->nbuffers; i++) {
            munmap(w->buffers[i].start, w->buffers[i].length);
        }

        free(w->buffers);
    }

    // Request the webcam's buffers for memory-mapping
    struct v4l2_requestbuffers req;
    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == _ioctl(w->fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s does not support memory mapping\n", w->name);
            return;
        } else {
            fprintf(stderr, "Unknown error with VIDIOC_REQBUFS: %d\n", errno);
            return;
        }
    }

    // Needs at least 2 buffers
    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory on %s\n", w->name);
        return;
    }

    // Storing buffers in webcam structure
    fprintf(stderr, "Preparing %d buffers for %s\n", req.count, w->name);
    w->nbuffers = req.count;
    w->buffers = calloc(w->nbuffers, sizeof(struct buffer));

    if (!w->buffers) {
        fprintf(stderr, "Out of memory\n");
        return;
    }

    // Prepare buffers to be memory-mapped
    for (i = 0; i < w->nbuffers; ++i) {
        CLEAR(buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (-1 == _ioctl(w->fd, VIDIOC_QUERYBUF, &buf)) {
            fprintf(stderr, "Could not query buffers on %s\n", w->name);
            return;
        }

        w->buffers[i].length = buf.length;
        w->buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, w->fd, buf.m.offset);

        if (MAP_FAILED == w->buffers[i].start) {
            fprintf(stderr, "Mmap failed\n");
            return;
        }
    }
}

static void moving(struct webcam *wIn, int vfd)
{
    struct v4l2_buffer buf;

    // Try getting an image from the device
    for (;;)
    {
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        // Dequeue a (filled) buffer from the video device
        if (-1 == _ioctl(wIn->fd, VIDIOC_DQBUF, &buf))
        {
            switch (errno)
            {
            case EAGAIN:
                continue;

            case EIO:
            default:
                fprintf(stderr, "%d: Could not read from device %s\n", errno, wIn->name);
                break;
            }
        }

        // Make sure we are not out of bounds
        assert(buf.index < wIn->nbuffers);

        // Lock frame mutex, and store RGB
        pthread_mutex_lock(&wIn->mtx_frame);
        // convertToRGB(w->buffers[buf.index], &w->frame);

        if (write(vfd, wIn->buffers[buf.index].start, wIn->buffers[buf.index].length) == -1)
            {
                perror("Failed to write frame to virtual webcam");
                printf("vfd: %d ; frame start: %x, length: %ld\n", vfd, wIn->buffers[buf.index].start, wIn->buffers[buf.index].length);
                exit(EXIT_FAILURE);
            }

        pthread_mutex_unlock(&wIn->mtx_frame);
        break;
    }

    // Queue buffer back into the video device
    if (-1 == _ioctl(wIn->fd, VIDIOC_QBUF, &buf))
    {
        fprintf(stderr, "Error Inhile swapping buffers on %s\n", wIn->name);
        return;
    }
}

int main()
{
    webcam_t *w = webcam_capture_open("/dev/video0");

    enum v4l2_buf_type type;
    buffer_t frame;
    frame.start = NULL;
    frame.length = 0;

    int vfd = open("/dev/video10", O_RDWR);
    if (vfd == -1)
    {
        perror("Failed to open virtual webcam");
        exit(EXIT_FAILURE);
    }

    webcam_resize(w, 640, 480);
    // webcam_stream(w, true);

    // Turn on streaming
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == _ioctl(w->fd, VIDIOC_STREAMON, &type))
    {
        fprintf(stderr, "Could not turn on streaming on %s\n", w->name);
        return 2;
    }

    moving(w, vfd);

    // Turn off streaming
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == _ioctl(w->fd, VIDIOC_STREAMOFF, &type))
    {
        fprintf(stderr, "Could not turn streaming off on %s\n", w->name);
        return 2;
    }

    // Cleanup
    // webcam_stream(w, false);
    // webcam_close(w);
    close(vfd);
    return 0;
}
