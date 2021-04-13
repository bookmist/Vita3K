#include <codec/state.h>

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <cassert>

uint32_t DecoderState::get(DecoderQuery query) {
    return 0;
}

uint32_t DecoderState::receive(uint8_t *data, DecoderSize *size) {
    return false;
}

void DecoderState::configure(void *options) {
    // do nothing
}

uint32_t DecoderState::get_es_size(const uint8_t *data) {
    return std::numeric_limits<uint32_t>::max();
}

void DecoderState::flush() {
    avcodec_flush_buffers(context);
}

DecoderState::~DecoderState() {
    avcodec_close(context);
    avcodec_free_context(&context);
}
