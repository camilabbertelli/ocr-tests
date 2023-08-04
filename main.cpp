#include "utils.hpp"
#include "League.hpp"

using namespace camicasa;

vector<League*> populateLeagues(){
    vector<League*> leagues;

    leagues.push_back(new League("bundesliga", SOCCER, {}, {}, {"score"}));
    leagues.push_back(new League("liga-italiana", SOCCER, {}, {}, {"score"}));
    leagues.push_back(new League("NBA", BASKETBALL, {}, {}, {"score"}));
    leagues.push_back(new League("NFL1", FOOTBALL, {}, {}, {"score"}));
    leagues.push_back(new League("NFL2", FOOTBALL, {}, {}, {"score"}));
    leagues.push_back(new League("pt-cup", SOCCER, {}, {}, {"score"}));
    leagues.push_back(new League("taca-da-liga", SOCCER, {}, {}, {"score"}));

    return leagues;
}

void destroyLeagues(vector<League*> leagues){
    for (vector<League*>::iterator it = leagues.begin(); it != leagues.end();)
    {
        delete *it;
        it = leagues.erase(it);
    }
    
}

/*
void fourPointsTransform(const Mat &frame, const Point2f vertices[], Mat &result)
{
    const Size outputSize = Size(100, 32);

    Point2f targetVertices[4] = {
        Point(0, outputSize.height - 1),
        Point(0, 0),
        Point(outputSize.width - 1, 0),
        Point(outputSize.width - 1, outputSize.height - 1)};
    Mat rotationMatrix = getPerspectiveTransform(vertices, targetVertices);

    warpPerspective(frame, result, rotationMatrix, outputSize);
}*/

