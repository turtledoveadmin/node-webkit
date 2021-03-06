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

#include "media/ffmpeg/ffmpeg_common.h"
#include "media/base/audio_bus.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include "ffmpeg_mediarecorder_wrapper.h"
  
  // forward declaration
  void avpriv_color_frame(AVFrame *frame, const int *c);

  FFMpegMediaRecorder::FFMpegMediaRecorder() {
    memset(&video_st, 0, sizeof(video_st));
    memset(&audio_st, 0, sizeof(audio_st));

    fmt = NULL;
    oc = NULL;
    have_video = 0;
    have_audio = 0;
    audio_only = 0;
    fileReady_ = false;
    lastSrcW_ = lastSrcH_ = 0;
    
    blackPixel_  = NULL;
    blackScaler_ = NULL;
    
    av_register_all();
  }

  PixelFormat VideoFormatToPixelFormat(media::VideoFrame::Format video_format) {
    if (video_format == media::VideoFrame::I420)
      return PIX_FMT_YUV420P;
    else
      return media::VideoFormatToPixelFormat(video_format);
  }

  int FFMpegMediaRecorder::InitVideo(short width, short height, char framerate, int bitrate, media::VideoFrame::Format videoFormat) {
    base::AutoLock lock(lock_);
    AVCodec *video_codec = NULL;

    /* Add the video streams using the default format codecs
    * and initialize the codecs. */
    if (fmt->video_codec != AV_CODEC_ID_NONE) {
      if (add_stream(&video_st, oc, &video_codec, 
        fmt->video_codec, bitrate, -1, -1, width, height, framerate, ::VideoFormatToPixelFormat(videoFormat)) == 0) {
        have_video = 1;
      }
    }

    /* Now that all the parameters are set, we can open the
    * video codecs and allocate the necessary encode buffers. */
    if (have_video)
      have_video = open_video(oc, video_codec, &video_st) == 0;
    
    if (have_video && have_audio)
      InitFile();

    return have_video;
  }

  int FFMpegMediaRecorder::InitAudio(int samplerate, int targetSampleRate, int bitrate, int channels, int frame_size) {
    base::AutoLock lock(lock_);
    AVCodec *audio_codec = NULL;
    const bool useQuality = bitrate >=0 && bitrate <= 10;

    if (fmt->audio_codec != AV_CODEC_ID_NONE) {
      if (add_stream(&audio_st, oc, &audio_codec, 
        fmt->audio_codec, useQuality ? 0 : bitrate, targetSampleRate, channels, -1, -1, -1, -1) == 0) {
        have_audio = 1;
      }
    }
    
    if (useQuality) {
      audio_st.st->codec->flags |= CODEC_FLAG_QSCALE;
      audio_st.st->codec->global_quality = FF_QP2LAMBDA * bitrate;
    }

    if (have_audio)
      have_audio = open_audio(oc, audio_codec, &audio_st, samplerate, channels, frame_size) == 0;

    if (have_audio && (have_video || audio_only))
      InitFile();

    return have_audio;
  }

  int FFMpegMediaRecorder::Init(const char* filename) {
    filename_ = filename;
    /* allocate the output media context */
    avformat_alloc_output_context2(&oc, NULL, NULL, filename_.c_str());
    if (!oc) {
      return -1;
    }

    fmt = oc->oformat;
    audio_only = strcmp(fmt->name, "ogg") == 0;

    return 0;
  }

  int FFMpegMediaRecorder::InitFile() {
    /* open the output file, if needed */
    if (!(fmt->flags & AVFMT_NOFILE)) {
      if (avio_open(&oc->pb, filename_.c_str(), AVIO_FLAG_WRITE) < 0) {
        return -1;
      }
    }

    /* Write the stream header, if any. */
    AVDictionary* opt = NULL;
    if (avformat_write_header(oc, &opt) < 0) {
      return -1;
    }

    fileReady_ = true;
    return 0;
  }

  bool FFMpegMediaRecorder::UpdateVideo(const media::VideoFrame& frame,
    const media::VideoCaptureFormat& format,
    const base::TimeTicks& estimated_capture_time) {

    if (!have_video || !fileReady_) return false;

    const AVCodecContext* c = video_st.st->codec;
    AVFrame* dstFrame = video_st.frame;

    if (videoStart_.is_null()) {
      videoStart_ = estimated_capture_time;
      dstFrame->pts = 0;
    }
    else {
      base::TimeDelta dt = estimated_capture_time - videoStart_;
      dstFrame->pts = dt.InSecondsF() / c->time_base.num * c->time_base.den;
    }

    const int srcW = frame.coded_size().width();
    const int srcH = frame.coded_size().height();
    const PixelFormat srcF = ::VideoFormatToPixelFormat(frame.format());
    
    const size_t srcNumPlanes = frame.NumPlanes(frame.format());
    const uint8* srcSlice[media::VideoFrame::kMaxPlanes];
    int srcStride[media::VideoFrame::kMaxPlanes];
    
    for(size_t i=0; i<srcNumPlanes; i++) {
      srcSlice[i]=frame.data(i);
      srcStride[i]=frame.stride(i);
    }
    for(size_t i=srcNumPlanes; i<media::VideoFrame::kMaxPlanes; i++) {
      srcSlice[i] = NULL;
      srcStride[i] = 0;
    }

    if (c->width != srcW || c->height != srcH ||
      c->pix_fmt != srcF) {

      SwsContext* swsC = video_st.sws_ctx;

      // Source resolution changed ?
      if (swsC && (lastSrcW_ != srcW || lastSrcH_ != srcH)) {
        sws_freeContext(video_st.sws_ctx);
        video_st.sws_ctx = NULL;
      }
      
      float scale = (float)c->height/srcH;
      int dstW = srcW * scale;
      int dstH;
      
      if (dstW <= c->width)
        dstH = scale * srcH;
      else {
        scale = (float) c->width/srcW;
        dstW = scale * srcW;
        dstH = scale * srcH;
      }
      
      assert(dstW <= c->width && dstH <= c->height);

      if (video_st.sws_ctx == NULL) {
        video_st.sws_ctx = sws_getContext(
          srcW, srcH, srcF,
          dstW, dstH, c->pix_fmt,
          SWS_BICUBIC, NULL, NULL, NULL);

        if (!video_st.sws_ctx) 
          return false;

        lastSrcW_ = srcW;
        lastSrcH_ = srcH;
        
        const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get((AVPixelFormat)dstFrame->format);
        if (desc->flags & AV_PIX_FMT_FLAG_PLANAR) {
          const int isRGB = desc->flags & AV_PIX_FMT_FLAG_RGB;
          // YUV black value is 0, 128, 128
          const int color[] = {0, isRGB ? 0 : 128, isRGB ? 0 : 128, 0};
          avpriv_color_frame(dstFrame, color);
        } else {
          // SLOW path, non planar, make black picture, scale it to the target !!
          if (blackPixel_ == NULL) {
            blackPixel_ = alloc_picture(AV_PIX_FMT_GRAY8, 1, 1);
            for (int i=0; i<AV_NUM_DATA_POINTERS; i++)
              memset(blackPixel_->data[i], 0, blackPixel_->linesize[i] * blackPixel_->height);
          }
          
          if (blackScaler_ == NULL) {
            blackScaler_ = sws_getContext(
              blackPixel_->width, blackPixel_->height, (AVPixelFormat)blackPixel_->format,
              c->width, c->height, c->pix_fmt,
              SWS_POINT, NULL, NULL, NULL);
          }

          sws_scale(blackScaler_,
                    blackPixel_->data, blackPixel_->linesize,
                    0, blackPixel_->height, dstFrame->data, dstFrame->linesize);
        }

        // Center the destination image
        const int shiftX = FFMAX(0,(c->width - dstW)/2);
        const int shiftY = FFMAX(0,(c->height - dstH)/2);
        const int* dstStride = dstFrame->linesize;
        int p;
        for (p=0; p<desc->nb_components; p++) {
          bool is_chroma = p == 1 || p == 2;
          int shiftx = is_chroma ? FF_CEIL_RSHIFT(shiftX, desc->log2_chroma_w) : shiftX;
          int shifty = is_chroma ? FF_CEIL_RSHIFT(shiftY, desc->log2_chroma_h) : shiftY;
          dstSlice_[p] = dstFrame->data[p] + shiftx + shifty * dstStride[p];
        }
        for(; p<media::VideoFrame::kMaxPlanes; p++)
          dstSlice_[p] = 0;
      }
      
      sws_scale(video_st.sws_ctx,
        srcSlice, srcStride,
        0, srcH, dstSlice_, dstFrame->linesize);

    }
    else {
      av_image_copy(dstFrame->data, dstFrame->linesize, srcSlice, srcStride,
                    (AVPixelFormat)dstFrame->format, dstFrame->width, dstFrame->height);
    }

    AVPacket pkt = { 0 };
    if (write_video_frame(oc, &video_st, dstFrame, &pkt) == 0) {
      base::AutoLock lock(lock_);
      if (write_frame(oc, &c->time_base, video_st.st, &pkt) != 0)
        return false;
    }
    return true;

  }

  // copy audio data from audiobus to avframe
  static void PutData(AVFrame* frame, int frameIdx, const media::AudioBus& audio_bus, int audioBusIdx, int size){
    const int channels = std::min(AV_NUM_DATA_POINTERS, audio_bus.channels());
    for (int i = 0; i < channels; i++) {
      assert(frameIdx + size <= frame->linesize[0]);
      memcpy(frame->data[i] + frameIdx, &audio_bus.channel(i)[audioBusIdx / sizeof(float)], size);
    }
  }

  bool FFMpegMediaRecorder::UpdateAudio(const media::AudioBus& audio_bus,
    const base::TimeTicks& estimated_capture_time) {
 
    if (!have_audio || !fileReady_) return false;

    if (audioStart_.is_null()) {
      // take note you can't use video capture time for audio, they differ by 30 secs 
      // due to audio delay compensation
      audioStart_ = estimated_capture_time;
      audio_st.frame->pts = 0;
    }

    //base::TimeDelta dt = estimated_capture_time - audioStart_;
    int left_over_data = audio_st.tmp_frame->linesize[0] - 0;
    PutData(audio_st.tmp_frame, 0, audio_bus, 0, left_over_data);

    int res = -1;
    do {
      AVPacket pkt = { 0 }; // data and size must be 0;
      res = write_audio_frame(&audio_st, &pkt, res > 0 ? 0 : audio_st.tmp_frame->nb_samples);
      if (res >= 0) {
        base::AutoLock lock(lock_);
        //int64_t scaledPktDts = av_rescale_q(pkt.dts, audio_st.st->codec->time_base, audio_st.st->time_base);

        // write_frame is most likely the encoding bottlenect, I/O bound
        if (write_frame(oc, &audio_st.st->codec->time_base, audio_st.st, &pkt) != 0) {
          DVLOG(1) << " Error while writing frame\n";
          //audio_st.st->cur_dts = scaledPktDts;
        }
      }
    } while (res > 0);

    return true;

  }

  FFMpegMediaRecorder::~FFMpegMediaRecorder() {
    Stop();
  }

  bool FFMpegMediaRecorder::Stop() {
    if (oc == NULL)
      return false;

    base::AutoLock lock(lock_);
    
    if (blackPixel_)  av_frame_free(&blackPixel_);
    if (blackScaler_) sws_freeContext(blackScaler_);

    /* Write the trailer, if any. The trailer must be written before you
    * close the CodecContexts open when you wrote the header; otherwise
    * av_write_trailer() may try to use memory that was freed on
    * av_codec_close(). */
    if (fileReady_)
      av_write_trailer(oc);

    /* Close each codec. */
    if (have_video)
      close_stream(oc, &video_st);
    if (have_audio)
      close_stream(oc, &audio_st);

    if (fileReady_ && !(fmt->flags & AVFMT_NOFILE))
      /* Close the output file. */
      avio_close(oc->pb);

    /* free the stream */
    avformat_free_context(oc);

    oc = NULL;
    return true;
  }
};
