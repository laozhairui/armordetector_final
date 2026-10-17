#include <video_capture.h>

#include <color_detector.h>

#include <contour_detector.h>

#include <light_bar_detector.h>
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
    LightBarDetector light_bar_detector;
    std::string color = "red";
    cv::Mat frame;

    cv::namedWindow("Contours", cv::WINDOW_NORMAL);
    cv::resizeWindow("Contours", 640, 480);
    cv::moveWindow("Contours", 0, 0);

    cv::namedWindow("Color Mask", cv::WINDOW_NORMAL);
    cv::resizeWindow("Color Mask", 640, 480);
    cv::moveWindow("Color Mask", 650, 0);
    
    while(reader.read_frame(frame))
    {
        if(frame.empty())
        {
            break;
        }

        cv::Mat mask = color_detector.detect(frame, color);

        cv::Mat contours_view = frame.clone();

        std::vector<std::vector<cv::Point>> contours = contour_detector.detect(mask);

        std::vector<LightBar> light_bar = light_bar_detector.detect(contours);

        for (const auto& detected_light_bar : light_bar)
        {
            cv::Point2f points[4];
            detected_light_bar.rect.points(points);

            for(int i = 0; i < 4; ++i)
            {
                cv::line(
                    contours_view,
                    points[i],
                    points[(i + 1) % 4],
                    cv::Scalar(0, 255, 0),
                    2
                );
            }
        }

        cv::imshow("Contours",contours_view);
        cv::imshow("Color Mask",mask);

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