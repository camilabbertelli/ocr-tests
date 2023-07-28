#include "utils.hpp"
#include "League.hpp"

using namespace camicasa;

vector<League*> populateLeagues(){
    vector<League*> leagues;

    leagues.push_back(new League("bundesliga", SOCCER));
    leagues.push_back(new League("liga-italiana", SOCCER));
    leagues.push_back(new League("NBA", BASKETBALL));
    leagues.push_back(new League("NFL1", FOOTBALL, {"downDistance"}));
    leagues.push_back(new League("NFL2", FOOTBALL, {"downDistance"}));
    leagues.push_back(new League("pt-cup", SOCCER));
    leagues.push_back(new League("taca-da-liga", SOCCER));

    return leagues;
}

void destroyLeagues(vector<League*> leagues){
    for (vector<League*>::iterator it = leagues.begin(); it != leagues.end();)
    {
        delete *it;
        it = leagues.erase(it);
    }
    
}

int main(int argc, char** argv)
{

    vector<League*> leagues = populateLeagues();

    // reset folder to store cropped data
    if (system("rm -r croppedImages") != -1)
        mkdir("croppedImages", 0777);

    // initializing tesseract variables
    tesseract::TessBaseAPI *ocr;
    ocr = new tesseract::TessBaseAPI();
    ocr->Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);

    for (League* league : leagues){
        string leagueName = league->getName();
        // opening json file
        ifstream jsonFile("leagues/" + leagueName + "/" + leagueName + ".json", ifstream::binary);

        Json::Value json;
        jsonFile >> json;

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

            Mat img = imread(pathImg);
            cout << leagueName << " " << imgName << "\n";
            puts("----------------");

            vector<string> keysVisited;
            

            for (string key : keys){

                // ensures that when there are two scores for example, score1 and score2, 
                //if we already saw score2, there is no need to compute text again
                if (contains(keysVisited, key))
                    continue;

                keysVisited.push_back(key);

                // checks 
                string commonKey;
                bool keepLetters = league->isKeyOutlier(key, commonKey);

                // tune available alphabet for better recognition
                tuneWhiteBlackLists(ocr, keepLetters);
                
                // variables used for control and return results
                string currentKey = key;

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
                    postprocess(temp, keepLetters);
                    textResultOpenCV += temp + " ";

                    // using tesseract recognition with tesseracts preprocess only
                    temp = tesseractRecognition(cropped, ocr, tesseract::PSM_SINGLE_LINE, 3);
                    postprocess(temp, keepLetters);
                    textResultTessPreProc += temp + " ";

                    // using tesseract recognition with custom preprocess
                    preprocess(cropped, cropped);

                    temp = tesseractRecognition(cropped, ocr, tesseract::PSM_RAW_LINE, 1);
                    postprocess(temp, keepLetters);
                    textResultCustomPreProc += temp + " ";

                    temp = tesseractRecognition(cropped, ocr, tesseract::PSM_SINGLE_LINE, 3);
                    postprocess(temp, keepLetters);
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
                removeIfLast(textResultOpenCV, ' ');
                removeIfLast(textResultTessPreProc, ' ');
                removeIfLast(textResultCustomPreProc, ' ');
                removeIfLast(textResultCustomTessPreProc, ' ');

                cout << "OPENCV -> " << commonKey << " : '" << textResultOpenCV << "'\n";
                cout << "TESSERACT WITH TESSERACTS' PREPROCESS -> " << commonKey << " : '" << textResultTessPreProc << "'\n";
                cout << "TESSERACT WITH CUSTOM PREPROCESS -> " << commonKey << " : '" << textResultCustomPreProc << "'\n";
                cout << "TESSERACT WITH CUSTOM AND TESSERACTS' PREPROCESS -> " << commonKey << " : '" << textResultCustomTessPreProc << "'\n\n";

                // reset folder to store cropped data
                mkdir(("croppedImages/" + leagueName).c_str(), 0777);
                //imshow(commonKey, resultFinal);
                imwrite("croppedImages/" + leagueName + "/" + commonKey + "_" + imgName, resultFinal);
            }

            //k = waitKey(0);
            cv::destroyAllWindows();

            // q for quit
            if (k == 113) {

                destroyLeagues(leagues);
                ocr->End();
                return 0;
            }

            // n for next league
            if (k == 110)
                break;
        }
    }

    destroyLeagues(leagues);
    ocr->End();
    return 0;
}
