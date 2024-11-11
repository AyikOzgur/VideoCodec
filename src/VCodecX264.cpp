#include "VCodecX264.h"
#include "VCodecX264Version.h"


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
    return false;
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