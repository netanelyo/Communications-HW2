#include "RoundRobin.h"
#include <vector>

using namespace std;



int main(int argc, char** argv)
{
	vector<string> args;

	for (auto i = 2; i < argc; i++)
		args.push_back(argv[i]);

	if (!strcmp(argv[1], "RR"))
		wrrHandler(args[0], args[1], stoi(args[2]));
	else
		drrHandler(args[0], args[1], stoi(args[2]), stoi(args[3]));
}

