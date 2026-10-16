#include <video_capture.h>

#include <opencv2/videoio.hpp>

VideoCapture::VideoCapture(const std::string& source)
    :
    m_capture()
{
    if(source == "0" || source  == "camera")
    {
        m_capture.open(0);
    }
    else
    {
        m_capture.open(source);
    }
}

bool VideoCapture::is_opened() const
{
    return m_capture.isOpened();
}

bool VideoCapture::read_frame(cv::Mat& frame)
{
    return m_capture.read(frame);
}