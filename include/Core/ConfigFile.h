#ifndef ConfigFile_h__
#define ConfigFile_h__

#include <string>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include "ResourceManager.h"

namespace dd
{

class ConfigFile : public Resource
{
	friend class ResourceManager;

private:
	ConfigFile(std::string path);

public:
	template <typename T>
	T GetValue(std::string key, T defaultValue);
	template <typename T>
	void SetValue(std::string key, T value);

	void SaveToDisk();

private:
	boost::filesystem::path m_Path;
	boost::property_tree::ptree m_PTreeDefaults;
	boost::property_tree::ptree m_PTreeOverrides;
	boost::property_tree::ptree m_PTreeMerged;
};

template <typename T>
T ConfigFile::GetValue(std::string key, T defaultValue)
{
	return m_PTreeMerged.get<T>(key, defaultValue);
}

template <typename T>
void ConfigFile::SetValue(std::string key, T value)
{
	m_PTreeOverrides.put<T>(key, value);
	m_PTreeMerged.put<T>(key, value);
}

};
#endif // ConfigFile_h__
