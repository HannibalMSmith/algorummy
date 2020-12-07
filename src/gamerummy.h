#pragma once
#include <vector>
#include <memory>
#include <map>
#include <string>

const int c_suit = 5;
const int c_rank = 13;
const int c_minimum =3;
enum E_SUIT
{
    e_heart=1,
    e_club,
    e_spade,
    e_diamond,
    e_joker
};

class Card
{
public:
    Card();
    Card(E_SUIT suit, int rank, int goldrank);
    static int genId(){return ++idx;}
    bool special();
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
    CardGroup(int id = genId()) : id_(id){}
    static int genId(){return ++idx;}
    using PCard = std::shared_ptr<Card>;
    void removeCard(const PCard &card);
    void removeCard(int id);
    void removeGroup(const CardGroup &rhs);
    void reset();
    bool restarMatch(CardGroup &matched, std::vector<PCard>::iterator &start);
public:
    int id_;
    std::vector<std::shared_ptr<Card>> cardlist_;   
private:
    static int idx;    

};

class Mgr
{
public:
    using PCard = std::shared_ptr<Card>;
    int match(const std::map<int, PCard> &hand, std::vector<CardGroup> &grouplist);
    int pickRun(const std::map<int, PCard> &hand, std::vector<CardGroup> &matchlist, std::vector<CardGroup> &candidates);
    static int pickRunFromGroup(const CardGroup &group, std::vector<CardGroup> &matchlist, std::vector<CardGroup> &candidates);
    static void printCardGroup(const CardGroup &group);
    static std::string getCardString(const Card &card);
    static int pickHighestMeld(std::vector<CardGroup> &matchlist, std::vector<CardGroup> &candidates);
private:

public:
    int special_;
private:
    std::map<int, PCard> hand_; 
};
