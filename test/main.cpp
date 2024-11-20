#include <iostream>
#include <opencv2/opencv.hpp>
#include "VideoCodec.h"
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

    // Decoded frames
    cr::video::Frame h264DecodedFrame(width, height, cr::video::Fourcc::BGR24);
    cr::video::Frame h265DecodedFrame(width, height, cr::video::Fourcc::BGR24);
    cr::video::Frame jpegDecodedFrame(width, height, cr::video::Fourcc::BGR24);

    // Create codecs
    VideoCodec h264Codec;
    VideoCodec h265Codec;
    VideoCodec jpegCodec;

    // Decoder
    VideoCodec h264Decoder;
    VideoCodec h265Decoder;
    VideoCodec jpegDecoder;

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

        // Decode the h264 frame
        if (!h264Decoder.decode(h264Frame, h264DecodedFrame))
        {
            std::cerr << "Failed to decode h264 frame" << std::endl;
            return -1;
        }
        cv::Mat h264DecodedMat(height, width, CV_8UC3);
        memcpy(h264DecodedMat.data, h264DecodedFrame.data, h264DecodedFrame.size);

        // Decode the h265 frame
        if (!h265Decoder.decode(h265Frame, h265DecodedFrame))
        {
            std::cerr << "Failed to decode h265 frame" << std::endl;
            return -1;
        }
        cv::Mat h265DecodedMat(height, width, CV_8UC3);
        memcpy(h265DecodedMat.data, h265DecodedFrame.data, h265DecodedFrame.size);

        // Decode the jpeg frame
        if (!jpegDecoder.decode(jpegFrame, jpegDecodedFrame))
        {
            std::cerr << "Failed to decode jpeg frame" << std::endl;
            return -1;
        }
        cv::Mat jpegDecodedMat(height, width, CV_8UC3);
        memcpy(jpegDecodedMat.data, jpegDecodedFrame.data, jpegDecodedFrame.size);
        
        /// Final Mat for display.
        // Define the crop width (1/3 of the width of the images)
        int cropWidth = width / 3;

        // Crop each image to the relevant horizontal section
        cv::Mat leftSection = h264DecodedMat(cv::Rect(0, 0, cropWidth, height));             // Left third
        cv::Mat centerSection = h265DecodedMat(cv::Rect(cropWidth, 0, cropWidth, height));    // Middle third
        cv::Mat rightSection = jpegDecodedMat(cv::Rect(2 * cropWidth, 0, cropWidth, height)); // Right third

        // Create a final Mat to hold the combined image (width = 3 * crop_width)
        cv::Mat combinedMat(height, width, CV_8UC3);

        // Place each cropped image at its corresponding horizontal position
        leftSection.copyTo(combinedMat(cv::Rect(0, 0, cropWidth, height)));
        centerSection.copyTo(combinedMat(cv::Rect(cropWidth, 0, cropWidth, height)));
        rightSection.copyTo(combinedMat(cv::Rect(2 * cropWidth, 0, cropWidth, height)));
        
        // Draw lines on the borders of each section
        cv::line(combinedMat, cv::Point(cropWidth, 0), cv::Point(cropWidth, height), (0, 0, 255), 1);
        cv::line(combinedMat, cv::Point(2 * cropWidth, 0), cv::Point(2 * cropWidth, height), (0, 0, 255), 1);

        // Text for each section
        std::string text1 = "h264";
        std::string text2 = "h265";
        std::string text3 = "jpeg";

        // Add text to the center of each section
        int font_face = cv::FONT_ITALIC;
        double fontScale = 3.5;
        int fontThickness = 3;
        // Calculate the size of the text to center it
        int baseline;
        cv::Size text_size = cv::getTextSize(text1, font_face, fontScale, fontThickness, &baseline);

        // Put text in the center of each section
        cv::putText(combinedMat, text1, cv::Point((cropWidth - text_size.width) / 2, height - height / 4), font_face, fontScale, cv::Scalar(255, 0, 0), fontThickness);
        cv::putText(combinedMat, text2, cv::Point(cropWidth + (cropWidth - text_size.width) / 2, height - height / 4), font_face, fontScale, cv::Scalar(0, 255, 0), fontThickness);
        cv::putText(combinedMat, text3, cv::Point(2 * cropWidth + (cropWidth - text_size.width) / 2, height - height / 4), font_face, fontScale, cv::Scalar(0, 0, 255), fontThickness);

        // Display the final frame
        cv::imshow("Preview", combinedMat);

        // Wait for 1ms and check if the user pressed the 'Esc' key
        if (cv::waitKey(1) == 27)
        {
            // Stop the video
            break;
        }
    }

    // Release the video file
    cap.release();

    // Close the output file
    outputFileH264.close();
    outputFileH265.close();
    outputFileJpeg.close();
}