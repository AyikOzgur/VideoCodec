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

        // Set GOP size
        m_param.i_keyint_max = 30;
        // Set frame rate
        m_param.i_fps_num = 30;

        // Set number of threads
        m_param.i_threads = 1;

        // Apply profile
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
        m_encoder = x264_encoder_open(&m_param);

        m_init = true;
    }

    // Copy Y plane
    memcpy(m_picIn.img.plane[0], src.data, src.width * src.height);
    // Copy U plane
    memcpy(m_picIn.img.plane[1], src.data + src.width * src.height, src.width * src.height / 4);
    // Copy V plane
    memcpy(m_picIn.img.plane[2], src.data + src.width * src.height * 5 / 4, src.width * src.height / 4);

    // Staring time
    auto start = std::chrono::high_resolution_clock::now();

    int i_frame = 0; // Number of NAL units
    // Encode frame
    x264_nal_t *nal;
    int i_frame_size = x264_encoder_encode(m_encoder, &nal, &i_frame, &m_picIn, &m_picOut);
    if (i_frame_size < 0)
    {
        std::cout << "x264_encoder_encode failed" << std::endl;
        return false;
    }

    // Ending time
    auto end = std::chrono::high_resolution_clock::now();

    // Calculate duration
    std::chrono::duration<double> duration = end - start;

    // Copy NAL data to destination frame
    int offset = 0;
    for (int i = 0; i < i_frame; ++i)
    {
        memcpy(dst.data + offset, nal[i].p_payload, nal[i].i_payload);
        offset += nal[i].i_payload;
    }
    dst.size = offset; // Set correct size for the encoded frame

    return true;
}

bool VCodecX264::setParam(cr::video::VCodecParam id, float value)
{
    return false;
}

float VCodecX264::getParam(cr::video::VCodecParam id)
{
    return -1.0f;
}

bool VCodecX264::executeCommand(cr::video::VCodecCommand cmd)
{
    return false;
}