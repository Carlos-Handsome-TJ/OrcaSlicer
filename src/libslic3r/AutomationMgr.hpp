#ifndef _AUTOMATIONMGR_H
#define _AUTOMATIONMGR_H
#include <string>
#include <vector>
namespace Slic3r 
{
	class AutomationMgr
	{
		enum AutomationType
		{
			None = 0,
			GCode,
		};
	public:
		static std::string g_3mfPath;
		static AutomationType g_automationType;
	public:
		static bool enabled();
		static void set3mfPath(const std::string& path);
		static void setFuncType(int type);
		static std::string get3mfPath();
		static void endFunction();
		static std::string getFileName();
        static void outputLog(const std::string& logContent, const int& logType);
        static std::string getCurrentTime();
        static void        exportGCode(const std::string& target_file);

	};
    template<typename Func> void calculateTimeout(Func func, std::chrono::milliseconds timeout)
        { 
            auto future = std::async(std::launch::async, func);

            if (future.wait_for(timeout) == std::future_status::timeout) {
                string logContent = "";
                AutomationMgr::outputLog(logContent, 2);
                AutomationMgr::endFunction();
            }

            future.get();
        }
    }
#endif // _AUTOMATIONMGR_H