int main(int argc, char** argv)
{

    vector<League*> leagues = populateLeagues();

    // reset folder to store cropped data
    if (system("rm -r croppedImages") != -1)
        mkdir("croppedImages", 0777);

    // TODO: corrigir isso amanha
    assert(system("rm obtained.json") != -1);

    // initializing tesseract variables
    tesseract::TessBaseAPI *ocr;
    ocr = new tesseract::TessBaseAPI();
    ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);


    Json::Value jsonObtained;
    Json::Value jsonExpected;

    ifstream jsonFileExpected("expected.json", ifstream::binary);

    jsonFileExpected >> jsonExpected;

    for (League* league : leagues){
        string leagueName = league->getName();
        // opening json file
        ifstream jsonFileLeague("leagues/" + leagueName + "/" + leagueName + ".json", ifstream::binary);

        Json::Value json;
        jsonFileLeague >> json;

        if (json.empty())
            continue;

        puts("");
        puts("**************************");
        cout << "LEAGUE : " << leagueName << "\n";
        puts("**************************");
        puts("");

        string elementsKey = "elements";

        vector<string> keys = league->populateKeys(elementsKey, json);
        int k = 0;

        //reading the image

        vector<string> pathImgs;
        glob("leagues/" + leagueName + "/*.jpg", pathImgs, false);

        for (string pathImg : pathImgs) {

            int pos = pathImg.find_last_of('/');
            string imgName = pathImg.substr(pos + 1);
            pos = imgName.find('.');
            string imgNumber = imgName.substr(0, pos);

            Mat img = imread(pathImg);
            cout << leagueName << " " << imgNumber << "\n";
            puts("--------------------------");

            vector<string> keysVisited;
            

            for (string key : keys){

                league->setCurrentKey(key);

                // ensures that when there are two scores for example, score1 and score2, 
                //if we already saw score2, there is no need to compute text again
                if (contains(keysVisited, key))
                    continue;

                keysVisited.push_back(key);

                // tune available alphabet for better recognition
                league->tuneWhiteBlackLists(ocr);
                
                // variables used for control and return results
                string currentKey = key;
                string commonKey = league->getCurrentCommonKey();

                string textResultOpenCV;
                string textResultTessPreProc;
                string textResultCustomPreProc;
                string textResultCustomTessPreProc;

                Mat resultFinal;

                int numberSplit = 1;
                while(contains(keys, currentKey)){
                    keysVisited.push_back(currentKey);

                    // get corresponding area of the original image
                    int x = json[elementsKey][currentKey]["x"].asInt();
                    int y = json[elementsKey][currentKey]["y"].asInt();
                    int w = json[elementsKey][currentKey]["w"].asInt();
                    int h = json[elementsKey][currentKey]["h"].asInt();

                    /*if (currentKey == "downDistance")
                    {
                        Mat mask = Mat(img.rows, img.cols, img.type(), 0.0);
                        //rectangle(mask, Rect(x, y, w, h), Scalar(255, 255, 255), -1);


                        Mat result;
                        //bitwise_and(img, mask, result);
                        result = mask.clone();

                        Mat cropped = img(Rect(x, y, w, h));
                        // enlarge images for better recognition
                        resize(cropped, cropped, Size(), 3, 3, INTER_NEAREST);
                        cropped.copyTo(result(Rect(0, 0, cropped.cols, cropped.rows)));

                        // Load model weights
                        TextDetectionModel_DB model("../models/DB_IC15_resnet50.onnx");
                        // Post-processing parameters
                        float binThresh = 0.3;
                        float polyThresh = 0.8;
                        uint maxCandidates = 4;
                        double unclipRatio = 2.5;
                        model.setBinaryThreshold(binThresh)
                            .setPolygonThreshold(polyThresh)
                            .setMaxCandidates(maxCandidates)
                            .setUnclipRatio(unclipRatio);
                        // Normalization parameters
                        double scale = 1.0 / 255.0;
                        Scalar mean = Scalar(122.67891434, 116.66876762, 104.00698793);

                        // The input shape
                        Size inputSize = Size(736, 736);
                        model.setInputParams(scale, inputSize, mean);

                        std::vector<std::vector<Point>> detResults;
                        model.detect(result, detResults);
                        Mat frame2 = result.clone();
                        // Visualization
                        polylines(result, detResults, true, Scalar(0, 255, 0), 2);

                        if (detResults.size() > 0)
                        {
                            // Text Recognition
                            Mat recInput;
                            
                            recInput = result;
                            
                            std::vector<std::vector<Point>> contours;
                            for (uint i = 0; i < detResults.size(); i++)
                            {
                                const auto &quadrangle = detResults[i];
                                CV_CheckEQ(quadrangle.size(), (size_t)4, "");

                                contours.emplace_back(quadrangle);

                                std::vector<Point2f> quadrangle_2f;
                                for (int j = 0; j < 4; j++)
                                    quadrangle_2f.emplace_back(quadrangle[j]);

                                // Transform and Crop
                                Mat cropped1;
                                fourPointsTransform(recInput, &quadrangle_2f[0], cropped1);

                                std::string recognitionResult = opencvRecognition(cropped1,
                                                                                  "../models/crnn_cs.onnx",
                                                                                  "../models/alphabet_94.txt");
                                std::cout << i << ": (before)'" << recognitionResult << "'" << std::endl;
                                league->postprocess(recognitionResult);
                                std::cout << i << ": (after)'" << recognitionResult << "'" << std::endl;

                                putText(frame2, recognitionResult, quadrangle[3], FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
                            }
                            polylines(frame2, contours, true, Scalar(0, 255, 0), 2);
                        }
                        else
                        {
                            std::cout << "No Text Detected." << std::endl;
                        }
                        imshow("welllll", frame2);
                        waitKey();
                    }*/

                    Mat cropped = img(Rect(x, y, w, h));
                    string temp;

                    // enlarge images for better recognition
                    resize(cropped, cropped, Size(), 3, 3, INTER_NEAREST);

                    // using opencv recognition mechanism
                    temp = opencvRecognition(cropped, 
                                             "../models/crnn_cs.onnx", 
                                             "../models/alphabet_94.txt");
                    league->postprocess(temp);
                    textResultOpenCV += temp + " ";

                    // using tesseract recognition with tesseracts preprocess only
                    temp = tesseractRecognition(cropped, ocr, tesseract::PSM_SINGLE_LINE, 3);
                    league->postprocess(temp);
                    textResultTessPreProc += temp + " ";

                    // using tesseract recognition with custom preprocess
                    preprocess(cropped, cropped);

                    temp = tesseractRecognition(cropped, ocr, tesseract::PSM_RAW_LINE, 1);
                    league->postprocess(temp);
                    textResultCustomPreProc += temp + " ";

                    temp = tesseractRecognition(cropped, ocr, tesseract::PSM_SINGLE_LINE, 3);
                    league->postprocess(temp);
                    textResultCustomTessPreProc += temp + " ";

                    if (resultFinal.empty())
                        resultFinal = cropped.clone();
                    else{
                        // resize cv::Mat so they have the same height 
                        int betterHeight = max(resultFinal.rows, cropped.rows);

                        resize(cropped, cropped, Size(cropped.cols, betterHeight));
                        resize(resultFinal, resultFinal, Size(resultFinal.cols, betterHeight));

                        hconcat(resultFinal, cropped, resultFinal);
                    }

                    currentKey = commonKey + to_string(++numberSplit);
                }

                // remove extra space in the end of every string
                removeIfFirstLast(textResultOpenCV, ' ');
                removeIfFirstLast(textResultTessPreProc, ' ');
                removeIfFirstLast(textResultCustomPreProc, ' ');
                removeIfFirstLast(textResultCustomTessPreProc, ' ');

                cout << "________\n";
                cout << "EXPECTED         -> " << commonKey << " : "   << jsonExpected[leagueName][imgNumber][commonKey] << "\n\n";
                cout << "OPENCV           -> " << commonKey << " : \"" << textResultOpenCV << "\"\n";
                cout << "TESSERACT ONLY   -> " << commonKey << " : \"" << textResultTessPreProc << "\"\n";
                cout << "TESSERACT CUSTOM -> " << commonKey << " : \"" << textResultCustomPreProc << "\"\n";
                cout << "TESSERACT ALL    -> " << commonKey << " : \"" << textResultCustomTessPreProc << "\"\n\n";

                jsonObtained[leagueName][imgNumber][commonKey]["opencv"]     = textResultOpenCV;
                jsonObtained[leagueName][imgNumber][commonKey]["tessOnly"]   = textResultTessPreProc;
                jsonObtained[leagueName][imgNumber][commonKey]["tessCustom"] = textResultCustomPreProc;
                jsonObtained[leagueName][imgNumber][commonKey]["tessAll"]    = textResultCustomTessPreProc;

                // reset folder to store cropped data
                mkdir(("croppedImages/" + leagueName).c_str(), 0777);
                //imshow(commonKey, resultFinal);
                imwrite("croppedImages/" + leagueName + "/" + commonKey + "_" + imgName, resultFinal);
            }
            
            cv::destroyAllWindows();
        }
    }
    
    writeJson(jsonObtained, "obtained.json");
    destroyLeagues(leagues);
    ocr->End();
    return 0;
}
