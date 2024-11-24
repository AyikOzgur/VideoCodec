**VideoCodec C++ library**

**v1.1.0**



# Table of contents

- [Overview](#overview)
- [Versions](#versions)
- [VideoCodec class description](#videocodec-class-description)
  - [Class declaration](#class-declaration)
  - [getVersion method](#getversion-method)
  - [encode method](#encode-method)
  - [decode method](#decode-method)
- [Build and connect to your project](#build-and-connect-to-your-project)
- [Example](#example)



# Overview

**VideoCodec** is C++ wrapper library that provides video encoding and decoding features. **VideoCodec** is based on libraries : [x264](https://www.videolan.org/developers/x264.html), [x265](https://www.videolan.org/developers/x265.html), [jpeg](https://libjpeg.sourceforge.net/) for encoding and [libav](https://trac.ffmpeg.org/wiki/Using%20libav*) for decoding. Also library uses [Frame](https://github.com/ConstantRobotics-Ltd/Frame) library from [ConstantRobotics](https://www.constantrobotics.com/). **VideoCodec** provides h264, h265 and jpeg video compression and decompression. All 3rdparty codecs that **VideoCodec** uses are software codecs. Although software codecs are slower than hardware codecs, their biggest advantage is portability.


# Versions

**Table 1** - Library versions.

| Version | Release date | What's new                                                   |
| ------- | ------------ | ------------------------------------------------------------ |
| 1.0.0   | 16.11.2024   | - First version.                                             |
| 1.1.0   | 24.11.2024   | - Decoding feature is implemented.                           |



# VideoCodec class description



## Class declaration

**VideoCodec** class declared in **VideoCodec.h** file. Class declaration:

```cpp
/**
 * @brief Video codec.
 */
class VideoCodec
{
public:

    /// Class destructor.
    ~VideoCodec();

    /// Class get version.
    static std::string getVersion();

    /// Frame encoding.
    bool encode(cr::video::Frame& src, cr::video::Frame& dst);

    /// Frame decoding.
    bool decode(cr::video::Frame& src, cr::video::Frame& dst);
};
```



## getVersion method

The **getVersion()** method returns string of current version of **VideoCodec**. Method declaration:

```cpp
static std::string getVersion();
```

Method can be used without **VideoCodec** class instance:

```cpp
std::cout << "VideoCodec class version: " << VideoCodec::getVersion();
```

Console output:

```bash
VideoCodec class version: 1.1.0
```



## encode method

The **encode(...)** method encodes the src frame by using codec related with Fourcc of dst frame. Method declaration:

```cpp
bool encode(cr::video::Frame& src, cr::video::Frame& dst);
```

| Parameter | Value                                                        |
| --------- | ------------------------------------------------------------ |
| src       | Source frame. Supported formats: YU12 for h264 and h265 encoding, RGB24 for jpeg encoding. |
| dst       | Destination frame for compressed data. **VideoCodec** class uses dst frames Fourcc to detect codec type.|

**Returns:** TRUE if the frame is encoded successfully.



## decode method

The **decode(...)** method decodes src frame by using codec related with Fourcc of src frame. Method declaration:

```cpp
bool decode(cr::video::Frame& src, cr::video::Frame& dst);
```

| Parameter | Value                                                        |
| --------- | ------------------------------------------------------------ |
| src       | Source frame. Supported formats : H264, H265 and JPEG.         |
| dst       | Destination frame for decompressed data. Fourcc must be RGB24. |

**Returns:** TRUE if the frame is decoded successfully.


# Build and connect to your project

Typical commands to build **VideoCodec** library:

```bash
cd VideoCodec
mkdir build
cd build
cmake ..
make
```

If you want connect **VideoCodec** library to your CMake project as source code you can make follow. For example, if your repository has structure:

```bash
CMakeLists.txt
src
    CMakeList.txt
    yourLib.h
    yourLib.cpp
```

Create folder **3rdparty** in your repository and copy **VideoCodec** repository folder there. New structure of your repository:

```bash
CMakeLists.txt
src
    CMakeList.txt
    yourLib.h
    yourLib.cpp
3rdparty
    VideoCodec
```

Create CMakeLists.txt file in **3rdparty** folder. CMakeLists.txt should contain:

```cmake
cmake_minimum_required(VERSION 3.13)

################################################################################
## 3RD-PARTY
## dependencies for the project
################################################################################
project(3rdparty LANGUAGES CXX)

################################################################################
## SETTINGS
## basic 3rd-party settings before use
################################################################################
# To inherit the top-level architecture when the project is used as a submodule.
SET(PARENT ${PARENT}_YOUR_PROJECT_3RDPARTY)
# Disable self-overwriting of parameters inside included subdirectories.
SET(${PARENT}_SUBMODULE_CACHE_OVERWRITE OFF CACHE BOOL "" FORCE)

################################################################################
## CONFIGURATION
## 3rd-party submodules configuration
################################################################################
SET(${PARENT}_SUBMODULE_VIDEO_CODEC                     ON  CACHE BOOL "" FORCE)
if (${PARENT}_SUBMODULE_VIDEO_CODEC)
    SET(${PARENT}_VIDEO_CODEC                           ON  CACHE BOOL "" FORCE)
    SET(${PARENT}_VIDEO_CODEC_TEST                      OFF CACHE BOOL "" FORCE)
endif()

################################################################################
## INCLUDING SUBDIRECTORIES
## Adding subdirectories according to the 3rd-party configuration
################################################################################
if (${PARENT}_SUBMODULE_VIDEO_CODEC)
    add_subdirectory(VideoCodec)
endif()
```

File **3rdparty/CMakeLists.txt** adds folder **VideoCodec** to your project and excludes test applications from compiling (by default test applications and example excluded from compiling if **VideoCodec** included as sub-repository).The new structure of your repository:

```bash
CMakeLists.txt
src
    CMakeList.txt
    yourLib.h
    yourLib.cpp
3rdparty
    CMakeLists.txt
    VideoCodec
```

Next you need include folder 3rdparty in main **CMakeLists.txt** file of your repository. Add string at the end of your main **CMakeLists.txt**:

```cmake
add_subdirectory(3rdparty)
```

Next you have to include VideoCodec library in your **src/CMakeLists.txt** file:

```cmake
target_link_libraries(${PROJECT_NAME} VideoCodec)
```

Done!

# Example

The example demonstrates how to use **VideoCodec** library. 

```cpp
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
    double fps = cap.get(cv::CAP_PROP_FPS);

    /// Frames. 
    cv::Mat inputFrame(height, width, CV_8UC3);
    cv::Mat YUV420Frame(height, width, CV_8UC3);
    cr::video::Frame YU12Frame(width, height, cr::video::Fourcc::YU12);
    cr::video::Frame h264Frame(width, height, cr::video::Fourcc::H264);

    /// Decoded frame.
    cr::video::Frame h264DecodedFrame(width, height, cr::video::Fourcc::BGR24);

    /// Codecs.
    VideoCodec h264Encoder;
    VideoCodec h264Decoder;

    while (true)
    {
        // Read the frame
        cap >> inputFrame;
        if (inputFrame.empty())
        {
            // End of video
            break;
        }

        // Convert the frame to YUV420 (YU12)
        cv::cvtColor(inputFrame, YUV420Frame, cv::COLOR_BGR2YUV_I420);

        // Copy the frame data
        memcpy(YU12Frame.data, YUV420Frame.data, YUV420Frame.total() * YUV420Frame.elemSize());

        h264Encoder.encode(YU12Frame, h264Frame);
        h264Decoder.decode(h264Frame, h264DecodedFrame);

        cv::Mat h264DecodedMat(height, width, CV_8UC3);
        memcpy(h264DecodedMat.data, h264DecodedFrame.data, h264DecodedFrame.size);

        // Display the decoded frame.
        cv::imshow("Preview", h264DecodedMat);

        // Wait for 1ms and check if the user pressed the 'Esc' key
        if (cv::waitKey(1) == 27)
        {
            // Stop the video
            break;
        }
    }

    return 0;
}
```