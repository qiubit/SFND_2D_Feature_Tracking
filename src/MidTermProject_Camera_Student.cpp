/* INCLUDES FOR THIS PROJECT */
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <limits>
#include <deque>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

#include "dataStructures.h"
#include "matching2D.hpp"

using namespace std;

string det = "SHITOMASI";
string desc = "BRISK";

void changeDetectorType(string newDetectorType)
{
    det = newDetectorType;
}

void changeDescriptorType(string newDescriptorType)
{
    desc = newDescriptorType;
}

void showDetectorHelp()
{
    std::cout << std::endl;
    std::cout << "Change Detector Type:" << std::endl;
    std::cout << "\t1 - SHITOMASI\n";
    std::cout << "\t2 - HARRIS\n";
    std::cout << "\t3 - AKAZE\n";
    std::cout << "\t4 - BRISK\n";
    std::cout << "\t5 - FAST\n";
    std::cout << "\t6 - ORB\n";
    std::cout << "\t7 - SIFT\n\n";
    std::cout << "Current Detector Type: " << det << std::endl << std::endl;
}

void showDescriptorHelp()
{
    std::cout << std::endl;
    std::cout << "Change Descriptor Type:" << std::endl;
    std::cout << "\tq - BRISK\n";
    std::cout << "\tw - BRIEF\n";
    std::cout << "\te - ORB\n";
    std::cout << "\tr - FREAK\n";
    std::cout << "\tt - AKAZE\n";
    std::cout << "\ty - SIFT\n\n";
    std::cout << "Current Descriptor Type: " << desc << std::endl << std::endl;
}

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

