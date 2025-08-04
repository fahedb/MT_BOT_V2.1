#include <iostream>
#include <unordered_map>
#include <string>

#pragma once
class DBMap
{
public:
	~DBMap() { myMap.clear(); }
	DBMap() {}
	DBMap(const DBMap& other)
	{
		myMap = other.myMap; // Copy the unordered_map
	}
public:
	std::unordered_map<std::string, int>* GetMap() { return &myMap; }
	void InsetIntoMap(const std::string& dateTime, int idx) { myMap.insert(std::make_pair(dateTime, idx)); }
	int GetIdx_Of_TimePoint(const std::string& dateTime);
	bool ContainsKey(const std::string& key) {return myMap.find(key) != myMap.end();}
	void Clear() { myMap.clear(); } // Clear the map
private:
	std::unordered_map<std::string, int> myMap;


};

