#ifndef TOOL_INTERFACE_H
#define TOOL_INTERFACE_H

#ifdef BUILD_TOOL
    #define EXPORT_API __declspec(dllexport)
#else
    #define EXPORT_API __declspec(dllimport)
#endif

#include "json.hpp"

class EXPORT_API ITool
{
public:
    virtual ~ITool() = default;
    virtual int run() = 0;
    virtual void toggle(bool on) = 0;

    // virtual void load_data(nlohmann::json& data);
};

#endif // TOOL_INTERFACE_H