#pragma once
#include <vector>
#include <memory>
#include <map>

const int c_suit = 5;
const int c_rank = 13;
const int C_minimum =3;
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
    Card() : suit_(e_joker),rank_(0), joker_(suit_==e_joker), id_(Card::genId()) {}
    Card(E_SUIT suit, int rank, bool joker_ = false, int id = Card::genId()) : suit_(suit),rank_(rank), id_(id){}
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
    void delCard(const PCard &card);
    void reset();
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
public:
    int special_;
private:
    std::map<int, PCard> hand_; 
};
