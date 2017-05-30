#include "RoundRobin.h"
#include "UtilStructs.h"

#include <fstream>

void insertPacketToQueue(const vector<string>& parameters, DrrParams& drrParams, int quantum, int weight)
{
	auto& flowToIndexMap = drrParams.flowToIndexMap;
	auto& flowQuantums = drrParams.flowQuantums;
	auto& flowCredits = drrParams.flowCredits;
	auto& flowPacketQueues = drrParams.flowPacketQueues;

	Flow flow(parameters[2], parameters[4], stoi(parameters[3]), stoi(parameters[5]));
	auto uniqueId = flow.calcUniqueId();
	auto packetIdAndSize = pair<long int, uint16_t>(stoi(parameters[0]), stoi(parameters[6]));

	if (flowToIndexMap.find(uniqueId) == flowToIndexMap.end())
	{
		auto tmpList = list<pair<long int, uint16_t>>();
		tmpList.push_front(packetIdAndSize);
		flowPacketQueues.push_back(tmpList);
		flowCredits.push_back(0);
		flowQuantums.push_back((parameters.size() == 8 ? quantum * stoi(parameters[7]) : quantum * weight));
		flowToIndexMap[uniqueId] = flowPacketQueues.size() - 1;
	}
	else
	{
		flowPacketQueues[flowToIndexMap[uniqueId]].push_front(packetIdAndSize);
	}
}

vector<string> getNextLine(ifstream& inputStream)
{
	string line, parameter;
	istringstream strStream;
	vector<string> parameters;

	getline(inputStream, line);
	strStream = istringstream(line);

	while (getline(strStream, parameter, ' '))
	{
		parameters.push_back(parameter);
	}

	return parameters;
}

void drrHandler(string inputFile, string outputFile, int weight, int quantum)
{
	DrrParams drrParams;
	vector<string> parameters;
	long int currentTime, lastSeenPacketTime;

	ifstream inputStream(inputFile);
	ofstream outputStream(outputFile, ios::trunc | ios::out);

	parameters = getNextLine(inputStream);

	while (inputStream)
	{
		currentTime = lastSeenPacketTime = stoi(parameters[1]);

		while (currentTime == lastSeenPacketTime)
		{
			insertPacketToQueue(parameters, drrParams, quantum, weight);

			parameters = getNextLine(inputStream);
			if (parameters.empty())
				lastSeenPacketTime = -1;
			else
				lastSeenPacketTime = stoi(parameters[1]);
		}

		size_t emptyCount;

		do
		{
			emptyCount = 0;

			for (size_t currentFlow = 0; currentFlow < drrParams.flowPacketQueues.size(); currentFlow++)
			{
				if (drrParams.flowPacketQueues[currentFlow].empty())
				{
					drrParams.flowCredits[currentFlow] = 0;
					emptyCount++;
				}
				else
				{
					drrParams.flowCredits[currentFlow] += drrParams.flowQuantums[currentFlow];

					auto currentPacket = drrParams.flowPacketQueues[currentFlow].back();

					while (drrParams.flowCredits[currentFlow] >= currentPacket.second)
					{
						outputStream << currentTime << ": " << currentPacket.first << endl;
					
						drrParams.flowPacketQueues[currentFlow].pop_back();
						drrParams.flowCredits[currentFlow] -= currentPacket.second;
					
						currentTime += currentPacket.second;

						while (lastSeenPacketTime <= currentTime && lastSeenPacketTime != -1)
						{
							insertPacketToQueue(parameters, drrParams, quantum, weight);

							parameters = getNextLine(inputStream);
							if (parameters.empty())
								lastSeenPacketTime = -1;
							else
								lastSeenPacketTime = stoi(parameters[1]);
						}

						if (drrParams.flowPacketQueues[currentFlow].empty())
							break;

						currentPacket = drrParams.flowPacketQueues[currentFlow].back();

					}
				}
			}
		} while (emptyCount < drrParams.flowPacketQueues.size());
	}
}