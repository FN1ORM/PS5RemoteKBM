// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#include "../gui/include/streamfps.h"
#include <SDL.h>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/hwcontext.h>
#include <opus/opus.h>
}
#include <cstdio>
#include <initializer_list>
int main(int, char**) {
    StreamFps counter;
    counter.presented(StreamFps::noFrame, 0);
    if (counter.sample(1000000) != -1) return 8;
    // 30 video frames per second on a display that redraws each frame twice.
    for (int i = 0; i <= 45; ++i) {
        int64_t now = 1000000 + i * 1000000LL / 30;
        counter.presented(i, now);
        counter.presented(i, now + 10000);
    }
    if (counter.sample(2500000) != 30 || counter.sample(3500000) != 0) return 9;
    counter.reset();
    if (counter.sample(4500000) != -1) return 10;
    std::puts("PASS: presentation FPS excludes redraws and handles stalls and reconnects");
    for(auto id:{AV_CODEC_ID_H264,AV_CODEC_ID_HEVC}) {
        auto codec=avcodec_find_decoder(id); if(!codec) return 1;
        auto context=avcodec_alloc_context3(codec); if(!context) return 2;
        if(avcodec_open2(context,codec,nullptr)<0) return 3;
        std::printf("PASS: software decoder opened: %s\n",codec->name); avcodec_free_context(&context);
    }
    int error; auto opus=opus_decoder_create(48000,2,&error);
    if(!opus || error!=OPUS_OK) return 4;
    opus_int16 samples[960*2];
    if(opus_decode(opus,nullptr,0,samples,960,0)!=960) return 5;
    opus_decoder_destroy(opus); std::puts("PASS: Opus decoder and loss concealment");
    if(SDL_Init(SDL_INIT_AUDIO)<0) {std::fprintf(stderr,"Audio init failed: %s\n",SDL_GetError());return 6;}
    SDL_AudioSpec wanted{},actual{}; wanted.freq=48000; wanted.channels=2; wanted.format=AUDIO_S16SYS; wanted.samples=512;
    auto device=SDL_OpenAudioDevice(nullptr,0,&wanted,&actual,SDL_AUDIO_ALLOW_ANY_CHANGE);
    if(!device) {std::fprintf(stderr,"Audio open failed: %s\n",SDL_GetError());SDL_Quit();return 7;}
    std::printf("PASS: SDL default audio device opened (%d Hz, %d channels)\n",actual.freq,actual.channels);
    SDL_CloseAudioDevice(device); SDL_Quit();
    std::puts("Hardware decoding and real stream remain owner-assisted tests.");
    return 0;
}
