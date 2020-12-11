#pragma once
#include <vector>
#include <memory>
#include <map>
#include <string>

const int c_suit = 5;
const int c_rank = 13;
const int c_minimum =3;
const int c_idxerror=-1;
enum E_SUIT
{
    e_joker=0,
    e_diamond,
    e_club,
    e_heart,
    e_spade
};

class Card
{
public:
    Card();
    Card(E_SUIT suit, int rank, int goldrank);
    static int genId(){return ++idx;}
    bool special();
    int getGoal();
public:
    E_SUIT suit_;
    int rank_;
    int id_;
    bool joker_;
    bool special_;
    bool magic_;
private:
    static int idx;    
};

class CardGroup
{
public:
    CardGroup(int idxSetMember=c_idxerror, bool opotional=false, int id = genId()) 
    : idxSetMember_(idxSetMember), optional_(opotional), id_(id){}
    static int genId(){return ++idx;}
    using PCard = std::shared_ptr<Card>;
    void removeCard(const PCard &card);
    void removeCard(int id);
    void removeGroup(const CardGroup &rhs);
    void reset();
    int getGoal();
    bool expandToMeld(const PCard &card);
    bool expandToRun(const PCard &card);
public:
    int id_;
    int idxSetMember_;
    bool optional_;
    std::vector<std::shared_ptr<Card>> cardlist_;   
private:
    static int idx;    

};

class GameRummy
{
public:
    using PCard = std::shared_ptr<Card>;
    static void printCardGroup(const CardGroup &group);
    static std::string getCardString(const Card &card);
    int match(const std::map<int, PCard> &hand, std::vector<CardGroup> &runList, 
                std::vector<CardGroup> &meldList, std::vector<CardGroup> &setList, std::vector<CardGroup> &unMatchedList);
    int buildRun(const std::map<int, PCard> &hand, std::vector<CardGroup> &runList, std::vector<CardGroup> &candidates);
    static int buildRunFromGroup(CardGroup &group, std::vector<CardGroup> &runList, std::vector<CardGroup> &candidates);
    static int buildMeldAndSet(std::vector<CardGroup> &candidates, std::vector<CardGroup> &meldList,
                                 std::vector<CardGroup> &setList);
    static int buildMeldFromTop(std::vector<CardGroup> &candidates, CardGroup &group, CardGroup &potential);
    static int buildSetFromTop(std::vector<CardGroup> &candidates, CardGroup &group, CardGroup &set);
    static void markSet(std::vector<CardGroup> &candidates, CardGroup &group);
    static bool tryBuildRunWithSpecial(CardGroup &specialGroup, CardGroup &meldCandidate, PCard &special);
    static int buildCandidates(std::vector<CardGroup> &runList, std::vector<CardGroup> &meldList, 
                                std::vector<CardGroup> &setList, std::vector<CardGroup> &candidates);
    static int getMaxCandidate(std::vector<CardGroup> &candidates, PCard &card);
    static bool expandToRunAndDelSource(std::vector<CardGroup> &runList, CardGroup &src);
    static bool expandToMeldAndDelSource(std::vector<CardGroup> &targetList, CardGroup &src, int idx);                            
private:
    static void removeSetFromCandidates(std::vector<CardGroup> &candidates);
public:
    int special_;
private:
    std::map<int, PCard> hand_; 
};
