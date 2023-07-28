#ifndef _UTILS_
#define _UTILS_

#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <iostream>
#include <vector>
#include <cstdlib>
#include <sys/stat.h>

#include <jsoncpp/json/json.h>
#include <fstream>

using namespace cv;
using namespace cv::dnn;
using namespace std;

/// @brief macro that returns literal string 
#define stringify(name) #name

/// @brief namespace for all functions created by Camila Bertelli
namespace camicasa
{

    /**
        @brief turn a bgr image to grayscale
        @param[in] input cv::Mat input image
        @param[out] output cv::Mat output result
    */
    void grayscale(Mat &input, Mat &output);
    /**
        @brief turn a grayscale image to bgr
        @param[in] input cv::Mat input image
        @param[out] output cv::Mat output result
    */
    void bgrscale(Mat &input, Mat &output);

    /**
        @brief processes a given image, cleaning and highlighting the pixels
        @param[in] input cv::Mat input image
        @param[out] output cv::Mat output result
    */
    void preprocess(Mat &input, Mat &output);

    /**
        @brief processes a given text, cleaning and normalizing it
        @param[inout] text std::string input text
        @param keepLetters flag that when true, maintains all letters found in given text
    */
    void postprocess(string &text, bool keepLetters = false);

    /**
        @brief uses the cv::dnn::dnn4_v20230620::TextRecognitionModel class to extract text given an image
        @param input cv::Mat input image
        @param modelPath std::string with the path to the pretrained recognition model to use
        @param vocabPath std::string with the path to the vocabulary to use
        @returns returns std::string with the predicted text
    */
    string opencvRecognition(Mat &input, string modelPath, string vocabPath);

    /**
        @brief uses the Tesseract framework to extract text given an image
        @param input cv::Mat input image
        @param ocr tesseract::TessBaseAPI object already initialized
        @param pageSegMode tesseract::PageSegMode mode for page layout analysis
        @param bitsPerPixel amount of bits to consider per pixel
        @returns returns std::string with the predicted text
    */
    string tesseractRecognition(Mat &input, tesseract::TessBaseAPI *ocr, tesseract::PageSegMode pageSegMode, int bitsPerPixel);

    /**
        @brief uses the Tesseract framework to extract text given an image
        @param imgPath std::string with the path of the image to be read
        @returns returns cd::Mat with the read image (empty if path not found)
    */
    Mat readImage(string imgPath);

    /**
        @brief checks if element is contained in a vector
        @param vec vector of std::string to check
        @param element std::string element to check
        @returns returns true if the element is in the vector
    */
    bool contains(vector<string> &vec, string element);

    /**
        @brief looks for string part in the given string, removing the numbers at the end
        @param key std::string input text
        @returns returns only letters string
    */
    string splitKeyLetters(string key);

    /// @brief transforms given string to lowercase 
    void toLowerString(string &text);

    /// @brief removes last character if it matches the one given
    void removeIfLast(string &text, char c);

    void tuneWhiteBlackLists(tesseract::TessBaseAPI *ocr, bool keepLetters);
}

#endif