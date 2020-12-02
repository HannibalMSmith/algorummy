#include <iostream>
#include <fstream>
#include <memory>
#include "json/json.h"
#include "gamerummy.h"
#include "RummyConfig.h"


int main(int argc, char *argv[])
{
    using std::cout;
    using std::endl;
    if (argc < 2) 
    {
        char version[20] = {0};
        cout << argv[0] << " " << Rummy_CONFIG_MAJOR << "." << Rummy_CONFIG_MINOR << endl;
        cout << "Usage: " << argv[0] << " PathToScript" << endl;
        return -1;
    }

    std::ifstream ifs("./config/script.json");
    Json::Reader reader;
    Json::Value root;
    if (!reader.parse(ifs, root))
    {
        cout << "script file format error" <<endl;
        return -1;
    }
    int suit = root["magiccard"]["suit"].asInt();
    int rank = root["magiccard"]["rank"].asInt();

    cout << "magiccard {suit: " << suit << " rank: "<< rank << "}"<<endl;

    int size = root["hand"][0]["card"].size();
    for (auto i = 0; i < size; i++)
    {
        int suit = root["hand"][0]["card"][i]["suit"].asInt();
        int rank = root["hand"][0]["card"][i]["rank"].asInt();
    }
    


    return 0;
}