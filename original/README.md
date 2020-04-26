


# 6inch e-Paper HAT

Description:

This is an E-Ink display HAT for Raspberry Pi, 6inch, 800x600 resolution, with embedded controller IT8951, communicating via USB/SPI/I80/I2C interface.

Due to the advantages like ultra low power consumption, wide viewing angle, clear display without electricity, it is an ideal choice for applications such as shelf label, industrial instrument, and so on.


Website：

CN: http://www.waveshare.net/shop/6inch-e-Paper-HAT.htm

EN: https://www.waveshare.com/6inch-e-paper-hat.htm

WIKI：

CN: http://www.waveshare.net/wiki/6inch_e-Paper_HAT

EN: https://www.waveshare.com/wiki/6inch_e-Paper_HAT

Driver install：

git clone https://github.com/waveshare/IT8951.git

cd IT8951

make

sudo ./IT8951 0 0 01.bmp
