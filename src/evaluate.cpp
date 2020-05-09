#include <vector>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

#include "dataStructures.h"
#include "matching2D.hpp"

using namespace std;


vector<string> DETECTORS{
    "SHITOMASI", "HARRIS", "AKAZE", "BRISK", "FAST", "ORB", "SIFT"
};

vector<string> DESCRIPTORS{
    "BRISK", "BRIEF", "ORB", "FREAK", "AKAZE", "SIFT"
};

struct EvaluationResults
{
    string detectorName;
    string descriptorName;
    double keypointsNum;
    double meanKeypointsNum;
    double matchedKeypointsNum;
    double meanMatchedKeypointNum;
    double detectionTime;
    double descriptionTime;
    double totalTime;
};

void filterKeypoints(vector<cv::KeyPoint> &keypoints)
{
    cv::Rect vehicleRect(535, 180, 180, 150);
    vector<cv::KeyPoint> filteredKeypoints;
    for (cv::KeyPoint &k : keypoints) {
        if (k.pt.x >= vehicleRect.x
            && k.pt.x <= vehicleRect.x + vehicleRect.width
            && k.pt.y >= vehicleRect.y
            && k.pt.y <= vehicleRect.y + vehicleRect.height) {
                filteredKeypoints.push_back(k);
            }
    }
    keypoints = filteredKeypoints;
}

void runEvaluation(string detectorType, string descriptorType, vector<EvaluationResults> &results)
{
    // data location
    string dataPath = "../";

    // camera
    string imgBasePath = dataPath + "images/";
    string imgPrefix = "KITTI/2011_09_26/image_00/data/000000"; // left camera, color
    string imgFileType = ".png";
    int imgStartIndex = 0; // first file index to load (assumes Lidar and camera names have identical naming convention)
    int imgEndIndex = 9;   // last file index to load
    int imgFillWidth = 4;  // no. of digits which make up the file index (e.g. img-0001.png)
    double matchingsTotal = imgEndIndex - imgStartIndex;

    EvaluationResults res;
    res.detectorName = detectorType;
    res.descriptorName = descriptorType;
    res.keypointsNum = 0;
    res.matchedKeypointsNum = 0;
    res.detectionTime = 0;
    res.descriptionTime = 0;
    res.totalTime = 0;

    for (int imgIndex = 1; imgIndex <= imgEndIndex - imgStartIndex; imgIndex++)
    {
        // assemble filenames for prev and current index
        ostringstream imgNumberPrev;
        imgNumberPrev << setfill('0') << setw(imgFillWidth) << imgStartIndex + imgIndex-1;
        string imgPrevFullFilename = imgBasePath + imgPrefix + imgNumberPrev.str() + imgFileType;

        ostringstream imgNumber;
        imgNumber << setfill('0') << setw(imgFillWidth) << imgStartIndex + imgIndex;
        string imgFullFilename = imgBasePath + imgPrefix + imgNumber.str() + imgFileType;

        cv::Mat img, imgGray, imgGrayPrev;

        img = cv::imread(imgPrevFullFilename);
        cv::cvtColor(img, imgGrayPrev, cv::COLOR_BGR2GRAY);

        img = cv::imread(imgFullFilename);
        cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);

        vector<cv::KeyPoint> prevKeypoints;
        vector<cv::KeyPoint> keypoints;

        double t = (double)cv::getTickCount();

        if (detectorType.compare("SHITOMASI") == 0)
        {
            detKeypointsShiTomasi(prevKeypoints, imgGrayPrev, false);
            detKeypointsShiTomasi(keypoints, imgGray, false);
        }
        else if (detectorType.compare("HARRIS") == 0)
        {
            detKeypointsHarris(prevKeypoints, imgGrayPrev, false);
            detKeypointsHarris(keypoints, imgGray, false);
        }
        else
        {
            detKeypointsModern(prevKeypoints, imgGrayPrev, detectorType, false);
            detKeypointsModern(keypoints, imgGray, detectorType, false);
        }

        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        res.detectionTime += 1000 * t / 1.0;

        filterKeypoints(prevKeypoints);
        filterKeypoints(keypoints);

        t = (double)cv::getTickCount();

        cv::Mat descriptors, prevDescriptors;
        descKeypoints(prevKeypoints, imgGrayPrev, prevDescriptors, descriptorType);
        descKeypoints(keypoints, imgGray, descriptors, descriptorType);

        t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();
        res.descriptionTime += 1000 * t / 1.0;

        string descriptorMetric;
        if (descriptorType.compare("SIFT") == 0) {
            descriptorMetric = "DES_HOG";
        } else {
            descriptorMetric = "DES_BINARY";
        }

        string matcherType = "MAT_BF";
        string selectorType = "SEL_KNN";

        vector<cv::DMatch> matches;
        matchDescriptors(prevKeypoints, keypoints,
                         prevDescriptors, descriptors,
                         matches, descriptorMetric, matcherType, selectorType);

        if (imgIndex == 1)
            res.keypointsNum += prevKeypoints.size();
        res.keypointsNum += keypoints.size();
        res.matchedKeypointsNum += matches.size();
    }

    res.meanKeypointsNum = res.keypointsNum / (matchingsTotal+1);
    res.meanMatchedKeypointNum = res.matchedKeypointsNum / matchingsTotal;
    res.detectionTime /= 2.0 * matchingsTotal; // we divide by 2, since every loop tick, we compute keypoints for prev picture as well
    res.descriptionTime /= 2.0 * matchingsTotal; // ditto
    res.totalTime = res.detectionTime + res.descriptionTime;

    results.push_back(res);
}


int main()
{
    vector<EvaluationResults> results;

    const string detector = DETECTORS[6];
    for (const string &descriptor : DESCRIPTORS) {
        std::cout << detector << "/" << descriptor << std::endl;
        try {
            runEvaluation(detector, descriptor, results);
        }
        catch (...) {
            std::cout << "FAIL\n";
        }
    }

    std::cout << std::endl << std::endl;

    for (EvaluationResults &res : results) {
        std::cout << res.detectorName << "/" << res.descriptorName << std::endl;
        std::cout << "Keypoints num: " << res.keypointsNum << std::endl;
        std::cout << "Mean keypoints num: " << res.meanKeypointsNum << std::endl;
        std::cout << "Matched keypoints num: " << res.matchedKeypointsNum << std::endl;
        std::cout << "Mean matched keypoints num: " << res.meanMatchedKeypointNum << std::endl;
        std::cout << "Detection time: " << res.detectionTime << "ms" << std::endl;
        std::cout << "Description time: " << res.descriptionTime << "ms" << std::endl;
        std::cout << "Total time: " << res.totalTime << "ms" << std::endl;
        std::cout << std::endl;
    }

    return 0;
}