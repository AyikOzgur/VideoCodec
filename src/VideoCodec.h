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
    virtual ~VideoCodec();

    /**
     * @brief Class constructor
     */
    VideoCodec() = default;

    /**
     * @brief Video codec is not copyable.
     */
    VideoCodec(VideoCodec&) = delete;
    void operator=(VideoCodec&) = delete;

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

    /// Encoder initialization flags.
    bool m_encoderInit{false};
    /// Decoder initialization flags.
    bool m_decoderInit{false};
    /// Video frame width.
    int m_width{-1};
    /// Video frame height.
    int m_height{-1};
    /// Encdoer bitrate in bps.
    int m_bitrate{5000000}; // 5 Mbps
    /// Pixel format.
    cr::video::Fourcc m_pixelFormat{cr::video::Fourcc::YUYV};

    /// x264 encoder parameters.
    x264_param_t m_h264Param;
    /// x264 input picture.
    x264_picture_t m_h264PicIn;
    /// x264 output picture.
    x264_picture_t m_h264PicOut;
    /// x264 encoder.
    x264_t *m_h264Encoder{nullptr};

    /**
     * @brief Initialize x264 encoder.
     * @param width Frame width.
     * @param height Frame height.
     * @return TRUE if the encoder was initialized successfully or FALSE.
     */
    bool initH264Encoder(int width, int height);

    /**
     * @brief Encode a frame using x264 encoder.
     * @param src Source frame.
     * @param dst Destination frame.
     * @return TRUE if the frame was encoded successfully or FALSE.
     */
    bool encodeH264Frame(cr::video::Frame& src, cr::video::Frame& dst);

    /// x265 encoder parameters.
    x265_param m_h265Param;
    /// x265 input picture.
    x265_picture *m_h265PicIn;
    /// x265 output picture.
    x265_picture m_h265PicOut;
    /// x265 encoder.
    x265_encoder *m_h265Encoder{nullptr};
    /// Internal buffer for YUV420 frame. H.265 encoder requires proving buffer.
    uint8_t *m_h265InternalBuffer{nullptr};

    /**
     * @brief Initialize x265 encoder.
     * @param width Frame width.
     * @param height Frame height.
     * @return TRUE if the encoder was initialized successfully or FALSE.
     */
    bool initH265Encoder(int width, int height);

    /**
     * @brief Encode a frame using x265 encoder.
     * @param src Source frame.
     * @param dst Destination frame.
     * @return TRUE if the frame was encoded successfully or FALSE.
     */
    bool encodeH265Frame(cr::video::Frame& src, cr::video::Frame& dst);

    /// Jpeg encoder parameters.
    struct jpeg_compress_struct cinfo;
    /// Error handler
    struct jpeg_error_mgr jerr;
    /// Jpeg buffer.
    unsigned char* jpeg_buffer = nullptr;
    /// Jpeg buffer size.
    unsigned long jpeg_size = 0;

    /**
     * @brief Initialize JPEG encoder.
     * @param width Frame width.
     * @param height Frame height.
     * @return TRUE if the encoder was initialized successfully or FALSE.
     */
    bool initJpegEncoder(int width, int height);

    /**
     * @brief Encode a frame using JPEG encoder.
     * @param src Source frame.
     * @param dst Destination frame.
     * @return TRUE if the frame was encoded successfully or FALSE.
     */
    bool encodeJpegFrame(cr::video::Frame& src, cr::video::Frame& dst);

    /// Libav software decoder.
    AVCodec *m_decoder;
    /// Libav codec context.
    AVCodecContext *codec_ctx;
    /// Libav packet to store encoded frame.
    AVPacket *packet;
    /// Libav frame to store decoded frame.
    AVFrame *frame;
    /// Libav software scaler context.
    struct SwsContext* sws_ctx;

    /**
     * @brief Initialize decoder.
     * @param src Source frame.
     * @return TRUE if the decoder was initialized successfully or FALSE.
     */
    bool initDecoder(cr::video::Frame frame);

    /**
     * @brief Decode a frame using software decoder.
     * @param src Source frame.
     * @param dst Destination frame.
     * @return TRUE if the frame was decoded successfully or FALSE.
     */
    bool decodeFrame(cr::video::Frame& src, cr::video::Frame& dst);
};