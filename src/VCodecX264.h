#pragma once
#include <string>
#include <iostream>
#include <stdint.h>
#include "x264.h"
#include "x265.h"
#include <jpeglib.h>
#include "VCodec.h"



/**
 * @brief Video codec based on x264 library.
 */
class VCodecX264 : public cr::video::VCodec
{
public:

    /**
     * @brief Class constructor.
     */
    VCodecX264();

    /**
     * @brief Class destructor.
     */
    ~VCodecX264();

    /**
     * @brief Get string of current library version.
     * @return String of current library version "Major.Minor.Patch".
     */
    static std::string getVersion();

    /**
     * @brief Encode or decode frame.
     * @param src Source frame. Valid pixel formats:
     *            NV12 to encode frame.
     *            H264, HEVC or JPEG to decode frame.
     * @param dst Result frame. Pixel formats:
     *            NV12 in case decoding.
     *            H264, HEVC or JPEG in case encoding. User must specify
     *            pixel format before encoding.
     * @return TRUE if frame was encoded/decoded or FALSE if not.
     */
    bool transcode(cr::video::Frame& src, cr::video::Frame& dst);

    /**
     * @brief Set codec parameter.
     * @param id Parameter ID.
     * @param value Parameter value.
     * @return TRUE if parameter was set of FALSE.
     */
    bool setParam(cr::video::VCodecParam id, float value);

    /**
     * @brief Get parameter value.
     * @param id Parameter ID.
     * @return Parameter value or -1.
     */
    float getParam(cr::video::VCodecParam id);

    /**
     * @brief Execute command.
     * @param id Command ID.
     * @return TRUE if the command is executed or FALSE if not.
     */
    bool executeCommand(cr::video::VCodecCommand id);

private:

    /// init flag
    bool m_init{false};
    int m_width{-1};
    int m_height{-1};
    int m_bitrate{5000000}; // 5 Mbps
    cr::video::Fourcc m_pixelFormat{cr::video::Fourcc::H264};

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

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    unsigned char* jpeg_buffer = nullptr;
    unsigned long jpeg_size = 0;
    bool initJpegEncoder(int width, int height);
    bool encodeJpegFrame(cr::video::Frame& src, cr::video::Frame& dst);
};
