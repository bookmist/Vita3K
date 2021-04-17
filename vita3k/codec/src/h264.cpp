#include <codec/state.h>

#include <util/log.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <cassert>

void copy_yuv_data_from_frame(AVFrame *frame, uint8_t *dest) {
    for (int32_t a = 0; a < frame->height; a++) {
        memcpy(dest, &frame->data[0][frame->linesize[0] * a], frame->width);
        dest += frame->width;
    }
    for (int32_t a = 0; a < frame->height / 2; a++) {
        memcpy(dest, &frame->data[1][frame->linesize[1] * a], frame->width / 2);
        dest += frame->width / 2;
    }
    for (int32_t a = 0; a < frame->height / 2; a++) {
        memcpy(dest, &frame->data[2][frame->linesize[2] * a], frame->width / 2);
        dest += frame->width / 2;
    }
}

uint32_t H264DecoderState::buffer_size(DecoderSize size) {
    return size.width * size.height * 3 / 2;
}

uint32_t H264DecoderState::get(DecoderQuery query) {
    switch (query) {
    case DecoderQuery::WIDTH: return context->width;
    case DecoderQuery::HEIGHT: return context->height;
    //case DecoderQuery::BUFFER_SIZE: return buffer_size(DecoderSize({ context->width, context->height }));
    default: return 0;
    }
}

bool H264DecoderState::send(const uint8_t *data, uint32_t size) {
    int error = 0;

    std::vector<uint8_t> au_frame(size + AV_INPUT_BUFFER_PADDING_SIZE);
    memcpy(au_frame.data(), data, size);

    AVPacket *packet = av_packet_alloc();
    error = av_parser_parse2(
        parser, // AVCodecParserContext *s,
        context, // AVCodecContext *avctx,
        &packet->data, // uint8_t **poutbuf,
        &packet->size, // int *poutbuf_size,
        au_frame.data(), // const uint8_t *buf,
        size, // int buf_size,
        pts == ~0ull ? AV_NOPTS_VALUE : pts, // int64_t pts,
        dts == ~0ull ? AV_NOPTS_VALUE : dts, // int64_t dts,
        0 // int64_t pos
    );
    if (error < 0) {
        LOG_WARN("Error parsing H264 packet: {}.", log_hex(static_cast<uint32_t>(error)));
        av_packet_free(&packet);
        return false;
    }

    error = avcodec_send_packet(context, packet);
    av_packet_free(&packet);
    if (error < 0) {
        LOG_WARN("Error sending H264 packet: {}.", log_hex(static_cast<uint32_t>(error)));
        return false;
    }

    return true;
}

bool H264DecoderState::receive(uint8_t *data, DecoderSize *size) {
    AVFrame *frame = av_frame_alloc();

    int error = avcodec_receive_frame(context, frame);
    if (error < 0) {
        LOG_WARN("Error receiving H264 frame: {}.", log_hex(static_cast<uint32_t>(error)));
        av_frame_free(&frame);
        return false;
    }

    if (data) {
        copy_yuv_data_from_frame(frame, data);
        /*
        auto s = _msize(data);
        auto r = buffer_size(DecoderSize({ (uint16_t)frame->width, (uint16_t)frame->height }));
        uint8_t *tmpData = data; //malloc(r);
        if (s < r){
            LOG_WARN("Wrong mem_size passed {}, got {}", s, r);
            tmpData = (uint8_t*)malloc(r);
        }
        copy_yuv_data_from_frame(frame, tmpData);
        if (tmpData != data) {
            memcpy(data, tmpData, s);
        }
        */
    }

    if (size) {
        *size = { static_cast<uint32_t>(context->width), static_cast<uint32_t>(context->height) };
    }

    av_frame_free(&frame);
    return true;
}

void H264DecoderState::configure(void *options) {
    auto *opt = reinterpret_cast<H264DecoderOptions *>(options);

    pts = static_cast<uint64_t>(opt->pts_upper) << 32u | static_cast<uint64_t>(opt->pts_lower);
    dts = static_cast<uint64_t>(opt->dts_upper) << 32u | static_cast<uint64_t>(opt->dts_lower);
}

H264DecoderState::H264DecoderState(uint32_t width, uint32_t height) {
    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    assert(codec);

    parser = av_parser_init(codec->id);
    assert(parser);
    parser->flags |= PARSER_FLAG_COMPLETE_FRAMES;

    context = avcodec_alloc_context3(codec);
    assert(context);
    context->width = width;
    context->height = height;

    int result = avcodec_open2(context, codec, nullptr);
    assert(result == 0);
}

H264DecoderState::~H264DecoderState() {
    av_parser_close(parser);
}
