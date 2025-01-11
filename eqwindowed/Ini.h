#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <map>
#include <stdexcept>
#include <Windows.h>
#include <Vector>
namespace INI
{
    template<typename T>
    T convertFromString(const std::string& str) {

        if constexpr (std::is_same_v<T, bool>) {
            if (str == "TRUE")
                return true;
            else
                return false;
        }
        std::istringstream iss(str);
        T value;
        iss >> std::boolalpha >> value;
        return value;
    }

    static inline bool exists(const std::string& section, const std::string& key, const char* filename) {
        char buffer[256];
        DWORD bytesRead = GetPrivateProfileStringA(section.c_str(), key.c_str(), "", buffer, sizeof(buffer), filename);

        if (bytesRead == 0) {
            return false;
        }
        return true;
    }

    static inline std::vector<std::string> getSectionNames(const char* filename) {
        std::vector<std::string> sectionNames;
        const DWORD bufferSize = 4096; // Adjust buffer size as needed
        char buffer[bufferSize];

        DWORD result = GetPrivateProfileSectionNamesA(buffer, bufferSize, filename);
        if (result == 0) {
            return sectionNames;
        }

        for (char* p = buffer; *p != '\0'; p += strlen(p) + 1) {
            sectionNames.push_back(p);
        }

        return sectionNames;
    }

    static inline  bool deleteSection(const std::string& sectionName, const char* filename) {
        // Delete the section and its contents by writing an empty string to it
        if (!WritePrivateProfileSectionA(sectionName.c_str(), nullptr, filename))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    template<typename T>
    T getValue(std::string section, std::string key, const char* filename) {
        char buffer[256];
        DWORD bytesRead = GetPrivateProfileStringA(section.c_str(), key.c_str(), "", buffer, sizeof(buffer), filename);

        if (bytesRead == 0) {
            return T{};
        }
        if constexpr (std::is_same_v<T, std::string>)
            return buffer;
        return convertFromString<T>(std::string(buffer));
    }

    template<typename T>
    void setValue(const std::string& section, const std::string& key, const T& value, const char* filename)
    {
        std::string valueStr;
        if constexpr (std::is_same_v<T, bool>)
        {
            valueStr = value ? "TRUE" : "FALSE";
        }
        else if constexpr (!std::is_same_v<T, std::string>)
        {
            valueStr = std::to_string(value);
        }
        else
        {
            valueStr = value;
        }
        BOOL result = WritePrivateProfileStringA(section.c_str(), key.c_str(), valueStr.c_str(), filename);
    }
}
