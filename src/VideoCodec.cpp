#include "VideoCodec.h"
#include "VideoCodecVersion.h"
#include <chrono>

VideoCodec::~VideoCodec()
{
    if (m_init)
    {
        switch (m_pixelFormat)
        {
        case cr::video::Fourcc::H264:
            x264_picture_clean(&m_h264PicIn);
            x264_encoder_close(m_h264Encoder);
            m_h264Encoder = nullptr;
            break;
        case cr::video::Fourcc::HEVC:
            x265_picture_free(m_h265PicIn);
            m_h265PicIn = nullptr;
            x265_encoder_close(m_h265Encoder);
            m_h265Encoder = nullptr;
            delete[] m_h265InternalBuffer;
            m_h265InternalBuffer = nullptr;
            break;
        case cr::video::Fourcc::JPEG:
            delete[] jpeg_buffer;
            jpeg_buffer = nullptr;
            jpeg_destroy_compress(&cinfo);
            break;
        default:
            break;
        }
    }
}

std::string VideoCodec::getVersion()
{
    return VIDEO_CODEC_VERSION;
}

bool VideoCodec::encode(cr::video::Frame &src, cr::video::Frame &dst)
{
    // Check of input frame is valid and destination frame has correct pixel format.
    if (dst.fourcc != cr::video::Fourcc::H264 && dst.fourcc != cr::video::Fourcc::HEVC && dst.fourcc != cr::video::Fourcc::JPEG)
    {
        std::cout << "Invalid pixel format" << std::endl;
        return false;
    }

    // Check if input frame is valid
    switch (dst.fourcc)
    {
    case cr::video::Fourcc::H264:
    case cr::video::Fourcc::HEVC:
        if (src.fourcc != cr::video::Fourcc::YU12)
        {
            std::cout << "Invalid pixel format" << std::endl;
            return false;
        }
        break;
    case cr::video::Fourcc::JPEG:
        if (src.fourcc != cr::video::Fourcc::RGB24)
        {
            std::cout << "Invalid pixel format" << std::endl;
            return false;
        }
        break;
    default:
        return false;
    }

    // Check if destination frame has enough memory
    if (dst.width != src.width || dst.height != src.height)
    {
        dst.release();
        switch (dst.fourcc)
        {
        case cr::video::Fourcc::H264:
            dst = cr::video::Frame(src.width, src.height, cr::video::Fourcc::H264);
            break;
        case cr::video::Fourcc::HEVC:
            dst = cr::video::Frame(src.width, src.height, cr::video::Fourcc::HEVC);
            break;
        case cr::video::Fourcc::JPEG:
            dst = cr::video::Frame(src.width, src.height, cr::video::Fourcc::JPEG);
            break;
        default:
            return false;
        }
    }

    if (!m_init || m_width != src.width || m_height != src.height || m_pixelFormat != dst.fourcc)
    {
        switch (dst.fourcc)
        {
        case cr::video::Fourcc::H264:
            if (!initH264Encoder(src.width, src.height))
            {
                return false;
            }
            break;
        case cr::video::Fourcc::HEVC:
            if (!initH265Encoder(src.width, src.height))
            {
                return false;
            }
            break;
        case cr::video::Fourcc::JPEG:
            if (!initJpegEncoder(src.width, src.height))
            {
                return false;
            }
            break;
        default:
            return false;
        }

        m_width = src.width;
        m_height = src.height;
        m_pixelFormat = dst.fourcc;
        m_init = true;
    }

    // Encode frame
    switch (dst.fourcc)
    {
    case cr::video::Fourcc::H264:
        if (!encodeH264Frame(src, dst))
        {
            return false;
        }
        break;
    case cr::video::Fourcc::HEVC:
        if (!encodeH265Frame(src, dst))
        {
            return false;
        }
        break;
    case cr::video::Fourcc::JPEG:
        if (!encodeJpegFrame(src, dst))
        {
            return false;
        }
        break;
    default:
        return false;
    }

    return true;
}

bool VideoCodec::initH264Encoder(int width, int height)
{
    // Check if it is already initialized and release resources
    if (m_h264Encoder != nullptr)
    {
        x264_picture_clean(&m_h264PicIn);
        x264_encoder_close(m_h264Encoder);
    }

    // Get default parameters
    x264_param_default_preset(&m_h264Param, "ultrafast", "zerolatency");

    // Initialize x264
    m_h264Param.i_width = width;
    m_h264Param.i_height = height;
    m_h264Param.i_csp = X264_CSP_I420;
    m_h264Param.i_bitdepth = 8;
    m_h264Param.b_vfr_input = 0;
    m_h264Param.b_repeat_headers = 1;
    m_h264Param.b_annexb = 1;
    // Set bitrate
    m_h264Param.rc.i_bitrate = m_bitrate;
    m_h264Param.rc.i_rc_method = X264_RC_CRF;
    // Set GOP size
    m_h264Param.i_keyint_max = 30;
    // Set frame rate
    m_h264Param.i_fps_num = 30;
    // Set number of threads
    m_h264Param.i_threads = 1;

    // Apply profile
    if (x264_param_apply_profile(&m_h264Param, "baseline") < 0)
    {
        std::cout << "x264_param_apply_profile failed" << std::endl;
        return false;
    }

    // Allocate picture
    if (x264_picture_alloc(&m_h264PicIn, m_h264Param.i_csp, m_h264Param.i_width, m_h264Param.i_height) < 0)
    {
        std::cout << "x264_picture_alloc failed" << std::endl;
        return false;
    }

    // Open encoder
    m_h264Encoder = x264_encoder_open(&m_h264Param);

    return true;
}

