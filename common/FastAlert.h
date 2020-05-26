#pragma once

#include <string>
#include <regex>

struct FastAlert
{
	bool parse(std::string alert)
	{
		std::regex parser(".*\((.*)\) (.*?) \[\*\*\] \[Classification: (.*?)\] \[Priority: (.*?)\] {(.*?)} (.*?) -> (.*)");
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

std::ostream& operator<<(std::ostream& stream, const FastAlert& alert)
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

std::istream& operator>>(std::istream& stream, FastAlert& alert)
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
