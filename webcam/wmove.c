#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "webcam.h"

static int _ioctl(int fh, int request, void *arg)
{
    int r;

    do
    {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}

static void moving(struct webcam *w, int vfd)
{
    struct v4l2_buffer buf;

    // Try getting an image from the device
    for (;;)
    {
        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        // Dequeue a (filled) buffer from the video device
        if (-1 == _ioctl(w->fd, VIDIOC_DQBUF, &buf))
        {
            switch (errno)
            {
            case EAGAIN:
                continue;

            case EIO:
            default:
                fprintf(stderr, "%d: Could not read from device %s\n", errno, w->name);
                break;
            }
        }

        // Make sure we are not out of bounds
        assert(buf.index < w->nbuffers);

        // Lock frame mutex, and store RGB
        pthread_mutex_lock(&w->mtx_frame);
        // convertToRGB(w->buffers[buf.index], &w->frame);

        if (write(vfd, w->buffers[buf.index].start, w->buffers[buf.index].length) == -1)
            {
                perror("Failed to write frame to virtual webcam");
                printf("vfd: %d ; frame start: %x, length: %ld\n", vfd, w->buffers[buf.index].start, w->buffers[buf.index].length);
                exit(EXIT_FAILURE);
            }

        pthread_mutex_unlock(&w->mtx_frame);
        break;
    }

    // Queue buffer back into the video device
    if (-1 == _ioctl(w->fd, VIDIOC_QBUF, &buf))
    {
        fprintf(stderr, "Error while swapping buffers on %s\n", w->name);
        return;
    }
}

int main()
{
    webcam_t *w = webcam_open("/dev/video0");

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
    webcam_close(w);
    close(vfd);
    return 0;
}
