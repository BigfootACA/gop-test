#include<Uefi.h>
typedef struct image_dsc{
	CONST UINT32 w;
	CONST UINT32 h;
	CONST UINT8 bpp;
	CONST UINTN size;
	CONST UINT8*image;
}image_dsc;

#define IMPORT_BIN(file,sym) asm(\
	".balign 4\n"\
	".global "#sym"\n"\
	".global "#sym"_start\n"\
	".global "#sym"_stop\n"\
	".global "#sym"_size\n"\
	#sym":\n"\
	#sym"_start:\n"\
	".incbin \""file"\"\n"\
	#sym"_stop:\n"\
	#sym"_size:\n"\
	".quad "#sym"_stop-"#sym"_start\n"\
	".balign 4\n"\
)

extern image_dsc default_image;