//
// Created by rolando on 5/15/17.
//

#include <memory>
#include <cstdint>
#include "H264Decoder.h"
#include <assert.h>
#include <cstring>
#include "Constants.h"
#include "../util/logging/Logger.h"

#include <libavutil/error.h>

H264Decoder::H264Decoder() :
    av_packet { av_packet_alloc() }
    {
    av_log_set_callback(log_av);

    assert(av_packet != NULL);

    const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    assert(codec != NULL);

    context = avcodec_alloc_context3(codec);
    assert(context != NULL);

    assert(avcodec_open2(context, codec, NULL) == 0);

    frame = av_frame_alloc();
    out_frame = av_frame_alloc();
    assert(frame != NULL);
    assert(out_frame != NULL);

    // Dimensions
    sws_context = sws_getContext(WII_VIDEO_WIDTH, WII_VIDEO_HEIGHT, AV_PIX_FMT_YUV420P,
                                 WII_VIDEO_WIDTH, WII_VIDEO_HEIGHT, AV_PIX_FMT_RGB24,
                                 SWS_FAST_BILINEAR, NULL, NULL, NULL);
    int bytes_req = 0;
    bytes_req = av_image_get_buffer_size(AV_PIX_FMT_RGB24, WII_VIDEO_WIDTH, WII_VIDEO_HEIGHT, 1);
    out_buffer = new uint8_t[bytes_req];
    assert(av_image_fill_arrays(out_frame->data, out_frame->linesize, out_buffer, AV_PIX_FMT_RGB24, WII_VIDEO_WIDTH,
                         WII_VIDEO_HEIGHT, 1) == bytes_req);
}

int H264Decoder::image(uint8_t *nals, int nals_size, uint8_t *image) {
    av_packet->data = nals;
    av_packet->size = nals_size;

    int sent_packet = avcodec_send_packet(context, av_packet);

    if (sent_packet) {
        char* err = new char[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(sent_packet, err, sizeof(err));
        Logger::error("h264", "Failed to send packet to context: %s", err);
        delete [] err;
        return 0;
    }

    int got_frame = avcodec_receive_frame(context, frame);

    if (got_frame) {
        char* err = new char[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(got_frame, err, sizeof(err));
        Logger::error("h264", "Failed to receive packet from context: %s", err);
        delete [] err;
        return 0;
    }
    else {
        sws_scale(sws_context, (const uint8_t *const *) frame->data, frame->linesize, 0, WII_VIDEO_HEIGHT,
                  out_frame->data, out_frame->linesize);
    }
    int image_size = out_frame->linesize[0] * WII_VIDEO_HEIGHT;
    memcpy(image, out_frame->data[0], (size_t) image_size);
    return image_size;
}

void H264Decoder::log_av(void *avcl, int level, const char *fmt, va_list vl) {
    (void) avcl;
    //TODO: We should really convert libav's log level into our own
    (void) level;
    Logger::log("h264", Logger::VERBOSE, fmt, vl);
}
