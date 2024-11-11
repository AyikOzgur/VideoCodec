#include <iostream>
#include <opencv2/opencv.hpp>
#include "VCodecX264.h"
#include <fstream>


int main (int argc, char *argv[])
{
    // Open the video file
    cv::VideoCapture cap("../../test/test.mp4");
    if (!cap.isOpened())
    {
        std::cerr << "Error! Unable to open video file\n";
        return -1;
    }

    // Get video properties
    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);

    // Frames 
    cv::Mat inputFrame(height, width, CV_8UC3);
    cv::Mat YUV420Frame(height, width, CV_8UC3);
    cr::video::Frame YU12Frame(width, height, cr::video::Fourcc::YU12);
    cr::video::Frame h264Frame(width, height, cr::video::Fourcc::H264);

    // Create x264 codec
    VCodecX264 codec;

    std::ofstream outputFile("encoded_video.h264", std::ios::binary);
    if (!outputFile.is_open())
    {
        std::cerr << "Failed to open output file for writing!" << std::endl;
        return -1;
    }

    while (true)
    {
        // Read the frame
        cap >> inputFrame;
        if (inputFrame.empty())
        {
            // End of video
            break;
        }

        // Convert the frame to YUV420
        cv::cvtColor(inputFrame, YUV420Frame, cv::COLOR_BGR2YUV_I420);

        // Copy the frame data
        memcpy(YU12Frame.data, YUV420Frame.data, YUV420Frame.total() * YUV420Frame.elemSize());

        // Encode the frame
        codec.transcode(YU12Frame, h264Frame);

        if (outputFile.is_open())
        {
            outputFile.write(reinterpret_cast<char*>(h264Frame.data), h264Frame.size);
        }
        else
        {
            std::cerr << "Failed to open output file for writing!" << std::endl;
        }
    }

    // Release the video file
    cap.release();

    // Close the output file
    outputFile.close();
}