
from ctypes import cdll, Structure, c_uint16, c_uint8

clib = cdll.LoadLibrary("/home/pi/projects/IT8951-remote/cmake-build-release-remote/libIT8951.so")

# get build dir and add to library path


# TODO: figure out how to get this from GetDeviceInfo, just seeing segfault
class IT8951DevInfo(Structure):
    pass
IT8951DevInfo._fields_ = [('usPanelW', c_uint16),
                       ('usPanelH', c_uint16),
                       ('usImgBufAddrL', c_uint16),
                       ('usImgBufAddrH', c_uint16),
                       ('usFWVersion', c_uint16 * 8),
                       ('usLUTVersion', c_uint16 * 8)]


class Device:

    def __init__(self):
        clib.Initialize()

        self.width = clib.GetWidth()
        self.height = clib.GetHeight()

    def get_dimensions(self):
        return self.width, self.height

    def _validate_image_data(self, image_data):
        # TODO: check type
        # if not type(image_data) == bytes:
        #     raise TypeError("Image Data should be type bytes")
        if not len(image_data) == self.width * self.height:
            raise ValueError(f"Image Data should be a {self.width*self.height} array, was {len(image_data)}")

        # check values
        for val in image_data:
            # TODO: think we need another check - rework this
            if val > 240:
                raise ValueError("Image Data malformed")

    def display_image(self, image_data):
        self._validate_image_data(image_data)
        size = self.width * self.height
        buff = (c_uint8 * size)(*image_data)
        clib.DisplayImage(buff)

