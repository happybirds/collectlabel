#include "opencv_detector.h"
#include <iostream>
#include <android/log.h>

std::vector<cv::Point2f> cnt;
cv::Mat roiGray1;
cv::Mat outputthresh;
cv::Mat paddedRoi;

using namespace std;
using namespace cv;

OpenCVDetector::OpenCVDetector(const std::string &imagePath)
{
    img = cv::imread(imagePath);
}

OpenCVDetector::~OpenCVDetector()
{
    // Clean up resources if needed
}

bool OpenCVDetector::compareContours(const std::vector<cv::Point2f> &contour1, const std::vector<cv::Point2f> &contour2)
{
    Rect rect1 = boundingRect(contour1);
    Rect rect2 = boundingRect(contour2);
    Point2f center1 = rect1.tl() + 0.5 * (rect1.br() - rect1.tl());
    Point2f center2 = rect2.tl() + 0.5 * (rect2.br() - rect2.tl());

    return (center1.y == center2.y) ? (center1.x < center2.x) : (center1.y < center2.y);
}

std::vector<cv::Mat> OpenCVDetector::drawRotatedRectangles()
{

    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    cv::threshold(gray, thresh, 120, 255, 1);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
    cv::dilate(thresh, dilated, kernel);

    cv::findContours(dilated, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    cv::cvtColor(dilated, dilated, cv::COLOR_GRAY2BGR);

    for (int i = 0; i < contours.size(); ++i)
    {

        // Skip outer contours
        approxPolyDP(contours[i], cnt, 0.05 * arcLength(contours[i], true), true);

        cv::Rect boundingRect = cv::boundingRect(contours[i]);

        int x = boundingRect.x;
        int y = boundingRect.y;
        int w = boundingRect.width;
        int h = boundingRect.height;

        if (abs(w - h) < 15 || abs(w - h) > 35 || contourArea(cnt) > 7000 || w < 20 || h < 15 || contourArea(cnt) < 1000)
        {
            continue;
        }

        if (hierarchy[i][3] != -1)
        {
            parentIndices.push_back(i);
            drawContours(dilated, contours, i, Scalar(0, 255, 0), 2);

            // counter[hierarchy[i][3]]++;

            cand.push_back(cnt);
        }
    }

    sort(cand.begin(), cand.end(), [this](const vector<Point2f> &a, const vector<Point2f> &b)
         { return compareContours(a, b); });

    for (const auto &contour : cand)
    {
        rotatedRectangles.push_back(minAreaRect(contour));
    }

    __android_log_print(ANDROID_LOG_ERROR, "", "rotatedRectangles size = %d", rotatedRectangles.size());
    // Draw rotated rectangles on the original image and mark with index numbers
    for (int i = 0; i < rotatedRectangles.size(); ++i)
    {
        // Get the four vertices of the rotated rectangle
        Point2f vertices[4];
        rotatedRectangles[i].points(vertices);

        // Draw the rotated rectangle on the original image
        for (int j = 0; j < 4; ++j)
        {
            line(img, vertices[j], vertices[(j + 1) % 4], Scalar(0, 0, 255), 2);
        }

        // Get the angle of the third side (index 2) relative to the horizontal direction
        Point2f vector1 = vertices[1] - vertices[0];
        Point2f vector2(0, 0); // Horizontal direction

        // Calculate the angle using the arctangent function
        float angle = atan2(vector1.y, vector1.x) - atan2(vector2.y, vector2.x);
        float angleDegrees = angle * 180.0 / CV_PI;


        if (angleDegrees < -50 && -90 <= angleDegrees)
        {
            angleDegrees = 90 + angleDegrees;
        }
        else if (0 > angleDegrees && angleDegrees > -50)
        {
        }
        else if (angleDegrees < -90)
        {
            angleDegrees = 90 - angleDegrees;
        }

        // Get bounding box for the rotated rectangle
        Rect boundingBox = rotatedRectangles[i].boundingRect();

        // Rotate and align the rectangle

        Mat M = getRotationMatrix2D(rotatedRectangles[i].center, angleDegrees, 1.0); // Rotate by 100 degrees
        warpAffine(img, rotated, M, img.size(), INTER_CUBIC);

        int borderSize = 7;
        Rect roiRect(boundingBox.x + borderSize, boundingBox.y + borderSize, boundingBox.width - 2 * borderSize, boundingBox.height - 2 * borderSize);
        roiRect &= Rect(0, 0, rotated.cols, rotated.rows); // Ensure the ROI is within the image boundaries
        roi = rotated(roiRect).clone();

        cv::cvtColor(roi, imgGray, cv::COLOR_BGR2GRAY);

        cv::threshold(imgGray, imgBinary, 80, 255, 1);

        cv::findContours(imgBinary, contoursnew, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        for (const auto &contour : contoursnew)
        {
            cv::Rect boundingRect = cv::boundingRect(contour);

            int x = boundingRect.x;
            int y = boundingRect.y;
            int w = boundingRect.width;
            int h = boundingRect.height;

            imgDigit = imgBinary(cv::Rect(x, y, w, h));

            int r = std::max(w, h);
            int y_pad = ((w - h) / 2 * (w > h ? 1 : 0)) + r / 5;
            int x_pad = ((h - w) / 2 * (h > w ? 1 : 0)) + r / 5;
            cv::copyMakeBorder(imgDigit, imgDigit, y_pad, y_pad, x_pad, x_pad, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));

            cv::resize(imgDigit, imgDigit, cv::Size(28, 28), 0, 0, cv::INTER_AREA);
            if (roiRect.area() > 0)
            {
                roiArray.push_back(imgDigit.clone()); // Store the ROI in the array
            }
        }

        putText(img, to_string(i + 1), Point(boundingBox.x + boundingBox.width / 2, boundingBox.y - 10),
                FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 0, 0), 2);
    }
    __android_log_print(ANDROID_LOG_ERROR, "", "roiArray size = %d", roiArray.size());
    roiArray.push_back(img);

    return roiArray; // Return the array of ROIs
}
