#include <string>
#include <map>
#include <vector>
using namespace std;
// zmienne dotyczące bazy danych
const int playableCardsDBSize = 48; // suma kart grywalnych
const int monstersDBSize = 13;
const int partyLeadersDBSize = 6;
const int challengesDBSize = 5;
const int modificatorsDBSize = 10;

map<int,map<string,string>> HeroesDB = {
    //{id, {inner_key, val}}
    {1, {
        {"klasa", "bard"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {2, {
        {"klasa", "bard"},
        {"minDiceVal", "5"},
        {"signDiceVal", "-"},
        {"effect", "????"}
    }},
    {3, {
        {"klasa", "bard"},
        {"minDiceVal", "5"},
        {"signDiceVal", "-"},
        {"effect", "????"}
    }},
    {4, {
        {"klasa", "bard"},
        {"minDiceVal", "5"},
        {"signDiceVal", "-"},
        {"effect", "????"}
    }},
    {5, {
        {"klasa", "bard"},
        {"minDiceVal", "5"},
        {"signDiceVal", "-"},
        {"effect", "????"}
    }},
    {6, {
        {"klasa", "bard"},
        {"minDiceVal", "5"},
        {"signDiceVal", "-"},
        {"effect", "????"}
    }},
    {7, {
        {"klasa", "bard"},
        {"minDiceVal", "5"},
        {"signDiceVal", "-"},
        {"effect", "????"}
    }},
    {8, {
        {"klasa", "bard"},
        {"minDiceVal", "5"},
        {"signDiceVal", "-"},
        {"effect", "????"}
    }},
    {9, {
        {"klasa", "czarodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {10, {
        {"klasa", "czarodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {11, {
        {"klasa", "czarodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {12, {
        {"klasa", "czarodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {13, {
        {"klasa", "czarodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {14, {
        {"klasa", "czarodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {15, {
        {"klasa", "czarodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {16, {
        {"klasa", "czarodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {17, {
        {"klasa", "lowca"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {18, {
        {"klasa", "lowca"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {19, {
        {"klasa", "lowca"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {20, {
        {"klasa", "lowca"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {21, {
        {"klasa", "lowca"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {22, {
        {"klasa", "lowca"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {23, {
        {"klasa", "lowca"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {24, {
        {"klasa", "lowca"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {25, {
        {"klasa", "straznik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {26, {
        {"klasa", "straznik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {27, {
        {"klasa", "straznik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {28, {
        {"klasa", "straznik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {29, {
        {"klasa", "straznik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {30, {
        {"klasa", "straznik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {31, {
        {"klasa", "straznik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {32, {
        {"klasa", "straznik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {33, {
        {"klasa", "wojownik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {34, {
        {"klasa", "wojownik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {35, {
        {"klasa", "wojownik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {36, {
        {"klasa", "wojownik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {37, {
        {"klasa", "wojownik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {38, {
        {"klasa", "wojownik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {39, {
        {"klasa", "wojownik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {40, {
        {"klasa", "wojownik"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {41, {
        {"klasa", "zlodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {42, {
        {"klasa", "zlodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {43, {
        {"klasa", "zlodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {44, {
        {"klasa", "zlodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {45, {
        {"klasa", "zlodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {46, {
        {"klasa", "zlodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {47, {
        {"klasa", "zlodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
    {48, {
        {"klasa", "zlodziej"},
        {"minDiceVal", "2"},
        {"signDiceVal", "+"},
        {"effect", "????"}
    }},
}; // Baza wiedzy

map<int,map<string,vector<int>>> monstersDB = {
    //{id, {inner_key, val}}
    {1, {
        {"requirements", {0,0,0,0,0,1,2}},
        {"val", {8}},
    }},
    {2, {
        {"requirements", {0,1,0,0,0,0,2}},
        {"val", {8}},
    }},
    {3, {
        {"requirements", {0,0,0,0,0,0,4}},
        {"val", {8}},
    }},
    {4, {
        {"requirements", {0,0,0,0,0,0,2}},
        {"val", {8}},
    }},
    {5, {
        {"requirements", {0,0,0,0,0,0,3}},
        {"val", {9}},
    }},
    {6, {
        {"requirements", {0,0,0,0,0,0,1}},
        {"val", {10}},
    }},
    {7, {
        {"requirements", {0,0,0,0,0,0,2}},
        {"val", {9}},
    }},
    {8, {
        {"requirements", {0,0,0,0,0,0,1}},
        {"val", {11}},
    }},
    {9, {
        {"requirements", {0,0,0,0,0,0,2}},
        {"val", {9}},
    }},
    {10, {
        {"requirements", {0,0,0,0,0,0,2}},
        {"val", {10}},
    }},
    {11, {
        {"requirements", {1,0,0,0,0,0,2}},
        {"val", {8}},
    }},
    {12, {
        {"requirements", {0,0,0,0,0,0,3}},
        {"val", {10}},
    }},
    {13, {
        {"requirements", {0,0,0,0,1,0,2}},
        {"val", {8}},
    }},
}; // Baza wiedzy

map<int,map<string,int>> modsDB = {
    //{id, {inner_key, val}}
    {201, {
        {"+", 4},
        {"-", 0},
    }},
    {202, {
        {"+", 3},
        {"-", 1},
    }},
    {203, {
        {"+", 2},
        {"-", 2},
    }},
    {204, {
        {"+", 1},
        {"-", 3},
    }},
    {205, {
        {"+", 0},
        {"-", 4},
    }},
    {206, {
        {"+", 4},
        {"-", 0},
    }},
    {207, {
        {"+", 3},
        {"-", 1},
    }},
    {208, {
        {"+", 2},
        {"-", 2},
    }},
    {209, {
        {"+", 1},
        {"-", 3},
    }},
    {210, {
        {"+", 0},
        {"-", 4},
    }},
    
}; // Baza wiedzy

map<int,map<string,string>> partyLeadersDB = {
    //{id, {inner_key, val}}
    {1, {
        {"klasa", "bard"},
        {"effect", "????"}
    }},
    {2, {
        {"klasa", "czarodziej"},
        {"effect", "????"}
    }},
    {3, {
        {"klasa", "lowca"},
        {"effect", "????"}
    }},
    {4, {
        {"klasa", "straznik"},
        {"effect", "????"}
    }},
    {5, {
        {"klasa", "wojownik"},
        {"effect", "????"}
    }},
    {6, {
        {"klasa", "zlodziej"},
        {"effect", "????"}
    }},
}; // Baza wiedzy