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
