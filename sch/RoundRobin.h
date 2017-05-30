#pragma once

#include <string>
#include <vector>

using namespace std;

void drrHandler(string inputFile, string outputFile, int weight, int quantum);

void wrrHandler(string inputFile, string outputFile, int weight);

vector<string> getNextLine(ifstream& inputStream);