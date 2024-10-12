#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "webcam.h"

int main()
{
    webcam_t *w = webcam_open("/dev/video0");
    buffer_t frame;
    frame.start = NULL;
    frame.length = 0;

    int vfd = open("/dev/video10", O_RDWR);
    if (vfd == -1) {
        perror("Failed to open virtual webcam");
        exit(EXIT_FAILURE);
    }

    webcam_resize(w, 640, 480);
    webcam_stream(w, true);

    while (1)
    {
        webcam_grab(w, &frame);

        if (frame.length > 0)
        {
            if (write(vfd, w->frame.start, w->frame.length) == -1)
            {
                perror("Failed to write frame to virtual webcam");
                printf("vfd: %d ; frame start: %x, length: %ld\n", vfd, w->frame.start, w->frame.length);
                exit(EXIT_FAILURE);
            }
        }
    }

    // Cleanup
    webcam_stream(w, false);
    webcam_close(w);
    close(vfd);
    return 0;
}
