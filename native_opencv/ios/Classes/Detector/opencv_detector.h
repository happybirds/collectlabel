#include <opencv2/opencv.hpp>
#include <vector>

typedef void (*ReceiveImageArrayCallback)(const unsigned char* data, int length);

class OpenCVDetector
{
public:
    OpenCVDetector(const std::string &imagePath);
    ~OpenCVDetector();

    std::vector<cv::Mat> drawRotatedRectangles(); // Updated method declaration

private:

    cv::Mat img;
    cv::Mat gray;
    cv::Mat edges;
    cv::Mat thresh;
    cv::Mat dilated;
    cv::Mat rotated;
    cv::Mat roi;
    cv::Mat warped;
    cv::Mat imgGray;
    cv::Mat imgBinary;
    cv::Mat imgDigit;

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point2f>> cand;
    std::vector<int> parentIndices;
   // std::unordered_map<int, int> counter;
    std::vector<cv::RotatedRect> rotatedRectangles;
    std::vector<cv::Mat> roiArray;
    std::vector<std::vector<cv::Point>> contoursnew;

    bool compareContours(const std::vector<cv::Point2f> &contour1, const std::vector<cv::Point2f> &contour2);
};
