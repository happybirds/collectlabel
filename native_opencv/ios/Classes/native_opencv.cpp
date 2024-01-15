#include <vector>
#include <opencv2/opencv.hpp>
#include "opencv_detector.h"
#include <android/log.h>

extern "C" __attribute__((visibility("default"))) __attribute__((used)) void processImage(const char *imagePath, ReceiveImageArrayCallback callback)
{
  std::string imagePathStr(imagePath);

  std::vector<std::vector<uchar>> pngDataArray;

  OpenCVDetector detector(imagePath);

  std::vector<cv::Mat> roiArray = detector.drawRotatedRectangles();

  for (const auto &mat : roiArray)
  {
    std::vector<uchar> pngData;
    cv::imencode(".png", mat, pngData);
    callback(pngData.data(), pngData.size());
  }
}
