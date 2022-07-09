#include <director/util.h>

uint32_t util_decode_version(uint16_t ver) {
	if (ver >= 0x79F) {
        return 1201;
    } else if (ver >= 0x783) {
		return 1200;
    } else if (ver >= 0x782) {
        return 1150;
    } else if (ver >= 0x781) {
        return 1100;
    } else if (ver >= 0x73b) {
        return 1000;
    } else if (ver >= 0x6a4) {
        return 850;
    } else if (ver >= 0x582) {
        return 800;
    } else if (ver >= 0x4c8) {
        return 700;
    } else if (ver >= 0x4c2) {
        return 600;
    } else if (ver >= 0x4b1) {
        return 500;
    } else if (ver >= 0x45d) {
        return 404;
    } else if (ver >= 0x45b) {
        return 400;
    } else if (ver >= 0x405) {
        return 310;
    } else if (ver >= 0x404) {
        return 300;
    }
    return 200;
}
