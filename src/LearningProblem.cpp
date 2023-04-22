#include "LearningProblem.h"
#include "CryptoMiniSatSolver.h"

#include <iostream>

using namespace std;

LearningProblem::LearningProblem() : args(0), count(0) {}

void LearningProblem::add_argument(string arg) {
	int_to_arg.push_back(arg);
	arg_to_int[arg] = args++;
}

void LearningProblem::initialize() {
	attack_var.resize(args);
	for (uint32_t i = 0; i < args; i++) {
		attack_var[i] = ++count;
	}
	constraints.resize(args, std::vector<std::vector<int>>(0));
	attacks.reserve(args*args);
}

void LearningProblem::learn_labeling(Labeling & lab) {
	switch (lab.semantics) {
		case /* constant-expression */:
			/* code */
			break;
	
		default:
			break;
	}
}

void LearningProblem::learn_labeling_cf(Labeling & lab) {
	for (uint32_t a = 0; a < args; a++) {
		vector<int> clause;
		switch (lab.get_label(a))
		{
			case LAB_IN: 
			case LAB_UNDEC:
				for (uint32_t b = 0; b < args; b++) {
					if (lab.get_label(b) == LAB_IN) {
						clause = { -attack_var[b] };
						add_constraint(a, clause);
					}
				}
				break;
			case LAB_OUT:
				for (uint32_t b = 0; b < args; b++) {
					if (lab.get_label(b) == LAB_IN) {
						clause.push_back(attack_var[b]);
					}
				}
				add_constraint(a, clause);
				break;	
		}
	}
}

void LearningProblem::learn_labeling_ad(Labeling & lab) {
	for (uint32_t a = 0; a < args; a++) {
		vector<int> clause;
		switch (lab.get_label(a))
		{
			case LAB_IN: 
				for (uint32_t b = 0; b < args; b++) {
					if (lab.get_label(b) != LAB_OUT) {
						clause = { -attack_var[b] };
						add_constraint(a, clause);
					}
				}
				break;
			case LAB_OUT:
				for (uint32_t b = 0; b < args; b++) {
					if (lab.get_label(b) == LAB_IN) {
						clause.push_back(attack_var[b]);
					}
				}
				add_constraint(a, clause);
				break;	
			case LAB_UNDEC:
				for (uint32_t b = 0; b < args; b++) {
					if (lab.get_label(b) == LAB_IN) {
						clause = { -attack_var[b] };
						add_constraint(a, clause);
					}
				}
				break;
		}
	}
}

void LearningProblem::learn_labeling_co(Labeling & lab) {
	for (uint32_t a = 0; a < args; a++) {
		vector<int> clause;
		switch (lab.get_label(a))
		{
			case LAB_IN: 
				for (uint32_t b = 0; b < args; b++) {
					if (lab.get_label(b) != LAB_OUT) {
						clause = { -attack_var[b] };
						add_constraint(a, clause);
					}
				}
				break;
			case LAB_OUT:
				for (uint32_t b = 0; b < args; b++) {
					if (lab.get_label(b) == LAB_IN) {
						clause.push_back(attack_var[b]);
					}
				}
				add_constraint(a, clause);
				break;	
			case LAB_UNDEC:
				vector<int> undec_clause;
				for (uint32_t b = 0; b < args; b++) {
					if (lab.get_label(b) == LAB_IN) {
						clause = { -attack_var[b] };
						add_constraint(a, clause);
					}
					if (lab.get_label(b) == LAB_UNDEC) {
						undec_clause.push_back(attack_var[b]);
					}
				}
				add_constraint(a, undec_clause);
				break;
		}
	}
}

void LearningProblem::learn_labeling_st(Labeling & lab) {
	for (uint32_t a = 0; a < args; a++) {
		vector<int> clause;
		switch (lab.get_label(a))
		{
			case LAB_IN: 
				for (uint32_t b = 0; b < args; b++) {
					if (lab.get_label(b) == LAB_IN) {
						clause = { -attack_var[b] };
						add_constraint(a, clause);
					}
				}
				break;
			case LAB_OUT:
				for (uint32_t b = 0; b < args; b++) {
					if (lab.get_label(b) == LAB_IN) {
						clause.push_back(attack_var[b]);
					}
				}
				add_constraint(a, clause);
				break;	
		}
	}
}

void LearningProblem::add_constraint(int arg, std::vector<int> & clause) {
    constraints[arg].push_back(clause);
}

void LearningProblem::set_solver_path(string path) {
	solver_path = path;
}

void LearningProblem::solve_problem() {
	for (uint32_t a = 0; a < args; a++) {
		CryptoMiniSatSolver solver = CryptoMiniSatSolver(count, solver_path);
		for (auto clause: constraints[a]) {
			solver.addClause(clause);
		}
		int sat = solver.solve();
		if (!sat) {
			cerr << "No model exists for learned labelings\n";
			return;
		}
		for (uint32_t b = 0; b < args; b++) {
			//cout << b << ": " << solver.model[b] << "\n";
			if (solver.model[attack_var[b]]) {
				attacks.push_back(make_pair(b, a));
			}
		}
		solver.free();
	}
}

void LearningProblem::print_constraint(int arg) {
    for (auto clause: constraints[arg]) {
		for (auto elem: clause) {
			std::cout << elem << " ";
		}
		std::cout << std::endl;
	}
}

void LearningProblem::print_af(std::string format) {
	if (format == "tgf") {
		for (size_t i = 0; i < args; i++) {
			cout << int_to_arg[i] << "\n";
		}
		cout << "#\n";
		for (auto att: attacks) {
			cout << int_to_arg[att.first] << " " << int_to_arg[att.second] << "\n";
		}
	}
	if (format == "apx") {
		for (size_t i = 0; i < args; i++) {
			cout << "arg(" << int_to_arg[i] << ").\n";
		}
		for (auto att: attacks) {
			cout << "att(" << int_to_arg[att.first] << "," << int_to_arg[att.second] << ").\n";
		}
	}	
}