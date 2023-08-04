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
        /**
            @brief currently used key for inferences on letters and numbers
            ex.: score1, score2, clock
        */
        string currentKey;
        /**
            @brief currently used common part of key for inferences on letters and numbers
            ex.: score, clock
        */
        string currentCommonKey;
        /// @brief vector std::string with important keys that define the league's elements 
        vector<string> keys;
        vector<string> hasLetters;
        vector<string> hasOnlyLetters;
        vector<string> nonEmpty;
        
    public:
        /**
            @brief constructor of class camicasa::League
            @param name std::string name of league
            @param sportType camicasa::SportType that defines what sport that league practices
            @param hasLetters vector of std::string defining a group of keys that should have
            letters in the final recognized text
            @param hasOnlyLetters vector of std::string defining a group of keys that should not have
            any numbers in the final recognized text
            @param hasOnlyLetters vector of std::string defining a group of keys that should not have
            empty parts in the final recognized text
        */
        League(string name, SportType sportType,
               const vector<string> hasLetters     = vector<string>(),
               const vector<string> hasOnlyLetters = vector<string>(),
               const vector<string> nonEmpty       = vector<string>());

        /// @brief destructor of class camicasa::League
        ~League();

        /// @returns camicasa::League::name
        string getName();

        /// @returns camicasa::League::currentCommonKey
        string getCurrentCommonKey();

        /// @returns camicasa::League::keys
        vector<string> getKeys();
        
        /**
            @brief populates the keys defining important elements
            @param key std::string json key that cointains all the elements
            @param json Json:::Value corresponding the json file with the elements
            @returns returns vector of std::string corresponding to the keys
        */
        vector<string> populateKeys(string masterKey, Json::Value json);

        /**
            @brief sets current key used internally
            @note check out camicasa::splitKeyLetters for more information
        */
        void setCurrentKey(string key);

        /**
            @brief determines if current common key (part that doesnt contain numbers) is part of the
            group that may have letters in the element
            @returns returns true if key is present in the hasLetters group
        */
        bool keyHasLetters();

        /**
            @brief determines if information held by current common key has numbers
            @returns returns true if key is not present in the hasOnlyLetters group
        */
        bool keyHasNumbers();

        /**
            @brief determines if information held by current common key has only numbers
            @returns returns true if key is not present in the hasLetters group
        */
        bool keyOnlyNumbers();

        /**
            @brief determines if information held by current common key cannot have empty parts
            @returns returns true if key is present in the nonEmpty group
        */
        bool isNonEmpty();

        /**
            @brief looks for string part in the current key, removing the numbers at the end
            @note check out camicasa::League::currentCommonKey
        */
        void splitKeyLetters();

        /**
            @brief processes a given text, cleaning and normalizing it
            @param[inout] text std::string input text
        */
        void postprocess(string &text);

        /**
            @brief sets the white and black list of tesseract ocr, depending if there should be letters on the recognized text
            @param ocr tesseract::TessBaseAPI object already initialized
        */
        void tuneWhiteBlackLists(tesseract::TessBaseAPI *ocr);
    };

}
#endif
