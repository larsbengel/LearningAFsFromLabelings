#include "Labeling.h"

using namespace std;

Labeling::Labeling(int args, std::string semantics) {
    in.resize(args);
    out.resize(args);
    semantics = semantics;
}

void Labeling::set_label(int arg, int label) {
    if (label == LAB_IN) {
        in[arg] = true;
        out[arg] = false;
    }
    if (label == LAB_OUT) {
        in[arg] = false;
        out[arg] = true;
    }
    if (label == LAB_UNDEC) {
        in[arg] = false;
        out[arg] = false;
    }
    return;
}

int Labeling::get_label(int arg) {
    if (in[arg] && !out[arg]) {
        return LAB_IN;
    }
    if (!in[arg] && out[arg]) {
        return LAB_OUT;
    }
    if (!in[arg] && !out[arg]) {
        return LAB_UNDEC;
    }
    return LAB_UNLABELED;
}