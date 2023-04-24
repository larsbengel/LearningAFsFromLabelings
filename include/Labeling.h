#include <vector>
#include <string>

enum Semantics { CF, AD, CO, ST, UNKNOWN_SEM};

#define LAB_IN 1
#define LAB_OUT 2
#define LAB_UNDEC 4
#define LAB_UNLABELED -1

class Labeling {
    public:

    Labeling(int args, Semantics semantics);

    std::vector<bool> in;
    std::vector<bool> out;
    Semantics semantics;

    void set_label(int arg, int label);
    int get_label(int arg);
    Semantics get_semantics();
};