#ifndef TOOL_INTERFACE_H
#define TOOL_INTERFACE_H

// g++ -shared -o ToolA.dll ToolA.cpp

class ITool
{
public:
    virtual ~ITool() = default;
    virtual void execute() = 0;
};

// Factory function to create an instance of a tool
extern "C" __declspec(dllexport) ITool* create_tool();

#endif // TOOL_INTERFACE_H