#pragma once

#include <fstream>
#include <vector>
#include <string>

static void DumpToFile(std::string filename, void* data, size_t size)
{
	std::ofstream out(filename, std::ios::binary);
	out.write((char*)data, size);
}

static void AddToFile(std::string filename, void* data, size_t size)
{
	std::ofstream out(filename, std::ios::binary | std::ios::app | std::ios::ate);
	out.write((char*)data, size);
}

template <typename T>
static void DumpToFile(std::string filename, std::vector<T> data)
{
	DumpToFile(filename, data.data(), data.size() * sizeof(T));
}

template <typename T>
static void AddToFile(std::string filename, std::vector<T> data)
{
	AddToFile(filename, data.data(), data.size() * sizeof(T));
}
