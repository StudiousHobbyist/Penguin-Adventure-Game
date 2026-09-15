// log.h
#pragma once

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

std::string GetTimeString()
{
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);

    std::tm localTime;
    localtime_s(&localTime, &t);   // Windows

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");

    return oss.str();
}

enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    BLOAT
};

struct TerminalColors {
    std::string reset = "\033[0m";
    std::string red = "\033[31m";
    std::string yellow = "\033[33m";
    std::string blue = "\033[34m";

};

class Logger {
private:
    std::vector<std::string> logs;
    bool bloatEnabled = true;
	bool printToConsole = false;
public:
    void log(LogLevel lvl, std::string msg) {

        auto now = GetTimeString();

        switch (lvl) {
        case LogLevel::INFO:
            if (printToConsole) std::cout << now << " [INFO] " << msg << std::endl;
            logs.push_back(now + " [INFO] " + msg);
            break;
        case LogLevel::WARNING:
            if (printToConsole) std::cout << now << TerminalColors().yellow << " [WARNING] " << msg << TerminalColors().reset << std::endl;
            logs.push_back(now + TerminalColors().yellow + " [WARN] " + msg + TerminalColors().reset);
            break;
        case LogLevel::ERROR:
            if (printToConsole) std::cout << now << TerminalColors().red << " [ERROR] " << msg << TerminalColors().reset << std::endl;
            logs.push_back(now + TerminalColors().red + " [ERR] " + msg + TerminalColors().reset);

            break;

        case LogLevel::BLOAT:
            if (bloatEnabled) {
                if (printToConsole) std::cout << now << TerminalColors().blue << " [BLOAT] " << msg << TerminalColors().reset << std::endl;
                logs.push_back(now + TerminalColors().blue + " [BLOAT] " + msg + TerminalColors().reset);
            }
            break;
        }

    }

    void printLogs() {
        log(LogLevel::INFO, "Printing logs:");
        for (const auto& log : logs) {
            std::cout << log << std::endl;
        }
    }
};

Logger logger{};