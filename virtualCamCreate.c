#include <stdio.h>
#include <stdlib.h>

int main() {
    // Load the v4l2loopback module with the desired name for the virtual device
    // sudo modprobe v4l2loopback devices=1 video_nr=5 card_label="DistantCam"
    // modprobe v4l2loopback video_nr=3,4,7 card_label="device number 3","the number four","the last one"
    system("sudo modprobe v4l2loopback devices=1 video_nr=3 card_label=\"DistantCam\"");
    
    // Check if the device was created (e.g., /dev/video10)
    printf("Virtual webcam 'DistantCam' created at /dev/video3\n");

    // while(1);
    return 0;
}
