#include "League.hpp"

camicasa::League::League(string name, SportType sportType, const vector<string> outliers) {
    this->name = name;
    this->sportType = sportType;
    this->outliers = outliers;
}

camicasa::League::~League() {
    this->keys.clear();
}

string camicasa::League::getName(){
    return this->name;
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

bool camicasa::League::isKeyOutlier(string key, string &splitKey){
    splitKey = splitKeyLetters(key);
    return contains(this->outliers, splitKey);
}