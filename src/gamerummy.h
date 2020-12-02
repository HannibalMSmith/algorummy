#pragma once
#include <vector>

enum E_SUIT
{
    E_HEART=1,
    E_CLUB,
    E_SPADE,
    E_DIAMOND,
    E_JOKER
};

class Card
{
    public:
        Card() : suit_(E_JOKER),rank_(0), id_(Card::GenId()) {};
        Card(E_SUIT suit, int rank, int id = Card::GenId()) : suit_(suit),rank_(rank), id_(id){}
        Card(const Card &rhs) = delete;
        Card& operator=(const Card &rhs) = delete;
        ~Card() = default;
        static int GenId(){return ++idx;}
    public:
        E_SUIT suit_;
        int rank_;
        int id_;
    private:
        static int idx;    
};

class CardGroup
{
    public:
        CardGroup() : id_(CardGroup::GenId()) {}
        
        static int GenId(){return ++idx;}
    public:
        int id_;
        std::vector<Card> cardlist_;    
    private:
        static int idx;    


};