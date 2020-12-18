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
    CardGroup(int id = genId(), int idxSetMember=c_idxerror, bool opotional=false) 
    : id_(id),idxSetMember_(idxSetMember), optional_(opotional){}
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
    static void printCardGroup(const CardGroup &group, int specialRank = c_idxerror);
    static std::string getCardString(const Card &card);
    static bool greater4CardOrder(const PCard &a, const PCard &b){return a->rank_ > b->rank_;}
    static bool less4CardOrder(const PCard &a, const PCard &b){return a->rank_ < b->rank_;}
    int match(const std::map<int, PCard> &hand, std::vector<CardGroup> &runList, 
                std::vector<CardGroup> &meldList, std::vector<CardGroup> &setList, std::vector<CardGroup> &unmatchedList);
private:
    static int partitionHand(const std::map<int, PCard> &hand, std::vector<CardGroup> &candidates);
    static int initRunList(std::vector<CardGroup> &candidates, std::vector<CardGroup> &runList);
    static int buildGroupByScore(std::vector<CardGroup> &candidates, std::vector<CardGroup> &runList, 
                                 std::vector<CardGroup> &meldList, std::vector<CardGroup> &setList);
    static int arrangeUnmatched(std::vector<CardGroup> &unmatchedList, std::vector<CardGroup> &newList);
    static int buildRun(CardGroup &group, std::vector<CardGroup> &runList);
    static int buildRunFromTop(CardGroup &group, CardGroup &run);
    static int buildSetFromTop(std::vector<CardGroup> &candidates, CardGroup &group, CardGroup &set);
    static int expandTopMaxToRunlist(const vector<int> &runGoal, int rank, std::vector<CardGroup> &candidates, std::vector<CardGroup> &runList);

    

    int buildRun(const std::map<int, PCard> &hand, std::vector<CardGroup> &runList, std::vector<CardGroup> &candidates);
    static int buildRunFromGroup(CardGroup &group, std::vector<CardGroup> &runList, std::vector<CardGroup> &candidates);
    static int buildMeldAndSet(std::vector<CardGroup> &candidates, std::vector<CardGroup> &runList,
                               std::vector<CardGroup> &meldList, std::vector<CardGroup> &setList);
    
    static int buildMeldFromTop(std::vector<CardGroup> &candidates, CardGroup &group, CardGroup &meld);
    
    static void markSet(std::vector<CardGroup> &candidates, CardGroup &group);
    static void buildRunOrMeld(std::vector<CardGroup> &candidates, std::vector<CardGroup> &runList,
                               std::vector<CardGroup> &meldList, CardGroup &group, int idxInCandidates);
    static bool tryBuildRunWithSpecial(CardGroup &specialGroup, CardGroup &meldCandidate, CardGroup &newGroup, PCard &special);
    static int findCardInBothRunAndSet(std::vector<CardGroup> &tempRunList, int rankSet, PCard &cardInBoth, int &runInCandidates);
    static int findCardInBothMeldAndSet(std::vector<CardGroup> &candidates, std::vector<CardGroup> &meldList, int rank, PCard &card, int &meldInCandidates);
    static int buildMeldWithRank(std::vector<CardGroup> &candidates, CardGroup &group, int rank);
    static int buildCandidates(std::vector<CardGroup> &runList, std::vector<CardGroup> &meldList, 
                                std::vector<CardGroup> &setList, std::vector<CardGroup> &candidates);
    static int getMaxCandidate(std::vector<CardGroup> &candidates, PCard &card);
    static bool expandToRunAndDelSource(std::vector<CardGroup> &runList, CardGroup &src);
    static bool expandToMeldAndDelSource(std::vector<CardGroup> &targetList, CardGroup &src, int idx = 0);
    static void arrangeUnmatchList(std::vector<CardGroup> &unmatchedList, std::vector<CardGroup> &newList);                          
    static void removeSetFromCandidates(std::vector<CardGroup> &candidates);
    static bool buildMinorSet(std::vector<CardGroup> &unmatchedList, int idx, std::vector<CardGroup> &tempList, std::vector<CardGroup> &newList);
public:
    int special_;
private:
    std::map<int, PCard> hand_; 
};
