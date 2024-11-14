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
    cv::Mat rgbFrame(height, width, CV_8UC3);
    cr::video::Frame YU12Frame(width, height, cr::video::Fourcc::YU12);
    cr::video::Frame h264Frame(width, height, cr::video::Fourcc::H264);
    cr::video::Frame h265Frame(width, height, cr::video::Fourcc::HEVC);
    cr::video::Frame rgb24Frame(width, height, cr::video::Fourcc::RGB24);
    cr::video::Frame jpegFrame(width, height, cr::video::Fourcc::JPEG);

    // Create codecs
    VideoCodec h264Codec;
    VideoCodec h265Codec;
    VideoCodec jpegCodec;

    std::ofstream outputFileH264("encoded_video.h264", std::ios::binary);
    if (!outputFileH264.is_open())
    {
        std::cerr << "Failed to open output file of h264 for writing!" << std::endl;
        return -1;
    }

    std::ofstream outputFileH265("encoded_video.h265", std::ios::binary);
    if (!outputFileH265.is_open())
    {
        std::cerr << "Failed to open output file of h265 for writing!" << std::endl;
        return -1;
    }

    std::ofstream outputFileJpeg("encoded_video.mjpeg", std::ios::binary);
    if (!outputFileJpeg.is_open())
    {
        std::cerr << "Failed to open output file of jpeg for writing!" << std::endl;
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
        cv::cvtColor(inputFrame, rgbFrame, cv::COLOR_BGR2RGB);

        // Copy the frame data
        memcpy(YU12Frame.data, YUV420Frame.data, YUV420Frame.total() * YUV420Frame.elemSize());
        memcpy(rgb24Frame.data, rgbFrame.data, rgbFrame.total() * rgbFrame.elemSize());

        auto start = std::chrono::high_resolution_clock::now();
        h264Codec.encode(YU12Frame, h264Frame);
        auto end = std::chrono::high_resolution_clock::now();

        std::cout << "H264 encoding time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms ( " << h264Frame.size << " bytes ) | ";

        start = std::chrono::high_resolution_clock::now();
        h265Codec.encode(YU12Frame, h265Frame);
        end = std::chrono::high_resolution_clock::now();

        std::cout << "H265 encoding time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms  ( " << h265Frame.size << " bytes ) | ";

        start = std::chrono::high_resolution_clock::now();
        jpegCodec.encode(rgb24Frame, jpegFrame);
        end = std::chrono::high_resolution_clock::now();

        std::cout << "JPEG encoding time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms  ( " << jpegFrame.size << " bytes )" << std::endl;

        outputFileH264.write(reinterpret_cast<char*>(h264Frame.data), h264Frame.size);
        outputFileH265.write(reinterpret_cast<char*>(h265Frame.data), h265Frame.size);
        outputFileJpeg.write(reinterpret_cast<char*>(jpegFrame.data), jpegFrame.size);
    }

    // Release the video file
    cap.release();

    // Close the output file
    outputFileH264.close();
    outputFileH265.close();
    outputFileJpeg.close();
}