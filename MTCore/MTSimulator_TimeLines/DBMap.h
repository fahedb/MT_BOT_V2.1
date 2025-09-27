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
	std::unordered_map<long long, int>* GetMap() { return &myMap; }
	void InsetIntoMap(long long dateTime, int idx) { myMap.insert(std::make_pair(dateTime, idx)); }
	int GetIdx_Of_TimePoint(long long dateTime);
	bool ContainsKey(long long key) {return myMap.find(key) != myMap.end();}
	void Clear() { myMap.clear(); } // Clear the map
private:
	std::unordered_map<long long, int> myMap;


};

