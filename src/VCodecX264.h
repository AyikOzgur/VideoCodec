#pragma once
#include <string>
#include <iostream>
#include <stdint.h>
#include "x264.h"
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
    bool m_init;
    /// x264 encoder parameters
    x264_param_t m_param;
    x264_picture_t m_picIn;
    x264_picture_t m_picOut;
    x264_t *h;
    int i_frame = 0;
    int i_frame_size;
    x264_nal_t *nal;
    int i_nal;
};
