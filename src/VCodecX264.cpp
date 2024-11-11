#include "VCodecX264.h"
#include "VCodecX264Version.h"
#include <chrono>

VCodecX264::VCodecX264()
{

}

VCodecX264::~VCodecX264()
{

}

std::string VCodecX264::getVersion()
{
    return VCODEC_X264_VERSION;
}

bool VCodecX264::transcode(cr::video::Frame& src, cr::video::Frame& dst)
{
    if (!m_init)
    {
        // Get default parameters
        x264_param_default_preset(&m_param, "ultrafast", "zerolatency");

        // Initialize x264
        m_param.i_width = src.width;
        m_param.i_height = src.height;
        m_param.i_csp = X264_CSP_I420;
        m_param.i_bitdepth = 8;
        m_param.b_vfr_input = 0;
        m_param.b_repeat_headers = 1;
        m_param.b_annexb = 1;

        // Appy profile
        if (x264_param_apply_profile(&m_param, "baseline") < 0)
        {
            std::cout << "x264_param_apply_profile failed" << std::endl;
            return false;
        }

        // Allocate picture
        if (x264_picture_alloc(&m_picIn, m_param.i_csp, m_param.i_width, m_param.i_height) < 0)
        {
            std::cout << "x264_picture_alloc failed" << std::endl;
            return false;
        }

        // Open encoder
        h = x264_encoder_open(&m_param);

        m_init = true;
    }

    // Copy frame data
    memcpy(m_picIn.img.plane[0], src.data, src.size);

    // Staring time
    auto start = std::chrono::high_resolution_clock::now();

    // Encode frame
    x264_nal_t *nal;
    int i_frame_size = x264_encoder_encode(h, &nal, &i_frame, &m_picIn, &m_picOut);
    if (i_frame_size < 0)
    {
        std::cout << "x264_encoder_encode failed" << std::endl;
        return false;
    }

    // Ending time
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate duration
    std::chrono::duration<double> duration = end - start;

    // Print frame size
    std::cout << "Frame size: " << i_frame_size << "  Duration(ms): " << duration.count() * 1000 << std::endl;

    // Copy frame data
    memcpy(dst.data, m_picOut.img.plane[0], i_frame_size);

    return true;
}

bool VCodecX264::setParam(cr::video::VCodecParam id, float value)
{
    return false;
}

float VCodecX264::getParam(cr::video::VCodecParam id)
{
    return -1;
}

bool VCodecX264::executeCommand(cr::video::VCodecCommand cmd)
{
    return false;
}