/* MAIN PROGRAM */
int main(int argc, const char *argv[])
{
    /* INIT VARIABLES AND DATA STRUCTURES */

    // data location
    string dataPath = "../";

    // camera
    string imgBasePath = dataPath + "images/";
    string imgPrefix = "KITTI/2011_09_26/image_00/data/000000"; // left camera, color
    string imgFileType = ".png";
    int imgStartIndex = 0; // first file index to load (assumes Lidar and camera names have identical naming convention)
    int imgEndIndex = 9;   // last file index to load
    int imgFillWidth = 4;  // no. of digits which make up the file index (e.g. img-0001.png)

    // misc
    deque<DataFrame> dataBuffer; // list of data frames which are held in memory at the same time
    bool bVis = false;            // visualize results

    /* MAIN LOOP OVER ALL IMAGES */

    bool detChanged = false;
    bool descChanged = false;

    for (int imgIndex = 0; imgIndex <= imgEndIndex - imgStartIndex; imgIndex++)
    {
        /* LOAD IMAGE INTO BUFFER */

        // assemble filenames for current index
        ostringstream imgNumber;
        imgNumber << setfill('0') << setw(imgFillWidth) << imgStartIndex + imgIndex;
        string imgFullFilename = imgBasePath + imgPrefix + imgNumber.str() + imgFileType;

        // load image from file and convert to grayscale
        cv::Mat img, imgGray, imgGrayPrev;
        img = cv::imread(imgFullFilename);
        cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);

        //// STUDENT ASSIGNMENT
        //// TASK MP.1 -> replace the following code with ring buffer of size dataBufferSize

        if (!detChanged && !descChanged) {
            // push image into data frame buffer
            DataFrame frame;
            frame.cameraImg = imgGray;
            dataBuffer.push_back(frame);
        }

        detChanged = false;
        descChanged = false;

        //// EOF STUDENT ASSIGNMENT
        cout << "#1 : LOAD IMAGE INTO BUFFER done" << endl;

        if (dataBuffer.size() > 2) {
            std::cout << "pop" << std::endl;
            dataBuffer.pop_front();
        }

        if (dataBuffer.size() < 2) {
            continue;
        }

        imgGrayPrev = dataBuffer.front().cameraImg;

        /* DETECT IMAGE KEYPOINTS */

        // extract 2D keypoints from current image
        vector<cv::KeyPoint> prevKeypoints;
        vector<cv::KeyPoint> keypoints; // create empty feature list for current image

        string detectorType = det;

        //// STUDENT ASSIGNMENT
        //// TASK MP.2 -> add the following keypoint detectors in file matching2D.cpp and enable string-based selection based on detectorType
        //// -> HARRIS, FAST, BRISK, ORB, AKAZE, SIFT

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

        //// EOF STUDENT ASSIGNMENT

        //// STUDENT ASSIGNMENT
        //// TASK MP.3 -> only keep keypoints on the preceding vehicle

        // only keep keypoints on the preceding vehicle
        bool bFocusOnVehicle = true;
        if (bFocusOnVehicle)
        {
            filterKeypoints(prevKeypoints);
            filterKeypoints(keypoints);
        }

        //// EOF STUDENT ASSIGNMENT

        // optional : limit number of keypoints (helpful for debugging and learning)
        bool bLimitKpts = false;
        if (bLimitKpts)
        {
            int maxKeypoints = 50;

            if (detectorType.compare("SHITOMASI") == 0)
            { // there is no response info, so keep the first 50 as they are sorted in descending quality order
                prevKeypoints.erase(prevKeypoints.begin() + maxKeypoints, prevKeypoints.end());
                keypoints.erase(keypoints.begin() + maxKeypoints, keypoints.end());
            }
            cv::KeyPointsFilter::retainBest(prevKeypoints, maxKeypoints);
            cv::KeyPointsFilter::retainBest(keypoints, maxKeypoints);
            cout << " NOTE: Keypoints have been limited!" << endl;
        }

        // push keypoints and descriptor for current frame to end of data buffer
        dataBuffer.front().keypoints = prevKeypoints;
        dataBuffer.back().keypoints = keypoints;
        cout << "#2 : DETECT KEYPOINTS done" << endl;

        /* EXTRACT KEYPOINT DESCRIPTORS */

        //// STUDENT ASSIGNMENT
        //// TASK MP.4 -> add the following descriptors in file matching2D.cpp and enable string-based selection based on descriptorType
        //// -> BRIEF, ORB, FREAK, AKAZE, SIFT

        cv::Mat descriptors, prevDescriptors;
        string descriptorName = desc;
        descKeypoints(dataBuffer.front().keypoints, dataBuffer.front().cameraImg, prevDescriptors, descriptorName);
        descKeypoints(dataBuffer.back().keypoints, dataBuffer.back().cameraImg, descriptors, descriptorName);
        //// EOF STUDENT ASSIGNMENT

        // push descriptors for current frame to end of data buffer
        dataBuffer.front().descriptors = prevDescriptors;
        dataBuffer.back().descriptors = descriptors;

        cout << "#3 : EXTRACT DESCRIPTORS done" << endl;

        if (dataBuffer.size() > 1) // wait until at least two images have been processed
        {

            /* MATCH KEYPOINT DESCRIPTORS */

            vector<cv::DMatch> matches;
            string matcherType = "MAT_BF";        // MAT_BF, MAT_FLANN
            string descriptorType;
            string selectorType = "SEL_KNN";       // SEL_NN, SEL_KNN

            if (descriptorName.compare("SIFT") == 0) {
                descriptorType = "DES_HOG";
            } else {
                descriptorType = "DES_BINARY";
            }

            //// STUDENT ASSIGNMENT
            //// TASK MP.5 -> add FLANN matching in file matching2D.cpp
            //// TASK MP.6 -> add KNN match selection and perform descriptor distance ratio filtering with t=0.8 in file matching2D.cpp

            matchDescriptors(dataBuffer.front().keypoints, dataBuffer.back().keypoints,
                             dataBuffer.front().descriptors, dataBuffer.back().descriptors,
                             matches, descriptorType, matcherType, selectorType);

            //// EOF STUDENT ASSIGNMENT

            // store matches in current data frame
            dataBuffer.back().kptMatches = matches;

            cout << "#4 : MATCH KEYPOINT DESCRIPTORS done" << endl;

            // visualize matches between current and previous image
            bVis = true;
            if (bVis)
            {
                cv::Mat matchImg = (dataBuffer.back().cameraImg).clone();
                cv::drawMatches(dataBuffer.front().cameraImg, dataBuffer.front().keypoints,
                                dataBuffer.back().cameraImg, dataBuffer.back().keypoints,
                                matches, matchImg,
                                cv::Scalar::all(-1), cv::Scalar::all(-1),
                                vector<char>(), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

                string windowName = "Matching keypoints between two camera images";
                cv::namedWindow(windowName, 7);
                cv::imshow(windowName, matchImg);
                cout << "Press key to continue to next image" << endl;

                showDetectorHelp();
                showDescriptorHelp();

                int key = cv::waitKey(0); // wait for key to be pressed

                int zeroAscii = 48;
                int chosenDetId = key-zeroAscii;

                if (chosenDetId >= 1 && chosenDetId <= 7) {
                    detChanged = true;

                    if (chosenDetId == 1) {
                        det = "SHITOMASI";
                    } else if (chosenDetId == 2) {
                        det = "HARRIS";
                    } else if (chosenDetId == 3) {
                        det = "AKAZE";
                    } else if (chosenDetId == 4) {
                        det = "BRISK";
                    } else if (chosenDetId == 5) {
                        det = "FAST";
                    } else if (chosenDetId == 6) {
                        det = "ORB";
                    } else if (chosenDetId == 7) {
                        det = "SIFT";
                    }
                }

                vector<int> qwertyAsciiCodes{113, 119, 101, 114, 116, 121};
                vector<string> descriptors{
                    "BRISK", "BRIEF", "ORB", "FREAK", "AKAZE", "SIFT"
                };

                for (int i = 0; i < qwertyAsciiCodes.size(); ++i) {
                    if (key == qwertyAsciiCodes[i]) {
                        descChanged = true;
                        desc = descriptors[i];
                    }
                }

                if (detChanged || descChanged) {
                    imgIndex -= 1;
                }
            }
            bVis = false;
        }

    } // eof loop over all images

    return 0;
}
