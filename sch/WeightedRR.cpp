#include "RoundRobin.h"
#include "UtilStructs.h"

#include <fstream>

void insertPacketToQueue(const vector<string>& parameters, WrrParams& wrrParams, int weight)
{
	auto& flowToIndexMap = wrrParams.flowToIndexMap;
	auto& flowWeights = wrrParams.flowWeights;
	auto& flowPacketQueues = wrrParams.flowPacketQueues;

	Flow flow(parameters[2], parameters[4], stoi(parameters[3]), stoi(parameters[5]));
	auto uniqueId = flow.calcUniqueId();
	auto packetIdAndSize = pair<long int, uint16_t>(stoi(parameters[0]), stoi(parameters[6]));

	if (flowToIndexMap.find(uniqueId) == flowToIndexMap.end())
	{
		auto tmpList = list<pair<long int, uint16_t>>();
		tmpList.push_front(packetIdAndSize);
		flowPacketQueues.push_back(tmpList);
		flowWeights.push_back((parameters.size() == 8 ? stoi(parameters[7]) : weight));
		flowToIndexMap[uniqueId] = flowPacketQueues.size() - 1;
	}
	else
	{
		flowPacketQueues[flowToIndexMap[uniqueId]].push_front(packetIdAndSize);
	}
}

void wrrHandler(string inputFile, string outputFile, int weight)
{
	WrrParams wrrParams;
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
			insertPacketToQueue(parameters, wrrParams, weight);

			parameters = getNextLine(inputStream);
			if (parameters.empty())
				lastSeenPacketTime = -1;
			else
				lastSeenPacketTime = stoi(parameters[1]);
		}

		size_t emptyCounter;

		do
		{
			emptyCounter = 0;

			for (size_t i = 0; i < wrrParams.flowPacketQueues.size(); i++)
			{
				auto packetCounter = 0;
				if (wrrParams.flowPacketQueues[i].empty())
					emptyCounter++;

				else
					while (!(wrrParams.flowPacketQueues[i].empty()) && wrrParams.flowWeights[i] > packetCounter)
					{
						auto currentPacket = wrrParams.flowPacketQueues[i].back();

						outputStream << currentTime << ": " << currentPacket.first << endl;

						wrrParams.flowPacketQueues[i].pop_back();
						packetCounter++;

						currentTime += currentPacket.second;

						while (lastSeenPacketTime <= currentTime && lastSeenPacketTime != -1)
						{
							insertPacketToQueue(parameters, wrrParams, weight);

							parameters = getNextLine(inputStream);
							if (parameters.empty())
								lastSeenPacketTime = -1;
							else
								lastSeenPacketTime = stoi(parameters[1]);
						}
					}
			}
		}
		while (emptyCounter < wrrParams.flowPacketQueues.size());

	}
}
