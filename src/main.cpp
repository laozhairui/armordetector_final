#include <video_capture.h>

#include <iostream>
#include <string>

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

    cv::Mat frame;

    while(reader.read_frame(frame))
    {
        if(frame.empty())
        {
            break;
        }
        cv::imshow("Armor Detection",frame);

        int key = cv::waitKey(1);

        if(key == 27 || key == 'q' || key == 'Q')
        {
            break;
        }
    }

    cv::destroyAllWindows();

    return 0;
}