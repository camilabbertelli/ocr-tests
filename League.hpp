#ifndef _LEAGUE_
#define _LEAGUE_

#include "utils.hpp"

/// @brief namespace for all functions created by Camila Bertelli
namespace camicasa{

    /// @brief enum that describes the sport type of the league
    enum SportType {SOCCER, BASKETBALL, FOOTBALL};


    /// @brief class that controls a league and its important keys to search for
    class League
    {
    private:
        /// @brief camicasa::SportType that defines what sport that league practices
        SportType sportType;
        /// @brief std::string name of league
        string name;
        /// @brief vector std::string with important keys that define the league's elements 
        vector<string> keys;
        vector<string> outliers;
        
    public:
        /**
            @brief constructor of class camicasa::League
            @param name std::string name of league
            @param sportType camicasa::SportType that defines what sport that league practices
        */
        League(string name, SportType sportType, const vector<string> outliers = vector<string>());
        
        /// @brief destructor of class camicasa::League
        ~League();

        /**
            @brief populates the keys defining important elements
            @param key std::string json key that cointains all the elements
            @param json Json:::Value corresponding the json file with the elements
            @returns returns vector of std::string corresponding to the keys
        */
        vector<string> populateKeys(string masterKey, Json::Value json);

        /// @returns camicasa::League::keys
        vector<string> getKeys();

        /// @returns camicasa::League::name
        string getName();

        /**
            @brief determines if key (part that doenst contains numbers is part of the 
            group that may have letters in the element)
            @param key std::string to check
            @param spliKey std::string that holds the returned value of camicasa::splitKeyLetters,
            a method called
            @returns returns true if key is present in the outliers group
            @note check out camicasa::splitKeyLetters for more information
        */
        bool isKeyOutlier(string key, string &splitKey);
    };

}
#endif
