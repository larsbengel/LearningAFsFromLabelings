#include "ExternalSatSolver.h"

#include <iostream>

#include "pstream.h"

using namespace std;

/*
 * The following is adapted from the fudge argumentation-solver
 * and is subject to the GPL3 licence.
*/
ExternalSatSolver::ExternalSatSolver(uint32_t number_of_vars, string path_to_solver) {
    n_vars = number_of_vars;
    model = std::vector<bool>(n_vars+1);
    clauses = std::vector<std::vector<int>>();

    num_clauses = 0;
    solver_path = path_to_solver;
}

void ExternalSatSolver::addClause(std::vector<int> & clause) {
    clauses.push_back(clause);
    num_clauses++;
    clauses[num_clauses-1].push_back(0);
}

int ExternalSatSolver::solve() {
    redi::pstream process(solver_path, redi::pstreams::pstdout | redi::pstreams::pstdin | redi::pstreams::pstderr);
    //TODO properly implement setting the --polar flag
    //redi::pstream process(solver_path + " --polar false");
    process << "p cnf " << n_vars << " " << num_clauses << "\n";
    //std::cout << "p cnf " << num_vars << " " << num_clauses << "\n";
    for(auto const& clause: clauses) {
        for(const int lit: clause){
            process << lit << " ";
            //std::cout << lit << " ";
        }
        process << "\n";
        //std::cout << "\n";
    }
    
    //std::cout << "-----------------------------------------------" << std::endl;
    process << redi::peof;
    std::string line;
    model.clear();
    //while (std::getline(process.out(),line)) {
    while (process.peek() != EOF && std::getline(process, line)) {
        std::cout << line << std::endl;
        if (line.rfind("c ", 0) == 0) {
            continue;
        }
        if (line.rfind("s ", 0) == 0) {
            if (line.rfind("UNSATISFIABLE") != std::string::npos) {
                return 20;
            }
        }
        if (line.rfind("v ", 0) == 0) {
            line.erase(0, 2);
            size_t pos = 0;
            while(line.length() > 0) {
                pos = line.find(" ");
                if (pos == std::string::npos) {
                    pos = line.length();
                }
                int var = stoi(line.substr(0, pos));
                if (var > 0) {
                    model[var] = true;
                } else if (var < 0) {
                    model[-var] = false;
                } else {
                    break;
                }
                line.erase(0, pos + 1);
            }
        }
    }
    return 10;
}

void ExternalSatSolver::free() {
    clauses.clear();
    model.clear();
}