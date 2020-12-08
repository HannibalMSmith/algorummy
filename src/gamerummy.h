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
    e_spade,
    e_heart,
    e_diamond,
    e_club
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
    bool optional_;
private:
    static int idx;    
};



class CardGroup
{
public:
    CardGroup(int idxBuild = 0,int id = genId()) : idxBuild_(idxBuild), id_(id){}
    static int genId(){return ++idx;}
    using PCard = std::shared_ptr<Card>;
    void removeCard(const PCard &card);
    void removeGroup(const CardGroup &rhs);
    void reset();
    int getGoal();
public:
    int id_;
    int idxBuild_;
    int idxBuildValue_;
    std::vector<std::shared_ptr<Card>> cardlist_;   
private:
    static int idx;    

};

class Mgr
{
public:
    using PCard = std::shared_ptr<Card>;
    int match(const std::map<int, PCard> &hand, std::vector<CardGroup> &grouplist);
    int buildRun(const std::map<int, PCard> &hand, std::vector<CardGroup> &matchlist, std::vector<CardGroup> &candidates);
    static int buildRunFromGroup(const CardGroup &group, std::vector<CardGroup> &matchlist, std::vector<CardGroup> &candidates);
    static void printCardGroup(const CardGroup &group);
    static std::string getCardString(const Card &card);
    static int buildMeld(std::vector<CardGroup> &matchlist, std::vector<CardGroup> &candidates);
    static int buildMeldFromGroup(CardGroup& group, CardGroup &potential);
    static int buildSet(std::vector<CardGroup> &candidates, CardGroup& group, CardGroup &set);
private:

public:
    int special_;
private:
    std::map<int, PCard> hand_; 
};
