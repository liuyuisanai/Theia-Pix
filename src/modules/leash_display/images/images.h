#ifndef _IMAGES_H_
#define _IMAGES_H_
#ifdef __cplusplus
extern "C" {
#endif
#define IMAGE_SCREENS_LOGO 0

struct ImageInfo {
	int w;
	int h;
	int offset;
};
extern const struct ImageInfo imageInfo[];
extern const unsigned char imageData[];
#ifdef __cplusplus
}
#endif
#endif
