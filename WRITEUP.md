# SFND_2D_Feature_Tracking Writeupp

This WRITEUP contains writeup addressing tasks of 2D Feature Tracking project of Udacity's Sensor Fusion Nanodegree. Performance evaluation of feature detectors and descriptors is also listed.

An additional application `./evaluate` was implemented, that displays statistics that are listed in evaluation tables.

#### MP.1 Data Buffer Optimization
Since we only need to preserve at most 2 images in memory during keypoint matching, the data buffer is implemented using double-ended queue `std::deque` and its size is kept at 2.

#### MP.2 Keypoint Detection
Detectors HARRIS, FAST, BRISK, ORB, AKAZE, and SIFT were implemented using `cv::goodFeaturesToTrack` and `cv::Feature2D` subclasses shipped with OpenCV.

#### MP.3 Keypoint Removal
Keypoints are filtered by preserving only the keypoints located inside `cv::Rect vehicleRect(535, 180, 180, 150)`, which contains the car of interest.

#### MP.4 Keypoint Descriptors
BRIEF, ORB, FREAK, AKAZE and SIFT descriptors were implemented using `cv::DescriptorExtractor` subclasses shipped with OpenCV.

#### MP.5 Descriptor Matching
FLANN and k-nearest neighbours matching was implemeted using `cv::DescriptorMatcher` subclasses shipped with OpenCV.

#### MP.6 Descriptor Distance Ratio
Distance ratio test for rejecting false positives is implemented, with SSD ratio threshold set to 0.8.

#### MP.7 Performance Evaluation 1
The following table lists number of keypoints detected in all images for all image descriptors inside rectangle containing vehicle of interest.

| Detector Name | Total Keypoints |
|:-------------:|:---------------:|
|   SHITOMASI   |       1189      |
|     HARRIS    |       561       |
|     AKAZE     |       1655      |
|     BRISK     |       2714      |
|      FAST     |       4136      |
|      ORB      |       1150      |
|      SIFT     |       1371      |

#### MP.8 Performance Evaluation 2
Rows list detector names, columns list descriptor names. N/A was inserted in case the given detector/descriptor pair could not be used. The numbers describe total number of matched keypoints for a given detector/descriptor pair.

|           | BRISK | BRIEF |  ORB | FREAK | AKAZE | SIFT |
|:---------:|:-----:|:-----:|:----:|:-----:|:-----:|:----:|
| SHITOMASI |  771  |  953  |  914 |  768  |  N/A  |  946 |
|   HARRIS  |  393  |  461  |  450 |  403  |  N/A  |  468 |
|   AKAZE   |  1204 |  1257 | 1177 |  1181 |  1249 | 1290 |
|   BRISK   |  1545 |  1676 | 1480 |  1497 |  N/A  | 1659 |
|    FAST   |  2209 |  2850 | 2785 |  2246 |  N/A  | 2949 |
|    ORB    |  744  |  540  |  754 |  417  |  N/A  |  772 |
|    SIFT   |  586  |  693  |  N/A |  591  |  N/A  |  809 |

#### MP.9 Performance Evaluation 3
This table is similar to the one presented in previous section, except this time, mean description and detection time is listed instead of number of matched keypoints.

|           |   BRISK   |   BRIEF  |    ORB   |   FREAK  |  AKAZE  |   SIFT   |
|:---------:|:---------:|:--------:|:--------:|:--------:|:-------:|:--------:|
| SHITOMASI |  123.49ms |  7.27ms  |  8.52ms  |  25.64ms |   N/A   |  16.31ms |
|   HARRIS  |  135.81ms |  7.25ms  |  8.58ms  |  27.42ms |   N/A   |  16.73ms |
|   AKAZE   | 183.512ms |  52.89ms |  57.09ms |  71.60ms | 94.06ms |  66.41ms |
|   BRISK   |  277.18ms | 156.80ms | 163.90ms | 176.14ms |   N/A   | 184.53ms |
|    FAST   |  126.42ms |  2.14ms  |  3.52ms  |  21.72ms |   N/A   |  14.77ms |
|    ORB    |  132.43ms |  5.76ms  |  11.60ms |  24.99ms |   N/A   |  30.64ms |
|    SIFT   |  220.81ms |  70.67ms |    N/A   |  88.15ms |   N/A   | 113.15ms |

TOP 3 detector/descriptor pairs, judged by execution time (less is better): FAST/BRIEF, FAST/ORB, ORB/BRIEF.

All these combinations allow for real-time processing needed for self-driving cars.