bool VideoCodec::encodeH264Frame(cr::video::Frame &src, cr::video::Frame &dst)
{
    // Copy Y plane
    memcpy(m_h264PicIn.img.plane[0], src.data, src.width * src.height);
    // Copy U plane
    memcpy(m_h264PicIn.img.plane[1], src.data + src.width * src.height, src.width * src.height / 4);
    // Copy V plane
    memcpy(m_h264PicIn.img.plane[2], src.data + src.width * src.height * 5 / 4, src.width * src.height / 4);

    int i_frame = 0; // Number of NAL units
    // Encode frame
    x264_nal_t *nal;
    int i_frame_size = x264_encoder_encode(m_h264Encoder, &nal, &i_frame, &m_h264PicIn, &m_h264PicOut);
    if (i_frame_size < 0)
    {
        std::cout << "x264_encoder_encode failed" << std::endl;
        return false;
    }

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

bool VideoCodec::initH265Encoder(int width, int height)
{
    // Check if it is already initialized and release resources
    if (m_h265Encoder != nullptr)
    {
        x265_picture_free(m_h265PicIn);
        x265_encoder_close(m_h265Encoder);
        delete[] m_h265InternalBuffer;
        m_h265InternalBuffer = nullptr;
        m_h265PicIn = nullptr;
    }

    // Get default parameters
    x265_param_default_preset(&m_h265Param, "ultrafast", "zerolatency");

    // Initialize x265
    m_h265Param.sourceWidth = width;
    m_h265Param.sourceHeight = height;
    m_h265Param.frameNumThreads = 1;
    m_h265Param.sourceBitDepth = 8;
    m_h265Param.bRepeatHeaders = 1;
    m_h265Param.bAnnexB = 1;
    m_h265Param.internalCsp = X265_CSP_I420;
    // Set bitrate
    m_h265Param.rc.bitrate = m_bitrate;
    m_h265Param.rc.rateControlMode = X265_RC_CRF;
    // Set GOP size
    m_h265Param.keyframeMax = 30;
    // Set frame rate
    m_h265Param.fpsNum = 30;
    m_h265Param.fpsDenom = 1;
    m_h265Param.frameNumThreads = 1;

    // Apply profile
    if (x265_param_apply_profile(&m_h265Param, "main") < 0)
    {
        std::cout << "x265_param_apply_profile failed" << std::endl;
        return false;
    }

    // Allocate picture
    m_h265PicIn = x265_picture_alloc();
    x265_picture_init(&m_h265Param, m_h265PicIn);

    // Open encoder
    m_h265Encoder = x265_encoder_open(&m_h265Param);

    m_h265InternalBuffer = new uint8_t[width * height * 3];

    return true;
}

bool VideoCodec::encodeH265Frame(cr::video::Frame &src, cr::video::Frame &dst)
{
    // Copy YUV420 frame to the x265 picture
    memcpy(m_h265InternalBuffer, src.data, src.size);

    // Set only Pointer to the Y plane
    m_h265PicIn->planes[0] = m_h265InternalBuffer;
    m_h265PicIn->planes[1] = m_h265InternalBuffer + src.width * src.height;
    m_h265PicIn->planes[2] = m_h265InternalBuffer + src.width * src.height * 5 / 4;

    m_h265PicIn->stride[0] = src.width;     // Y stride
    m_h265PicIn->stride[1] = src.width / 2; // U stride
    m_h265PicIn->stride[2] = src.width / 2; // V stride

    // Encode frame
    x265_nal *nal;
    uint32_t i_nal;
    if (x265_encoder_encode(m_h265Encoder, &nal, &i_nal, m_h265PicIn, &m_h265PicOut) < 0)
    {
        std::cout << "x265_encoder_encode failed" << std::endl;
        return false;
    }

    // Copy NAL data to destination frame
    int offset = 0;
    for (uint32_t i = 0; i < i_nal; ++i)
    {
        memcpy(dst.data + offset, nal[i].payload, nal[i].sizeBytes);
        offset += nal[i].sizeBytes;
    }
    dst.size = offset; // Set correct size for the encoded frame

    return true;
}

bool VideoCodec::initJpegEncoder(int width, int height)
{
    // Check if it is already initialized and release resources
    if (jpeg_buffer != nullptr)
    {
        delete[] jpeg_buffer;
        jpeg_buffer = nullptr;
        jpeg_destroy_compress(&cinfo);
    }

    // Allocate memory for the JPEG buffer
    jpeg_buffer = new unsigned char[width * height * 3];

    // Initialize JPEG compressor
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 75, TRUE);

    return true;
}

bool VideoCodec::encodeJpegFrame(cr::video::Frame &src, cr::video::Frame &dst)
{
    // Set destination buffer
    jpeg_mem_dest(&cinfo, &jpeg_buffer, &jpeg_size);

    // Start compression
    jpeg_start_compress(&cinfo, TRUE);

    // Write scanlines
    JSAMPROW row_pointer[1];
    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &src.data[cinfo.next_scanline * src.width * 3];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    // Finish compression
    jpeg_finish_compress(&cinfo);

    // Copy JPEG data to destination frame
    memcpy(dst.data, jpeg_buffer, jpeg_size);
    dst.size = jpeg_size;

    return true;
}