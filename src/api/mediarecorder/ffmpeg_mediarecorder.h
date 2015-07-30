// Copyright (c) 2015 Jefry Tedjokusumo
// Copyright (c) 2015 The Chromium Authors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell co
// pies of the Software, and to permit persons to whom the Software is furnished
//  to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in al
// l copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IM
// PLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNES
// S FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
//  OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WH
// ETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
//  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

  struct AVStream;
  struct AVFrame;
  struct AVFormatContext;
  struct AVOutputFormat;
  struct AVCodec;
  struct AVPacket;
  struct AVRational;

  // a wrapper around a single output AVStream
  typedef struct OutputStream {
    AVStream *st;

    AVFrame *frame;
    AVFrame *tmp_frame;

    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
  } OutputStream;


  /* Add an output stream. */
  int add_stream(OutputStream *ost, AVFormatContext *oc,
    AVCodec **codec, int AVCodecID, int bitrate,
    int samplerate, int channels,// audio
    short width, short height, char framerate, int AVPixelFormat  //video
    );

  int open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, const int samplerate, const int channels);
  int open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost);
  void close_stream(AVFormatContext *oc, OutputStream *ost);

  AVFrame *alloc_picture(int AVPixelFormat, int width, int height);

  /*
  * encode one video frame and send it to the muxer
  * return 1 when encoding is finished, 0 otherwise
  */
  int write_video_frame(AVFormatContext *oc, OutputStream *ost, AVFrame *frame, AVPacket* pkt);
  /*
  * encode one audio frame and send it to the muxer
  * return 1 when encoding is finished, 0 otherwise
  */
  int write_audio_frame(OutputStream *ost, AVFrame* frame, AVPacket* pkt);

  int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt);

#ifdef __cplusplus
};
#endif
