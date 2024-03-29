#ifndef EXTERNAL_SAT_SOLVER_H
#define EXTERNAL_SAT_SOLVER_H

#include <vector>
#include <string>

/*
Class for all kinds of pre-compiled SAT solvers, e.g. cadical, cryptominisat5
SAT calls are answered by opening a pipe to an instance of the external solver with pstream
TODO some bug in pstream
*/
class ExternalSatSolver {
public:
    std::vector<bool> model;
    std::vector<std::vector<int>> clauses;
    uint32_t n_vars;
    int num_clauses;
    std::string solver_path;

    ExternalSatSolver(uint32_t number_of_vars, std::string path_to_solver);
    void addClause(std::vector<int> & clause);
    int solve();
    void free();
    
};

#endif