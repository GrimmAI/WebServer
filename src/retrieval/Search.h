#pragma once
#include <json/json.h>
#include <vector>
#include <string>

class Search {
private:
    Json::Value id_map_;
public:
    Search();
    ~Search();
    
    std::vector<float> GetUserQueryTextEmbedding(std::string &);
    std::vector<std::string> GetTopKResults(std::vector<float>, int);
    float CalcSim(std::vector<float> &, std::vector<float> &);
};

