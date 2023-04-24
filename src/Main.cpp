#include <iostream>			//std::cout
#include <fstream>			//ifstream
#include <sstream>
#include <algorithm>
#include <stack>

#include "LearningProblem.h"

#include <getopt.h>			// parsing commandline options

using namespace std;

static int version_flag = 0;
static int usage_flag = 0;
static int formats_flag = 0;

Semantics string_to_sem(string semantics) {
	if (semantics == "CF") return CF;
	if (semantics == "ADM") return AD;
	if (semantics == "CO") return CO;
	if (semantics == "ST") return ST;
	return UNKNOWN_SEM;
}

void print_usage(string solver_name) {
	cout << "Usage: " << solver_name << " -a <arguments> -f <file> -fo <format> \n\n";
	cout << "  <arguments> input list of arguments\n";
	cout << "  <file>      input list of labelings\n";
	cout << "  <format>    file format for input argument list; for a list of available formats use option --formats\n";
	cout << "Options:\n";
	cout << "  --help      Displays this help message.\n";
	cout << "  --version   Prints version and author information.\n";
	cout << "  --formats   Prints available file formats.\n";
}

void print_version(string solver_name) {
	cout << solver_name << " (version 1.0)\n" << "Lars Bengel, University of Hagen <lars.bengel@fernuni-hagen.de>\n";
}

void print_formats() {
	cout << "[apx,tgf]\n";
}

int main(int argc, char ** argv) {
	ios_base::sync_with_stdio(false);
	cin.tie(NULL);

	if (argc == 1) {
		print_version(argv[0]);
		return 0;
	}

	const struct option longopts[] =
	{
		{"help", no_argument, &usage_flag, 1},
		{"version", no_argument, &version_flag, 1},
		{"formats", no_argument, &formats_flag, 1},
		{"a", required_argument, 0, 'a'},
		{"f", required_argument, 0, 'f'},
		{"fo", required_argument, 0, 'o'},
		{"s", required_argument, 0, 's'},
		{0, 0, 0, 0}
	};

	int option_index = 0;
	int opt = 0;
	string arguments, file, fileformat, sat_path;

	while ((opt = getopt_long_only(argc, argv, "", longopts, &option_index)) != -1) {
		switch (opt) {
			case 0:
				break;
			case 'a':
				arguments = optarg;
				break;
			case 'f':
				file = optarg;
				break;
			case 'o':
				fileformat = optarg;
				break;
			case 's':
				sat_path = optarg;
				break;
			default:
				return 1;
		}
	}

	if (version_flag) {
		print_version(argv[0]);
		return 0;
	}

	if (usage_flag) {
		print_usage(argv[0]);
		return 0;
	}

	if (formats_flag) {
		print_formats();
		return 0;
	}

	if (arguments.empty()) {
		cerr << argv[0] << ": Argument list must be specified via -a flag\n";
		return 1;
	}

	if (file.empty()) {
		cerr << argv[0] << ": Input file must be specified via -f flag\n";
		return 1;
	}

	if (fileformat.empty()) {
		cerr << argv[0] << ": File format must be specified via -fo flag\n";
		return 1;
	}

	ifstream input_arguments;
	input_arguments.open(arguments);

	if (!input_arguments.good()) {
		cerr << argv[0] << ": Cannot open arguments file\n";
		return 1;
	}

	ifstream input_labelings;
	input_labelings.open(file);

	if (!input_labelings.good()) {
		cerr << argv[0] << ": Cannot open input file\n";
		return 1;
	}

	LearningProblem af = LearningProblem();
	string line, arg;
	
	if (fileformat == "apx") {
		while (!input_arguments.eof()) {
			getline(input_arguments, line);
			line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
			if (line.length() == 0 || line[0] == '/' || line[0] == '%') continue;
			if (line.length() < 6) cerr << "Warning: Cannot parse line: " << line << "\n";
			string op = line.substr(0,3);
			if (op == "arg") {
				if (line[3] == '(' && line.find(')') != string::npos) {
					arg = line.substr(4,line.find(')')-4);
					af.add_argument(arg);
				} else {
					cerr << "Warning: Cannot parse line: " << line << "\n";
				}
			} else if (op == "att") {
				break;
			} else {
				cerr << "Warning: Cannot parse line: " << line << "\n";
			}
		}
	} else if (fileformat == "tgf") {
		while (input_arguments >> arg) {
			if (arg == "#") break;
			af.add_argument(arg);
		}
	} else {
		cerr << argv[0] << ": Unsupported file format\n";
		return 1;
	}

	input_arguments.close();

	vector<Labeling> labelings;
	labelings.reserve(1000);

	while(!input_labelings.eof()) {
		getline(input_labelings, line);
		line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
		if (line.length() == 0 || line[0] == '/' || line[0] == '%') continue;
		int endSemantics = line.find(":");
		string semantics = line.substr(0, endSemantics);
		Labeling lab = Labeling(af.args, string_to_sem(semantics));
		string subs = line.substr(endSemantics+2, line.length()-endSemantics-3);
		int endIn = subs.find("],[");
		int endOut = subs.find("],[", endIn+1);
		stringstream strIn(subs.substr(1, endIn-1));
		stringstream strOut(subs.substr(endIn+3, endOut-endIn-3));
		stringstream strUnd(subs.substr(endOut+3, subs.length()-endOut-4));
		string arg;
		while (strIn.good()) {
			getline(strIn, arg, ',');
			if (arg == "") {
				break;
			}
			lab.set_label(af.arg_to_int.find(arg)->second, LAB_IN);
		}
		while (strOut.good()) {
			getline(strOut, arg, ',');
			if (arg == "") {
				break;
			}
			lab.set_label(af.arg_to_int.find(arg)->second, LAB_OUT);
		}
		while (strUnd.good()) {
			getline(strUnd, arg, ',');
			if (arg == "") {
				break;
			}
			lab.set_label(af.arg_to_int.find(arg)->second, LAB_UNDEC);
		}
		labelings.push_back(lab);
	}

	input_labelings.close();

	//%% Learning %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	af.initialize();
	af.set_solver_path(sat_path);

	// iterate over labelings
	for(auto lab: labelings) {
		// learn each labeling
		af.learn_labeling(lab);
	}

	af.solve_problem();
	af.print_af(fileformat);

	return 0;
}