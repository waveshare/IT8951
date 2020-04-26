//
// Created by Reese on 4/4/20.
//

#include "interface.h"

int main(int argc, char *argv[]) {

    if(Initialize()) {
        printf("IT8951_Init error \n");
        return 1;
    }

    IT8951DevInfo deviceInfo = GetDeviceInfo();

    int width = deviceInfo.usPanelW;
    int height = deviceInfo.usPanelH;

    uint8_t * image = malloc(width*height);
    for (int i=0; i<width*height; i++) {
        uint8_t val = ((rand() % 16u) << 4u);
        image[i] = val;
    }
    DisplayImage(image);

    free(image);

    Close();

    return 0;
}