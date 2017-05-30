#pragma once

#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <list>

using namespace std;

typedef struct flow_t
{
	string	sourceIP, destIP;
	int		sourcePort, destPort;

	flow_t(string srcIP, string dstIP, int srcPort, int dstPort) : sourceIP(srcIP), destIP(dstIP),
		sourcePort(srcPort), destPort(dstPort) {}

	int calcUniqueId() const
	{
		return ipToInteger(sourceIP) + 2 * ipToInteger(destIP) + 4 * sourcePort + 8 * destPort;
	}

	int ipToInteger(string ip) const
	{
		istringstream strStream(ip);
		string line;
		int ret = 0, tmp;
		for (int i = 24; i >= 0 && getline(strStream, line, '.'); i -= 8)
		{
			tmp = stoi(line);
			ret |= (tmp << i);
		}

		return ret;
	}
} Flow;

typedef struct dataStructuresForDrr {
	unordered_map<int, int> flowToIndexMap;
	vector<int> flowQuantums;
	vector<int> flowCredits;
	vector<list<pair<long int, uint16_t>>> flowPacketQueues;
} DrrParams;

typedef struct dataStructuresForWrr {
	unordered_map<int, int> flowToIndexMap;
	vector<int> flowWeights;
	vector<list<pair<long int, uint16_t>>> flowPacketQueues;
} WrrParams;