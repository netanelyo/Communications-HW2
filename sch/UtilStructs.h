#pragma once

#include <string>
#include <map>
#include <vector>
#include <list>

using namespace std;

typedef struct flow_t
{
	string	sourceIP, destIP;
	int		sourcePort, destPort;

	flow_t(string srcIP, string dstIP, int srcPort, int dstPort) : sourceIP(srcIP), destIP(dstIP),
		sourcePort(srcPort), destPort(dstPort) {}

	string calcUniqueId() const
	{
		return sourceIP + ":" + to_string(sourcePort) + ":" + destIP + ":" + to_string(destPort);
	}

} Flow;

typedef struct dataStructuresForDrr {
	map<string, int> flowToIndexMap;
	vector<int> flowQuantums;
	vector<int> flowCredits;
	vector<list<pair<long int, uint16_t>>> flowPacketQueues;
} DrrParams;

typedef struct dataStructuresForWrr {
	map<string, int> flowToIndexMap;
	vector<int> flowWeights;
	vector<list<pair<long int, uint16_t>>> flowPacketQueues;
} WrrParams;