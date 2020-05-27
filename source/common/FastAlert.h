#pragma once

#include <string>
#include <regex>
/*
05/27-21:38:16.150993  [**] [122:5:1] (portscan) CHG Filtered Portscan [**] [Classification: Attempted Information Leak] [Priority: 2] {PROTO:255} 192.168.0.129 -> 192.168.0.178
05/27-22:23:04.903166  [**] [122:5:1] (portscan) TCP Filtered Portscan [**] [Classification: Attempted Information Leak] [Priority: 2] {PROTO:255} 192.168.0.129 -> 192.168.0.178
*/
struct FastAlert
{
	bool parse(std::string alert)
	{
		// .*\((.*)\) (.*?) \[\*\*\] \[Classification: (.*?)\] \[Priority: (.*?)\]  \{(.*?)\} (.*?) -> (.*)
		std::regex parser(".*\\((.*)\\) (.*?) \\[\\*\\*\\] \\[Classification: (.*?)\\] \\[Priority: (.*?)\\] \\{(.*?)\\} (.*?) -> (.*)");
		std::smatch matches;

		if (!std::regex_match(alert, matches, parser))
		{
			return false;
		}

		int index = 1;
		type = matches[index++];
		info = matches[index++];
		classification = matches[index++];
		priority = std::stoi(
			matches[index++]);
		protocol = matches[index++];
		source = matches[index++];
		target = matches[index++];

		return true;
	}

	std::string type,
		info,
		classification,
		protocol,
		source, 
		target;
	uint8_t priority;
};

inline std::ostream& operator<<(std::ostream& stream, const FastAlert& alert)
{
	stream 
		<< alert.type
		<< alert.info
		<< alert.classification
		<< alert.protocol
		<< alert.source
		<< alert.target
		<< alert.priority;
	return stream;
}

inline std::istream& operator>>(std::istream& stream, FastAlert& alert)
{
	stream
		>> alert.type
		>> alert.info
		>> alert.classification
		>> alert.protocol
		>> alert.source
		>> alert.target
		>> alert.priority;
	return stream;
}
