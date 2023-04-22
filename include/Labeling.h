#include <vector>
#include <string>

#define LAB_IN 1
#define LAB_OUT 2
#define LAB_UNDEC 4
#define LAB_UNLABELED -1

class Labeling {
    public:

    Labeling(int args, std::string semantics);
    std::vector<bool> in;
    std::vector<bool> out;
    std::string semantics;

    void set_label(int arg, int label);
    int get_label(int arg);
};