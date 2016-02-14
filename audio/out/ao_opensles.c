#include "ao.h"
#include "internal.h"
#include "common/msg.h"
#include "audio/format.h"
#include "osdep/timer.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

struct priv {
    SLObjectItf sl;
    SLPlayItf playItf;
    SLBufferQueueItf bufferQueueItf;
    char *buffer;
    size_t buffer_size;
};

static const int fmtmap[][2] = {
    { AF_FORMAT_U8, SL_PCMSAMPLEFORMAT_FIXED_8 },
    { AF_FORMAT_S16, SL_PCMSAMPLEFORMAT_FIXED_16 },
    { AF_FORMAT_S32, SL_PCMSAMPLEFORMAT_FIXED_32 },
    { 0 }
};

static void uninit(struct ao *ao)
{
    struct priv *priv = ao->priv;

    if (priv->sl)
        (*priv->sl)->Destroy(priv->sl);

    free(priv->buffer);
    priv->buffer = NULL;
    priv->buffer_size = 0;
}

static void BufferQueueCallback(SLBufferQueueItf queueItf, void *context)
{
    int read;
    struct ao *ao = context;
    struct priv *priv = (struct priv*)ao->priv;
    SLresult res;
    void *data[1] = { priv->buffer };
    double delay = priv->buffer_size / (double)ao->bps;
    read = ao_read_data(ao, data, priv->buffer_size / ao->sstride, mp_time_us() + delay);
    res = (*queueItf)->Enqueue(queueItf, priv->buffer, ao->sstride * read);
    if (res != SL_RESULT_SUCCESS)
        MP_ERR(ao, "Failed to Enqueue: %d\n", res);
}

