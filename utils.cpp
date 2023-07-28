#include "utils.hpp"

using namespace camicasa;

void camicasa::grayscale(Mat &input, Mat &output)
{
    cvtColor(output, output, COLOR_BGR2GRAY);
}

void camicasa::bgrscale(Mat &input, Mat &output)
{
    cvtColor(input, output, COLOR_GRAY2BGR, 3);
}

void camicasa::preprocess(Mat &input, Mat &output)
{
    // grayscale
    grayscale(input, output);

    // thresholding && binarization
    threshold(input, output, 0, 255, THRESH_BINARY | THRESH_OTSU);

    // guarantee most important information is in black
    int area = output.cols * output.rows;
    if (countNonZero(output) < (area - countNonZero(output)))
        output = ~output;

    // create structuring elements
    int morph_size = 2;
    Mat element = getStructuringElement(
        MORPH_RECT,
        Size(2 * morph_size + 1,
             2 * morph_size + 1));
    //dilate(output, output, element);
}

void camicasa::postprocess(string &text, bool keepLetters){

    toLowerString(text);

    removeIfLast(text, '\n');
    removeIfLast(text, ' ');

    if (!keepLetters)
        replace(text.begin(), text.end(), 'o', '0');
}

string camicasa::opencvRecognition(Mat &input, string modelPath, string vocabPath)
{
    // opencv ocr set recognition model
    TextRecognitionModel model(modelPath);
    model.setDecodeType("CTC-greedy");

    // opencv ocr set vocabulary
    std::ifstream vocFile;
    vocFile.open(vocabPath);
    CV_Assert(vocFile.is_open());

    String vocLine;
    std::vector<String> vocabulary;
    while (std::getline(vocFile, vocLine))
        vocabulary.push_back(vocLine);

    model.setVocabulary(vocabulary);

    // input params
    double scale = 1.0 / 127.5;
    Scalar mean = Scalar(127.5, 127.5, 127.5);
    Size inputSize = Size(100, 32);
    model.setInputParams(scale, inputSize, mean);

    // recognizition phase
    string recognitionResult = string(model.recognize(input));

    return recognitionResult;
}

string camicasa::tesseractRecognition(Mat &input, tesseract::TessBaseAPI *ocr, tesseract::PageSegMode pageSegMode, int bitsPerPixel)
{
    ocr->SetPageSegMode(pageSegMode);
    ocr->SetImage(input.data, input.cols, input.rows, bitsPerPixel, input.step);
    ocr->SetSourceResolution(300);

    string recognitionResult = ocr->GetUTF8Text();

    return recognitionResult;
}

Mat camicasa::readImage(string imgPath){
    ifstream imgFile(imgPath);
    if (imgFile) {
        imgFile.close();
        return imread(imgPath);
    }
    
    return Mat();
}

bool camicasa::contains(vector<string> &vec, string element){
    return (find(vec.begin(), vec.end(), element) != vec.end());
}

string camicasa::splitKeyLetters(string key){

    string letters;
    
    for (char const &ch : key) {
        if (isdigit(ch))
            break;
        letters.push_back(ch);
    }
    return letters;
}

void camicasa::toLowerString(string &text){
    transform(text.begin(), text.end(), text.begin(), ::tolower);
}

void camicasa::removeIfLast(string &text, char c){
    if (text.back() == c)
        text.pop_back();
}

void camicasa::tuneWhiteBlackLists(tesseract::TessBaseAPI *ocr, bool keepLetters){
    const char* tessedit_char_whitelist = (keepLetters) ? "0123456789.:- ()&abccdefghijklmnopqrstuvwxyz" : "0123456789.:- ()";
    const char* tessedit_char_blacklist = (keepLetters) ? "" : "abccdefghijklmnopqrstuvwxyz";

    ocr->SetVariable("tessedit_char_whitelist", tessedit_char_whitelist);
    ocr->SetVariable("tessedit_char_blacklist", tessedit_char_blacklist);
}