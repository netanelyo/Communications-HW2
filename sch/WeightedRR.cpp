#include "RoundRobin.h"
#include "UtilStructs.h"

#include <fstream>

/**
 * inserts the packet to the queue of the flow it belongs to 
 * by checking if its flow has already appeared (meaning it already has a queue).
 * If so, we insert the packet at the end of the relevant queue. Otherwise, we create a new queue for the new flow, 
 * we save the new flow's weight and insert the packet to its queue. 
 */
void insertPacketToQueue(const vector<string>& parameters, WrrParams& wrrParams, int weight)
{
	auto& flowToIndexMap = wrrParams.flowToIndexMap;
	auto& flowWeights = wrrParams.flowWeights;
	auto& flowPacketQueues = wrrParams.flowPacketQueues;

	Flow flow(parameters[2], parameters[4], stoi(parameters[3]), stoi(parameters[5]));
	auto uniqueId = flow.calcUniqueId();
	auto packetIdAndSize = pair<long int, uint16_t>(stoi(parameters[0]), stoi(parameters[6]));
	
	/*the flow of the packet hasn't appeared yet (meaning the packet belongs to a new flow)*/
	if (flowToIndexMap.find(uniqueId) == flowToIndexMap.end())
	{
		auto tmpList = list<pair<long int, uint16_t>>();
		tmpList.push_front(packetIdAndSize);
		flowPacketQueues.push_back(tmpList);
		flowWeights.push_back((parameters.size() == 8 ? stoi(parameters[7]) : weight));
		flowToIndexMap[uniqueId] = flowPacketQueues.size() - 1;
	}
	/*the packet belongs to a flow that we have already encountered (it already has a queue)*/
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
			insertPacketToQueue(parameters, wrrParams, weight);
			
			parameters = getNextLine(inputStream);
			if (parameters.empty())
				lastSeenPacketTime = -1;
			else
				lastSeenPacketTime = stoi(parameters[1]);
		}

		size_t emptyCounter;
		/*sends packets as long as the queues aren't empty.
		  when the queues are empty, we "wait" for the arrival of the next packets and continue
		  with the same process*/
		do
		{
			emptyCounter = 0;
			/*goes over the flows according to their order (first flow we pass is the first flow we encountered and so on)*/
			for (size_t i = 0; i < wrrParams.flowPacketQueues.size(); i++)
			{
				auto packetCounter = 0;
				if (wrrParams.flowPacketQueues[i].empty())
					emptyCounter++;

				/*if the current flow's queue isn't empty, 
				  we send packets from this queue according to the weight of the flow*/
				else
					while (!(wrrParams.flowPacketQueues[i].empty()) && wrrParams.flowWeights[i] > packetCounter)
					{
						auto currentPacket = wrrParams.flowPacketQueues[i].back();

						outputStream << currentTime << ": " << currentPacket.first << endl;

						wrrParams.flowPacketQueues[i].pop_back();
						packetCounter++;

						currentTime += currentPacket.second;
						/* since the time has changed, we add to our queues all the packets that arrived 
						 * in the time interval (current_time - time_it_took_to_send_packet, current_time)
						 */
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
