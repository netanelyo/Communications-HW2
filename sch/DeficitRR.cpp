#include "RoundRobin.h"
#include "UtilStructs.h"

#include <sstream>
#include <fstream>


/**
* inserts the packet to the queue of the flow it belongs to
* by checking if its flow has already appeared (meaning it already has a queue).
* If so, we insert the packet at the end of the relevant queue. Otherwise, we create a new queue for the new flow,
* save the quantom the flow should get at every cycle (basic_quantom * weight),
* set its current credit to zero and insert the packet to its queue.
*/
void insertPacketToQueue(const vector<string>& parameters, DrrParams& drrParams, int quantum, int weight)
{
	auto& flowToIndexMap = drrParams.flowToIndexMap;
	auto& flowQuantums = drrParams.flowQuantums;
	auto& flowCredits = drrParams.flowCredits;
	auto& flowPacketQueues = drrParams.flowPacketQueues;

	Flow flow(parameters[2], parameters[4], stoi(parameters[3]), stoi(parameters[5]));
	auto uniqueId = flow.calcUniqueId();
	auto packetIdAndSize = pair<long int, uint16_t>(stoi(parameters[0]), stoi(parameters[6]));

	/*the flow of the packet hasn't appeared yet (meaning the packet belongs to a new flow)*/
	if (flowToIndexMap.find(uniqueId) == flowToIndexMap.end())
	{
		auto tmpList = list<pair<long int, uint16_t>>();
		tmpList.push_front(packetIdAndSize);
		flowPacketQueues.push_back(tmpList);
		flowCredits.push_back(0);
		flowQuantums.push_back((parameters.size() == 8 ? quantum * stoi(parameters[7]) : quantum * weight));
		flowToIndexMap[uniqueId] = flowPacketQueues.size() - 1;
	}
	/*the packet belongs to a flow that we have already encountered (it already has a queue)*/
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

	/*gets first line from input file (gets all the parameters of the first packet)*/
	parameters = getNextLine(inputStream);

	/*performs scheduling until there are no more packets*/
	while (inputStream)
	{
		currentTime = lastSeenPacketTime = stoi(parameters[1]);
		/*gets all the packets that arrived at this time and inserts them to
		the queue of their flow*/
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

		/*sends packets as long as the queues aren't empty.
		when the queues are empty, we "wait" for the arrival of the next packets and continue
		with the same process*/
		do
		{
			emptyCount = 0;
			/*goes over the flows according to their order (first flow we pass is the first flow we encountered and so on)*/
			for (size_t currentFlow = 0; currentFlow < drrParams.flowPacketQueues.size(); currentFlow++)
			{
				/*if the queue of the current flow is empty, we set its credit to zero*/
				if (drrParams.flowPacketQueues[currentFlow].empty())
				{
					drrParams.flowCredits[currentFlow] = 0;
					emptyCount++;
				}
				/*current flow's queue isn't empty*/
				else
				{
					/*we add a quantom to the credit of the current flow (the quantom of the flow: basic_quantom * weight)*/
					drrParams.flowCredits[currentFlow] += drrParams.flowQuantums[currentFlow];

					auto currentPacket = drrParams.flowPacketQueues[currentFlow].back();
					/*we send packets from this queue as long as it has enough credit*/
					while (drrParams.flowCredits[currentFlow] >= currentPacket.second)
					{
						outputStream << currentTime << ": " << currentPacket.first << endl;
					
						drrParams.flowPacketQueues[currentFlow].pop_back();
						drrParams.flowCredits[currentFlow] -= currentPacket.second;
					
						currentTime += currentPacket.second;
						/*after sending a packet, the time changes so 
						  we add to our queues all the packets that arrived 
						  in the time interval (current_time - time_it_took_to_send_packet, current_time)*/
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