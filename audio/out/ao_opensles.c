#include "ao.h"
#include "internal.h"
#include "common/msg.h"
#include "audio/format.h"
#include "osdep/timer.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

struct priv {
    SLObjectItf sl, output_mix, player;
    SLBufferQueueItf buffer_queue;
    SLEngineItf engine;
    SLPlayItf play;
    char *buffer;
    size_t buffer_size;
};

static const int fmtmap[][2] = {
    { AF_FORMAT_U8, SL_PCMSAMPLEFORMAT_FIXED_8 },
    { AF_FORMAT_S16, SL_PCMSAMPLEFORMAT_FIXED_16 },
    { AF_FORMAT_S32, SL_PCMSAMPLEFORMAT_FIXED_32 },
    { 0 }
};

#define DESTROY(thing) \
    if (p->thing) { \
        (*p->thing)->Destroy(p->thing); \
        p->thing = NULL; \
    }

static void uninit(struct ao *ao)
{
    struct priv *p = ao->priv;

    DESTROY(player);
    DESTROY(output_mix);
    DESTROY(sl);

    p->buffer_queue = NULL;
    p->engine = NULL;
    p->play = NULL;

    free(p->buffer);
    p->buffer = NULL;
    p->buffer_size = 0;
}

#undef DESTROY

static void buffer_callback(SLBufferQueueItf buffer_queue, void *context)
{
    struct ao *ao = context;
    struct priv *p = ao->priv;
    void *data[1] = { p->buffer };
    SLresult res;

    double delay = p->buffer_size / (double)ao->bps;
    ao_read_data(ao, data, p->buffer_size / ao->sstride, mp_time_us() + delay);

    res = (*buffer_queue)->Enqueue(buffer_queue, p->buffer, p->buffer_size);
    if (res != SL_RESULT_SUCCESS)
        MP_ERR(ao, "Failed to Enqueue: %d\n", res);
}

#define BUFFER_SIZE_MS 50

#define CHK(stmt) \
    { \
        SLresult res = stmt; \
        if (res != SL_RESULT_SUCCESS) { \
            MP_ERR(ao, "%s: %d\n", #stmt, res); \
            goto error; \
        } \
    }

static int init(struct ao *ao)
{
    struct priv *p = ao->priv;
    SLDataLocator_BufferQueue locator_buffer_queue;
    SLDataLocator_OutputMix locator_output_mix;
    SLDataFormat_PCM pcm;
    SLDataSource audio_source;
    SLDataSink audio_sink;

    // This AO only supports two channels at the moment
    mp_chmap_from_channels(&ao->channels, 2);

    ao->format = af_fmt_from_planar(ao->format);
    switch (ao->format) {
    case AF_FORMAT_FLOAT:
        ao->format = AF_FORMAT_S16;
        break;
    case AF_FORMAT_DOUBLE:
        ao->format = AF_FORMAT_S32;
        break;
    }

    CHK(slCreateEngine(&p->sl, 0, NULL, 0, NULL, NULL));
    CHK((*p->sl)->Realize(p->sl, SL_BOOLEAN_FALSE));
    CHK((*p->sl)->GetInterface(p->sl, SL_IID_ENGINE, (void*)&p->engine));
    CHK((*p->engine)->CreateOutputMix(p->engine, &p->output_mix, 0, NULL, NULL));
    CHK((*p->output_mix)->Realize(p->output_mix, SL_BOOLEAN_FALSE));

    locator_buffer_queue.locatorType = SL_DATALOCATOR_BUFFERQUEUE;
    locator_buffer_queue.numBuffers = 1;

    pcm.formatType = SL_DATAFORMAT_PCM;
    pcm.numChannels = 2;
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
    p->buffer_size = ao->device_buffer * ao->channels.num *
        af_fmt_to_bytes(ao->format);
    p->buffer = calloc(1, p->buffer_size);

    audio_source.pFormat = (void*)&pcm;
    audio_source.pLocator = (void*)&locator_buffer_queue;

    locator_output_mix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
    locator_output_mix.outputMix = p->output_mix;

    audio_sink.pLocator = (void*)&locator_output_mix;
    audio_sink.pFormat = NULL;

    SLboolean required[] = { SL_BOOLEAN_TRUE };
    SLInterfaceID iid_array[] = { SL_IID_BUFFERQUEUE };
    CHK((*p->engine)->CreateAudioPlayer(p->engine, &p->player, &audio_source,
        &audio_sink, 1, iid_array, required));
    CHK((*p->player)->Realize(p->player, SL_BOOLEAN_FALSE));
    CHK((*p->player)->GetInterface(p->player, SL_IID_PLAY, (void*)&p->play));
    CHK((*p->player)->GetInterface(p->player, SL_IID_BUFFERQUEUE,
        (void*)&p->buffer_queue));
    CHK((*p->buffer_queue)->RegisterCallback(p->buffer_queue,
        buffer_callback, ao));

    return 1;
error:
    uninit(ao);
    return -1;
}

#undef CHK

static void set_play_state(struct ao *ao, SLuint32 state)
{
    struct priv *p = ao->priv;
    SLresult res = (*p->play)->SetPlayState(p->play, state);
    if (res != SL_RESULT_SUCCESS)
        MP_ERR(ao, "Failed to SetPlayState(%d): %d\n", state, res);
}

static void reset(struct ao *ao)
{
    set_play_state(ao, SL_PLAYSTATE_STOPPED);
}

static void resume(struct ao *ao)
{
    struct priv *p = ao->priv;
    set_play_state(ao, SL_PLAYSTATE_PLAYING);

    // The callback is fired once a buffer finishes playing, since after we set
    // the playing state the queue is empty, we need to enqueue something
    // to kick the callback (which lives in a different thread).
    static char empty = 0;
    (*p->buffer_queue)->Enqueue(p->buffer_queue, &empty, 1);
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
