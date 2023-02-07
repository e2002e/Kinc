#include <kinc/audio2/audio.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <string.h>
#include <stdlib.h>

static void (*a2_callback)(kinc_a2_buffer_t *buffer, int samples) = NULL;
static kinc_a2_buffer_t a2_buffer;

static SLObjectItf engineObject;
static SLEngineItf engineEngine;
static SLObjectItf outputMixObject;
static SLObjectItf bqPlayerObject;
static SLPlayItf bqPlayerPlay = NULL;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
#define AUDIO_BUFFER_SIZE 1 * 1024
static int16_t tempBuffer[AUDIO_BUFFER_SIZE];

static void copySample(void *buffer) {
	float value = *(float *)&a2_buffer.data[a2_buffer.read_location];
	a2_buffer.read_location += 4;
	if (a2_buffer.read_location >= a2_buffer.data_size)
		a2_buffer.read_location = 0;
	*(int16_t *)buffer = (int16_t)(value * 32767);
}

static void bqPlayerCallback(SLAndroidSimpleBufferQueueItf caller, void *context) {
	if (a2_callback != NULL) {
		a2_callback(&a2_buffer, AUDIO_BUFFER_SIZE);
		for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
			copySample(&tempBuffer[i]);
		}
		SLresult result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, tempBuffer, AUDIO_BUFFER_SIZE * 2);
	}
	else {
		memset(tempBuffer, 0, sizeof(tempBuffer));
		SLresult result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, tempBuffer, AUDIO_BUFFER_SIZE * 2);
	}
}

static bool initialized = false;

void kinc_a2_init() {
	if (initialized) {
		return;
	}

	initialized = true;

	kinc_a2_samples_per_second = 44100;
	a2_buffer.read_location = 0;
	a2_buffer.write_location = 0;
	a2_buffer.data_size = 128 * 1024;
	a2_buffer.data = malloc(a2_buffer.data_size);

	SLresult result;
	result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
	result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
	result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

	const SLInterfaceID ids[] = {SL_IID_VOLUME};
	const SLboolean req[] = {SL_BOOLEAN_FALSE};
	result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
	result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);

	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
	SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM,           2,
	                               SL_SAMPLINGRATE_44_1,        SL_PCMSAMPLEFORMAT_FIXED_16,
	                               SL_PCMSAMPLEFORMAT_FIXED_16, SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
	                               SL_BYTEORDER_LITTLEENDIAN};
	SLDataSource audioSrc = {&loc_bufq, &format_pcm};

	SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
	SLDataSink audioSnk = {&loc_outmix, NULL};

	const SLInterfaceID ids1[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
	const SLboolean req1[] = {SL_BOOLEAN_TRUE};
	result = (*engineEngine)->CreateAudioPlayer(engineEngine, &(bqPlayerObject), &audioSrc, &audioSnk, 1, ids1, req1);
	result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);

	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &(bqPlayerPlay));

	result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &(bqPlayerBufferQueue));

	result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);

	result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

	memset(tempBuffer, 0, sizeof(tempBuffer));
	result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, tempBuffer, AUDIO_BUFFER_SIZE * 2);
}

void pauseAudio() {
	if (bqPlayerPlay == NULL)
		return;
	SLresult result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
}

void resumeAudio() {
	if (bqPlayerPlay == NULL)
		return;
	SLresult result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
}

void kinc_a2_update() {}

void kinc_a2_shutdown() {
	if (bqPlayerObject != NULL) {
		(*bqPlayerObject)->Destroy(bqPlayerObject);
		bqPlayerObject = NULL;
		bqPlayerPlay = NULL;
		bqPlayerBufferQueue = NULL;
	}
	if (outputMixObject != NULL) {
		(*outputMixObject)->Destroy(outputMixObject);
		outputMixObject = NULL;
	}
	if (engineObject != NULL) {
		(*engineObject)->Destroy(engineObject);
		engineObject = NULL;
		engineEngine = NULL;
	}
}

void kinc_a2_set_callback(void (*kinc_a2_audio_callback)(kinc_a2_buffer_t *buffer, int samples)) {
	a2_callback = kinc_a2_audio_callback;
}
