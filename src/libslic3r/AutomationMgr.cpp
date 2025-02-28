
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <iostream>
#include <filesystem>
#include <functional>
#include <nlohmann/json.hpp>
#include <Windows.h>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <string>
#include <codecvt>
#include "AutomationMgr.hpp"
#include "libslic3r_version.h"
#include <boost/filesystem.hpp>
#include "Utils.hpp"

namespace Slic3r
{
	AutomationMgr::AutomationType AutomationMgr::g_automationType = AutomationMgr::AutomationType::None;
	std::string AutomationMgr::g_3mfPath = "";

	bool AutomationMgr::enabled()
	{
		return g_automationType != AutomationType::None;
	}

	void AutomationMgr::set3mfPath(const std::string& path)
	{
		g_3mfPath = path;
	}

	void AutomationMgr::setFuncType(int type)
	{
		g_automationType = (AutomationType)type;
	}

	std::string AutomationMgr::get3mfPath()
	{
		return g_3mfPath;
	}

	std::string AutomationMgr::getFileName()
	{
		namespace fs = boost::filesystem;
		fs::path path(g_3mfPath);
		std::string fileName = path.filename().string();
		return fileName;
	}

    void AutomationMgr::outputLog(const std::string& logContent, const int& logType)
	{
        // output log   0: slice log  1: file error log 
        std::string filePath;
        switch (logType) {
			case 0: filePath = Slic3r::data_dir() + "/automation/sliceTime.txt"; break;
			case 1: filePath = Slic3r::data_dir() + "/automation/error.txt"; break;
            case 2: filePath = Slic3r::data_dir() + "/automation/timeout.txt"; break;
	        default: break;
        }
        if (!std::filesystem::exists(filePath)) {
            std::filesystem::create_directories(Slic3r::data_dir() + "/automation");
        }
		std::ofstream log_file(filePath, std::ios_base::app);
		if (!log_file.is_open()) {
            std::cerr << "Failed to open the log" << std::endl;
            return;
		}
		std::string fileName = getFileName();
        std::ostringstream log_stream;
		// output log   0: slice log  1: file error log  2: input file log 3: 
        switch (logType) {
        case 0: 
            log_stream << logContent << std::endl;
			log_file << log_stream.str() << std::endl;
			break;
        case 1:
            log_stream << "Error File: " << fileName << "  " << logContent;
            log_file << log_stream.str() << std::endl;
			break;
        case 2:
            log_stream << "Slice Timeout File: " << fileName << "  " << logContent;
            log_file << log_stream.str() << std::endl;
            break;
        default: break;
        }
	}

    std::string AutomationMgr::getCurrentTime()
    {

        auto        now        = std::chrono::system_clock::now();
        std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

        std::tm* timeinfo = std::localtime(&now_time_t);
        timeinfo->tm_hour;

        if (timeinfo->tm_hour >= 24) {
            timeinfo->tm_hour -= 24;
            timeinfo->tm_mday += 1; 
        }

        std::ostringstream oss;
        oss << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S");
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        oss << "." << std::setfill('0') << std::setw(3) << ms.count(); 

        return oss.str();
    }

    void AutomationMgr::exportGCode(const std::string& target_file)
    {
        namespace fs = std::filesystem;
        // change to wstring due to chinese characters may cause errors 
        std::string _3mf_filename = getFileName();
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::wstring _3mf_w_filename = converter.from_bytes(_3mf_filename);

        fs::path filePath(_3mf_w_filename);
        std::string _g_code_filename = filePath.stem().string() + ".gcode";
        std::string destinationDir = Slic3r::data_dir() + "/automation/gcode";
        if (!fs::exists(destinationDir)) {
            fs::create_directories(destinationDir);
        }
        // copy target file to dir
        fs::copy(target_file, fs::path(destinationDir) / fs::path(target_file).filename(),
                 std::filesystem::copy_options::overwrite_existing);
        // change filename use 3mf filename
        fs::rename(fs::path(destinationDir) / fs::path(target_file).filename(), fs::path(destinationDir) / _g_code_filename);
    }


	void AutomationMgr::endFunction()
	{
		if (g_automationType == AutomationType::GCode)
		{
            HANDLE hprocess = GetCurrentProcess();
            TerminateProcess(hprocess, 1);
		}
	}
}
