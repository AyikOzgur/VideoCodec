#include <iostream>
#include <opencv2/opencv.hpp>
#include "VideoCodec.h"


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

    // Frames 
    cv::Mat inputFrame(height, width, CV_8UC3);
    cv::Mat YUV420Frame(height, width, CV_8UC3);
    cv::Mat rgbFrame(height, width, CV_8UC3);
    cr::video::Frame YU12Frame(width, height, cr::video::Fourcc::YU12);
    cr::video::Frame h264Frame(width, height, cr::video::Fourcc::H264);
    cr::video::Frame h265Frame(width, height, cr::video::Fourcc::HEVC);
    cr::video::Frame rgb24Frame(width, height, cr::video::Fourcc::RGB24);
    cr::video::Frame jpegFrame(width, height, cr::video::Fourcc::JPEG);

    // Decoded frames.
    cr::video::Frame h264DecodedFrame(width, height, cr::video::Fourcc::BGR24);
    cr::video::Frame h265DecodedFrame(width, height, cr::video::Fourcc::BGR24);
    cr::video::Frame jpegDecodedFrame(width, height, cr::video::Fourcc::BGR24);

    // Encoders.
    VideoCodec h264Codec;
    VideoCodec h265Codec;
    VideoCodec jpegCodec;

    // Decoders.
    VideoCodec h264Decoder;
    VideoCodec h265Decoder;
    VideoCodec jpegDecoder;

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
        auto h264EncodeTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        h265Codec.encode(YU12Frame, h265Frame);
        end = std::chrono::high_resolution_clock::now();
        auto h265EncodeTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        jpegCodec.encode(rgb24Frame, jpegFrame);
        end = std::chrono::high_resolution_clock::now();
        auto jpegEncodeTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();


        start = std::chrono::high_resolution_clock::now();
        h264Decoder.decode(h264Frame, h264DecodedFrame);
        end = std::chrono::high_resolution_clock::now();
        auto h264DecodeTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        cv::Mat h264DecodedMat(height, width, CV_8UC3);
        memcpy(h264DecodedMat.data, h264DecodedFrame.data, h264DecodedFrame.size);

        start = std::chrono::high_resolution_clock::now();
        h265Decoder.decode(h265Frame, h265DecodedFrame);
        end = std::chrono::high_resolution_clock::now();
        auto h265DecodeTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        cv::Mat h265DecodedMat(height, width, CV_8UC3);
        memcpy(h265DecodedMat.data, h265DecodedFrame.data, h265DecodedFrame.size);

        start = std::chrono::high_resolution_clock::now();
        jpegDecoder.decode(jpegFrame, jpegDecodedFrame);
        end = std::chrono::high_resolution_clock::now();
        auto jpegDecodeTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
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
        int fontFace = cv::FONT_ITALIC;
        double fontScale = 3.5;
        int fontThickness = 3;
        // Calculate the size of the text to center it
        int baseline;
        cv::Size textSize = cv::getTextSize(text1, fontFace, fontScale, fontThickness, &baseline);

        // Put text in the center of each section
        cv::putText(combinedMat, text1, cv::Point((cropWidth - textSize.width) / 2, height - height / 4), fontFace, fontScale, cv::Scalar(255, 0, 0), fontThickness);
        cv::putText(combinedMat, text2, cv::Point(cropWidth + (cropWidth - textSize.width) / 2, height - height / 4), fontFace, fontScale, cv::Scalar(0, 255, 0), fontThickness);
        cv::putText(combinedMat, text3, cv::Point(2 * cropWidth + (cropWidth - textSize.width) / 2, height - height / 4), fontFace, fontScale, cv::Scalar(0, 0, 255), fontThickness);

        // Put encoding and decoding times
        for (int i = 0; i < 3; i++)
        {
            int frameSize;
            std::string encodeTime;
            std::string decodeTime;
            switch (i)
            {
            case 0:
                encodeTime = std::to_string(h264EncodeTime);
                decodeTime = std::to_string(h264DecodeTime);
                frameSize = h264Frame.size / 1000; // in KB
                break;
            case 1:
                encodeTime = std::to_string(h265EncodeTime);
                decodeTime = std::to_string(h265DecodeTime);
                frameSize = h265Frame.size / 1000; // in KB
                break;
            case 2:
                encodeTime = std::to_string(jpegEncodeTime);
                decodeTime = std::to_string(jpegDecodeTime);
                frameSize = jpegFrame.size / 1000; // in KB
                break;
            default:
                break;
            }
            cv::putText(combinedMat, "Encoding time: " + encodeTime + " ms", cv::Point(i * cropWidth + 10, 30), fontFace, 1, cv::Scalar(255, 255, 255), 1);
            cv::putText(combinedMat, "Decoding time: " + decodeTime + " ms", cv::Point(i * cropWidth + 10, 60), fontFace, 1, cv::Scalar(255, 255, 255), 1);

            // Put also encoded frame size
            cv::putText(combinedMat, "Encoded size: " + std::to_string(frameSize) + " KB", cv::Point(i * cropWidth + 10, 90), fontFace, 1, cv::Scalar(255, 255, 255), 1);

        }

        // Display the final frame
        cv::imshow("Preview", combinedMat);

        // Wait for 1ms and check if the user pressed the 'Esc' key
        if (cv::waitKey(0) == 27)
        {
            // Stop the video
            break;
        }
    }

    // Release the video file
    cap.release();

    return 0;
}