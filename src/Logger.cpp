#include "Logger.hpp"


namespace
{
    bool LoggerStarted = false;
}

bool Logger::getLoggerState()
{
    return LoggerStarted;
}
void Logger::start(int hh, int mm)
{
    LoggerStarted = true;
}