static void resume(struct ao *ao)
{
    SLresult res;
    struct priv *priv = (struct priv*)ao->priv;
    SLPlayItf playItf = priv->playItf;
    res = (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    if (res != SL_RESULT_SUCCESS)
        MP_ERR(ao, "Failed to SetPlayState(SL_PLAYSTATE_PLAYING): %d\n", res);

    BufferQueueCallback(priv->bufferQueueItf, ao);
}

#define MAX_INTERFACES 5
#define BUFFER_SIZE_MS 50

static int init(struct ao *ao)
{
    struct priv *priv = ao->priv;
    SLObjectItf sl, outputmix, player;
    SLPlayItf playItf;
    SLEngineItf engine;
    SLboolean required[MAX_INTERFACES];
    SLInterfaceID iidArray[MAX_INTERFACES];
    SLDataLocator_BufferQueue bufferQueue;
    SLDataFormat_PCM pcm;
    SLDataSource audioSource;
    SLDataLocator_OutputMix locator_outputmix;
    SLDataSink audioSink;
    SLBufferQueueItf bufferQueueItf;
    SLresult res;
    int i;

    ao->format = af_fmt_from_planar(ao->format);
    switch (ao->format) {
    case AF_FORMAT_FLOAT:
        ao->format = AF_FORMAT_S16;
        break;
    case AF_FORMAT_DOUBLE:
        ao->format = AF_FORMAT_S32;
        break;
    }

    res = slCreateEngine(&sl, 0, NULL, 0, NULL, NULL);
    if (res != SL_RESULT_SUCCESS) {
        MP_ERR(ao, "slCreateEngine failed: %d\n", res);
        goto error;
    }

    priv->sl = sl;

    res = (*sl)->Realize(sl, SL_BOOLEAN_FALSE);
    if (res != SL_RESULT_SUCCESS) {
        MP_ERR(ao, "Failed to Realize the engine: %d\n", res);
        goto error;
    }

    res = (*sl)->GetInterface(sl, SL_IID_ENGINE, (void*)&engine);
    if (res != SL_RESULT_SUCCESS) {
        MP_ERR(ao, "Failed to get SL_IID_ENGINE interface: %d\n", res);
        goto error;
    }

    for (i = 0; i < MAX_INTERFACES; ++i) {
        required[i] = SL_BOOLEAN_FALSE;
        iidArray[i] = SL_IID_NULL;
    }

    res = (*engine)->CreateOutputMix(engine, &outputmix, 0, iidArray, required);
    if (res != SL_RESULT_SUCCESS) {
        MP_ERR(ao, "Failed to CreateOutputMix: %d\n", res);
        goto error;
    }

    res = (*outputmix)->Realize(outputmix, SL_BOOLEAN_FALSE);
    if (res != SL_RESULT_SUCCESS) {
        MP_ERR(ao, "Failed to Realize the Output Mix object: %d\n", res);
        goto error;
    }

    bufferQueue.locatorType = SL_DATALOCATOR_BUFFERQUEUE;
    bufferQueue.numBuffers = 1;

    pcm.formatType = SL_DATAFORMAT_PCM;
    pcm.numChannels = ao->channels.num;
    pcm.samplesPerSec = ao->samplerate * 1000;
    for (int n = 0; fmtmap[n][0]; ++n) {
        if (ao->format == fmtmap[n][0]) {
            pcm.bitsPerSample = fmtmap[n][1];
            break;
        }
    }
    pcm.containerSize = 8 * af_fmt_to_bytes(ao->format);
    pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

    ao->device_buffer = ao->samplerate * BUFFER_SIZE_MS / 1000;
    priv->buffer_size = ao->device_buffer * ao->channels.num * af_fmt_to_bytes(ao->format);
    priv->buffer = calloc(1, priv->buffer_size);

    audioSource.pFormat = (void*)&pcm;
    audioSource.pLocator = (void*)&bufferQueue;

    locator_outputmix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
    locator_outputmix.outputMix = outputmix;
    audioSink.pLocator = (void*)&locator_outputmix;
    audioSink.pFormat = NULL;

    required[0] = SL_BOOLEAN_TRUE;
    iidArray[0] = SL_IID_BUFFERQUEUE;
    res = (*engine)->CreateAudioPlayer(engine, &player, &audioSource, &audioSink, 1, iidArray, required);
    if (res != SL_RESULT_SUCCESS) {
        MP_ERR(ao, "Failed to CreateAudioPlayer: %d\n", res);
        goto error;
    }

    res = (*player)->Realize(player, SL_BOOLEAN_FALSE);
    if (res != SL_RESULT_SUCCESS) {
        MP_ERR(ao, "Failed to Realize the player: %d\n", res);
        goto error;
    }

    res = (*player)->GetInterface(player, SL_IID_PLAY, (void*)&playItf);
    if (res != SL_RESULT_SUCCESS) {
        MP_ERR(ao, "Failed to GetInterface(player, SL_IID_PLAY): %d\n", res);
        goto error;
    }
    priv->playItf = playItf;

    res = (*player)->GetInterface(player, SL_IID_BUFFERQUEUE, (void*)&bufferQueueItf);
    if (res != SL_RESULT_SUCCESS) {
        MP_ERR(ao, "Failed to GetInterface(player, SL_IID_BUFFERQUEUE: %d\n", res);
        goto error;
    }
    priv->bufferQueueItf = bufferQueueItf;

    res = (*bufferQueueItf)->RegisterCallback(bufferQueueItf, BufferQueueCallback, ao);
    if (res != SL_RESULT_SUCCESS) {
        MP_ERR(ao, "Failed to register buffer queue callback: %d\n", res);
        goto error;
    }

    return 1;
error:
    uninit(ao);
    return -1;
}

static void reset(struct ao *ao)
{
    SLresult res;
    struct priv *priv = (struct priv*)ao->priv;
    SLPlayItf playItf = priv->playItf;
    res = (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PAUSED);
    if (res != SL_RESULT_SUCCESS)
        MP_ERR(ao, "Failed to SetPlayState(SL_PLAYSTATE_PAUSED): %d\n", res);
}

const struct ao_driver audio_out_opensles = {
    .description = "OpenSL ES audio output",
    .name      = "opensles",
    .init      = init,
    .uninit    = uninit,
    .reset     = reset,
    .resume    = resume,

    .priv_size = sizeof(struct priv),
};
