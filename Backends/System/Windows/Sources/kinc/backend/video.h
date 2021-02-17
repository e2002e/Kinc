#pragma once

#include <kinc/graphics4/texture.h>
#include <kinc/io/filereader.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *renderer;
	double duration;
	double position;
	bool finished;
	bool paused;
} kinc_video_impl_t;

#ifdef __cplusplus
}
#endif
