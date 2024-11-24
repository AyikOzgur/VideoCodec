#pragma once
#include <string>
#include <iostream>
#include <stdint.h>
#include <x264.h>
#include <x265.h>
#include <jpeglib.h>

extern "C" 
{
    #include <libavcodec/avcodec.h>
    #include <libavutil/frame.h>
    #include <libavutil/imgutils.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavfilter/avfilter.h>
}

#include "Frame.h"



/**
 * @brief Video codec.
 */
class VideoCodec
{
public:

    /**
     * @brief Class destructor.
     */
    ~VideoCodec();

    /**
     * @brief Get string of current class version.
     * @return String of current class version "Major.Minor.Patch"
     */
    static std::string getVersion();

    /**
     * @brief Encodes a video frame.
     * @param src Source frame.
     * @param dst Destination frame.
     * @return TRUE if the frame was encoded successfully or FALSE.
     */
    bool encode(cr::video::Frame& src, cr::video::Frame& dst);

    /**
     * @brief Decodes a video frame.
     * @param src Source frame.
     * @param dst Destination frame.
     * @return TRUE if the frame was decoded successfully or FALSE.
     */
    bool decode(cr::video::Frame& src, cr::video::Frame& dst);

private:

    /// init flags.
    bool m_encoderInit{false};
    bool m_decoderInit{false};
    /// Video parameters.
    int m_width{-1};
    int m_height{-1};
    int m_bitrate{5000000}; // 5 Mbps
    cr::video::Fourcc m_pixelFormat{cr::video::Fourcc::YUYV};

    /// x264 encoder parameters
    x264_param_t m_h264Param;
    x264_picture_t m_h264PicIn;
    x264_picture_t m_h264PicOut;
    x264_t *m_h264Encoder{nullptr};
    bool initH264Encoder(int width, int height);
    bool encodeH264Frame(cr::video::Frame& src, cr::video::Frame& dst);

    /// x265 encoder parameters
    x265_param m_h265Param;
    x265_picture *m_h265PicIn;
    x265_picture m_h265PicOut;
    x265_encoder *m_h265Encoder{nullptr};
    uint8_t *m_h265InternalBuffer{nullptr};
    bool initH265Encoder(int width, int height);
    bool encodeH265Frame(cr::video::Frame& src, cr::video::Frame& dst);

    /// jpeg encoder parameters
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned char* jpeg_buffer = nullptr;
    unsigned long jpeg_size = 0;
    bool initJpegEncoder(int width, int height);
    bool encodeJpegFrame(cr::video::Frame& src, cr::video::Frame& dst);

    /// H.264 decoder parameters
    AVCodec *m_decoder;
    AVCodecContext *codec_ctx;
    AVPacket *packet;
    AVFrame *frame;
    struct SwsContext* sws_ctx;
    bool initDecoder(cr::video::Frame frame);
    bool decodeFrame(cr::video::Frame& src, cr::video::Frame& dst);
};