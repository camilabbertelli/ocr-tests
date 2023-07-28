#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <fstream>

#include "utils.hpp"

using namespace cv;
using namespace std;
using namespace camicasa;

// variables to store the bounding box coordinates and other information in the json files
Point top_left_corner, bottom_right_corner;
int width = 0;
int height = 0;
string name;

// cv::Mat used
Mat image;
Mat temp;

string windowName = "League";

// function which will be called on mouse input
void drawRectangle(int action, int x, int y, int flags, void *userdata)
{

    // mark the top left corner when left mouse button is pressed
    if (action == EVENT_LBUTTONDOWN)
    {
        image = temp.clone();
        imshow(windowName, image);
        top_left_corner = Point(x, y);
    }
    // mark the bottom right corner when right mouse button is pressed
    else if (action == EVENT_RBUTTONDOWN)
    {
        // save rest of info temporarily

        bottom_right_corner = Point(x, y);
        width = abs(bottom_right_corner.x - top_left_corner.x);
        height = abs(bottom_right_corner.y - top_left_corner.y);

        rectangle(image, top_left_corner, bottom_right_corner, Scalar(0, 255, 0), 1, LINE_4);

        imshow(windowName, image);
        
        cout << "Please enter this bounding box's name : ";
        cin >> name;
        cout << "\n";
    }
}

// Main function
int main(int argc, char **argv)
{
    try
    {
        string league = argv[1];
        vector<string> pathImgs;
        glob(league + "/*.jpg", pathImgs, false);

        image = readImage(pathImgs.front());
        windowName = league;
    }
    catch(const exception& e)
    {
        puts("Please provide name of league to analyze!");
        puts("----Attention!----");
        puts("The folder must have the same name as the league and the first image will be used");
        return 0;
    }

    cout << "-----------------------------------------------------------------------------------\n"
         << "This is a tool for extracting the bounding boxes of wanted elements in the screen\n\n"

         << "To capture a bounding box: \n"
         << "LEFT MOUSE BUTTON - define the first pixel\n"
         << "RIGHT MOUSE BUTTON - define the second pixel\n"
         << "S - save that element\n"
         << "Q - quit\n\n"

         << "----Attention---\n"
         << "If you get the SCORE as a whole, name it score,\n"
         << "If not, name the parts as score1 and score2. Same for clock and others\n"
         << "Note : scoreExtra, clockExtra, etc, are designed for a lesser weigthed version of score and clock, for example\n"
         << "-----------------------------------------------------------------------------------\n\n";

    Json::Value json;
    int valueCount = 0;

    //temp image to clear the original
    temp = image.clone();
    
    namedWindow(windowName, WINDOW_AUTOSIZE);
    setMouseCallback(windowName, drawRectangle);

    int k = 0;

    // loop until q character is pressed
    while (k != 113)
    {
        imshow(windowName, image);
        k = waitKey(0);

        // If c is pressed, done one!
        if (k == 115)
        {
            if (name.empty())
                name = "value" + to_string(valueCount++);

            json["score"][name]["x"] = top_left_corner.x;
            json["score"][name]["y"] = top_left_corner.y;
            json["score"][name]["h"] = height;
            json["score"][name]["w"] = width;

            image = temp.clone();
            name.clear();
        }
    }

    // writing thee json object

    if (!json.empty())
    {
        ofstream file;
        stringstream pathWrite;

        pathWrite << argv[1] << "/" << argv[1] << ".json";

        file.open(pathWrite.str());

        Json::StyledWriter jsonWriter;
        file << jsonWriter.write(json);

        file.close();
    }

    cv::destroyAllWindows();
    return 0;
}