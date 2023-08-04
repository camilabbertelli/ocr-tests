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

string camicasa::opencvRecognition(Mat &input, string modelPath, string vocabPath)
{
    // opencv ocr set recognition model
    TextRecognitionModel model(modelPath);
    model.setDecodeType("CTC-greedy");

    // opencv ocr set vocabulary
    std::ifstream vocFile;
    vocFile.open(vocabPath);
    CV_Assert(vocFile.is_open());

    string vocLine;
    std::vector<string> vocabulary;
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

void camicasa::toLowerString(string &text){
    transform(text.begin(), text.end(), text.begin(), ::tolower);
}

void camicasa::removeIfFirstLast(string &text, char c){
    // remove from the beginning
    int first = 0;
    while (text[first] == c)
        first++;
    text = text.substr(first, text.size());

    // remove from the end
    int last = text.size() - 1;
    while (last >= 0 && text[last] == c)
        --last;
    text = text.substr(0, last + 1);
}

void camicasa::removeLettersFromNumbers(string& text){
    string aux;
    bool lastSeenNumber = false;

    for (char ch : text)
        if (isalpha(ch)){
            if (lastSeenNumber){
                lastSeenNumber = false;
                aux.push_back(' ');
            }
        }
        else{
            lastSeenNumber = true;
            aux.push_back(ch);
        }
    
    text = aux;
}

void camicasa::writeJson(Json::Value json, string path){
    if (!json.empty())
    {
        ofstream file;

        file.open(path);

        Json::StyledWriter jsonWriter;
        file << jsonWriter.write(json);

        file.close();
    }
}

bool camicasa::containsNumber(string text){

    for (char ch : text)
        if (isdigit(ch))
            return true;

    return false;
}