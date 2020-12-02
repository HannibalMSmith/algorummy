#include <iostream>
#include <fstream>
#include <memory>
#include "RummyConfig.h"
#include "json/json.h"
#include "gamerummy.h"

int main(int argc, char *argv[])
{
    using std::cout;
    using std::endl;
    if (argc < 2)
    {   
        cout << argv[0] << " " << Rummy_VERSION_MAJOR << "." << Rummy_VERSION_MINOR <<"."\
        << Rummy_VERSION_MAJOR << "." << Rummy_VERSION_MAJOR << endl;
        cout << "Usage: " << argv[0] << " PathToScript(default ./config/script.json)" << endl;

    }

    const char *scriptfile = "./config/script.json";
    if (argc > 1)
    {
        scriptfile = argv[1];
    }

    std::ifstream ifs;
    ifs.open(scriptfile);
    Json::CharReaderBuilder builder;
    builder["collectComments"] = true;
    Json::Value root;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, ifs, &root, &errs)) 
    {
         cout << "script file format error" << endl;
         return 1;
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