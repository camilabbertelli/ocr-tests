#include "League.hpp"

camicasa::League::League(string name, SportType sportType, const vector<string> hasLetters, const vector<string> hasOnlyLetters, const vector<string> nonEmpty)
{
    this->name = name;
    this->sportType = sportType;
    this->hasLetters = hasLetters;
    this->hasOnlyLetters = hasOnlyLetters;
    this->nonEmpty = nonEmpty;
}

camicasa::League::~League() {
    this->keys.clear();
}

string camicasa::League::getName(){
    return this->name;
}

string camicasa::League::getCurrentCommonKey(){
    return this->currentCommonKey;
}

vector<string> camicasa::League::getKeys() {
    return this->keys;
}

vector<string> camicasa::League::populateKeys(string masterKey, Json::Value json)
{
    this->keys.clear();
    for (auto it = json[masterKey].begin(); it != json[masterKey].end(); ++it)
        this->keys.push_back(it.key().asString());

    return this->keys;
}

void camicasa::League::setCurrentKey(string key){
    this->currentKey = key;
    this->splitKeyLetters();
}

bool camicasa::League::keyHasLetters(){
    return contains(this->hasLetters, this->currentCommonKey);
}

bool camicasa::League::keyHasNumbers(){
    return !contains(this->hasOnlyLetters, this->currentCommonKey);
}

bool camicasa::League::keyOnlyNumbers(){
    return !contains(this->hasLetters, this->currentCommonKey);
}

bool camicasa::League::isNonEmpty()
{
    return contains(this->nonEmpty, this->currentCommonKey);
}

void camicasa::League::splitKeyLetters(){
    this->currentCommonKey.clear();
    for (char ch : currentKey) {
        if (isdigit(ch))
            break;
        this->currentCommonKey.push_back(ch);
    }

}

void camicasa::League::postprocess(string &text){
    if (this->sportType != BASKETBALL)
        replace(text.begin(), text.end(), '.', '-');

    replace(text.begin(), text.end(), '-', ' ');
    removeIfFirstLast(text, '\n');
    removeIfFirstLast(text, ' ');

    if (this->currentCommonKey == "downDistance")
        return;

    // flags that determine when to do or not certain procedures
    bool hasSomeNumbers = this->keyHasNumbers();
    bool hasOnlyNumbers = this->keyOnlyNumbers();
    bool notEmpty       = this->isNonEmpty();

    if (text.empty() && notEmpty && hasOnlyNumbers)
        text = "0";

    // if we only have numbers, we can make a few replacements in ambiguos letters
    if (hasOnlyNumbers) {
        if (text.size() <= 3){
            replace(text.begin(), text.end(), 'o', '0');
            replace(text.begin(), text.end(), 'O', '0');
            replace(text.begin(), text.end(), 'c', '0');
            replace(text.begin(), text.end(), 'C', '0');
            replace(text.begin(), text.end(), 'B', '8');
        }

        // if there are only numbers, remove letters from text, maintaining punctuations
        removeLettersFromNumbers(text);
    }

    // if has numbers but text contains no numbers, clear text (garbage detected)
    if (hasSomeNumbers && !containsNumber(text))
        text.clear();
        
    toLowerString(text);

    if (this->sportType == FOOTBALL && this->currentCommonKey == "downDistance"){

        int maxDownRule = 4;
        int minDownRule = 1;
        // extract first number 

        if (text.empty())
            return;

        string aux;
        // if is in range of possible downs
        if (text.at(0) - '0' >= minDownRule && text.at(0) - '0' <= maxDownRule){
            aux = text.at(0) + ' ';

            // check if there is only one number
            int countNumbers = 0;
            bool inNumber = false;
            for (char ch : text)
                if (!isdigit(ch))
                    inNumber = false;
                else if (!inNumber) {
                    inNumber = true;
                    countNumbers++;
                }

            if (countNumbers == 1) {
                removeIfFirstLast(text, ' ');
                text.append(" down");
            }
        }

            
    }
}


void camicasa::League::tuneWhiteBlackLists(tesseract::TessBaseAPI *ocr){

    bool hasLetters = this->keyHasLetters();
    
    const char* tessedit_char_whitelist = (hasLetters) ? "0123456789.:- ()&abccdefghijklmnopqrstuvwxyz" : "0123456789.:- ()";
    const char* tessedit_char_blacklist = (hasLetters) ? "" : "abccdefghijklmnopqrstuvwxyz";

    ocr->SetVariable("tessedit_char_whitelist", tessedit_char_whitelist);
    ocr->SetVariable("tessedit_char_blacklist", tessedit_char_blacklist);
}
