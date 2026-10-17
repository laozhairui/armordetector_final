#include <video_capture.h>

#include <color_detector.h>

#include <contour_detector.h>

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
    std::string source = "data/test.mp4";

    if (argc > 1)
    {
        source = argv[1];
    }

    VideoCapture reader(source);

    if(!reader.is_opened())
    {
        std::cerr << "Failed to open video source:" << source << std::endl;
        return -1;
    }

    ColorDetector color_detector;
    ContourDetector contour_detector;
    std::string color = "red";
    cv::Mat frame;

    while(reader.read_frame(frame))
    {
        if(frame.empty())
        {
            break;
        }

        cv::Mat mask = color_detector.detect(frame, color);

        cv::Mat contours_view = frame.clone();

        std::vector<std::vector<cv::Point>> contours = contour_detector.detect(mask);
        
        cv::drawContours(
            contours_view,
            contours,
            -1,
            cv::Scalar(0, 255, 0),
            2
        );

        cv::imshow("Contours",contours_view);
        cv::imshow("Color Mask",mask);
        cv::imshow("Original",frame);

        int key = cv::waitKey(1);

        if(key == 27 || key == 'q' || key == 'Q')
        {
            break;
        }
        if (key == 'b' || key == 'B')
        {
            color = "blue";
        }
        if(key == 'r' || key == 'R')
        {
            color = "red";
        }
    }

    cv::destroyAllWindows();

    return 0;
}