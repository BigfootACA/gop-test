#include"image.h"

IMPORT_BIN("sweat-smile.bin",default_image_data);

extern const UINT8 default_image_data_start[];
extern const UINT8 default_image_data_stop[];
extern const UINTN default_image_data_size;

image_dsc default_image={
	.w = 120,
	.h = 120,
	.bpp = 4,
	.size = 120*120*4,
	.image = default_image_data_start
};
