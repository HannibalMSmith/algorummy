#pragma once
#include <vector>
#include <memory>
#include <map>

const int c_suitcout = 5;
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
    Card() : suit_(e_joker),rank_(0), bmagic_(false), id_(Card::genId()) {}
    Card(E_SUIT suit, int rank, bool bmagic, int id = Card::genId()) : suit_(suit),rank_(rank), bmagic_(bmagic = false), id_(id){}
    static int genId(){return ++idx;}
    bool operator== (const Card &rhs) {return id_ == rhs.id_;}
public:
    E_SUIT suit_;
    int rank_;
    int id_;
    bool bmagic_;
private:
    static int idx;    
};

class CardGroup
{
public:
    CardGroup() : id_(CardGroup::genId()) {}
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
    void match(const std::map<int, PCard> &hand, std::vector<CardGroup> &grouplist);
    void tryPickRun(const std::map<int, PCard> &hand, std::vector<CardGroup> &matchlist, std::vector<CardGroup> &candidates);
    static void pickRunFromGroup(const CardGroup &group, std::vector<CardGroup> &matchlist);
    static void printCardGroup(CardGroup &group);
private:
    std::map<int, PCard> hand_;  
};
