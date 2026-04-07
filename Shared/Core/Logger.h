#pragma once
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <fstream>
#include <sstream>
#include <chrono>
#include <format>

enum class LogLevel { DEBUG, INFO, WARN, ERR };

struct LogEntry
{
    LogLevel    level;
    std::string message;
    std::string file;
    int         line;
    std::chrono::system_clock::time_point time;
};

// 비동기 로거 — Log() 호출은 큐 push만 수행, 파일 I/O는 백그라운드 스레드 처리
// 싱글턴으로 전역 접근, LOG_XXX 매크로 사용 권장
class Logger
{
public:
    static Logger& Instance()
    {
        static Logger instance;
        return instance;
    }

    void Log(LogLevel level, const char* file, int line, std::string msg)
    {
        LogEntry entry{ level, std::move(msg), file, line,
                        std::chrono::system_clock::now() };

        {
            std::lock_guard<std::mutex> lock(_mutex);
            _queue.push(std::move(entry));
        }
        _cv.notify_one(); // 백그라운드 스레드 깨움
    }

    void Flush()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _cv.wait(lock, [this] { return _queue.empty(); });
    }

    ~Logger()
    {
        _running = false;
        _cv.notify_all();
        if (_worker.joinable())
            _worker.join();
    }

private:
    Logger()
    {
        // 날짜 기반 로그 파일명
        auto now = std::chrono::system_clock::now();
        auto t   = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        localtime_s(&tm, &t);
        char buf[32];
        std::strftime(buf, sizeof(buf), "server_%Y%m%d.log", &tm);
        _logFile.open(buf, std::ios::app);

        _worker = std::thread(&Logger::Run, this);
    }

    void Run()
    {
        while (_running)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cv.wait(lock, [this] { return !_queue.empty() || !_running; });

            while (!_queue.empty())
            {
                LogEntry entry = std::move(_queue.front());
                _queue.pop();
                lock.unlock();

                Write(entry);

                lock.lock();
            }
        }
    }

    void Write(const LogEntry& entry)
    {
        static const char* levelStr[] = { "DEBUG", "INFO ", "WARN ", "ERROR" };

        auto t  = std::chrono::system_clock::to_time_t(entry.time);
        std::tm tm{};
        localtime_s(&tm, &t);
        char timeBuf[32];
        std::strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &tm);

        // 파일명에서 경로 제거
        const char* fileName = strrchr(entry.file.c_str(), '\\');
        fileName = fileName ? fileName + 1 : entry.file.c_str();

        std::ostringstream oss;
        oss << '[' << timeBuf << "] "
            << '[' << levelStr[static_cast<int>(entry.level)] << "] "
            << '[' << fileName << ':' << entry.line << "] "
            << entry.message << '\n';

        const std::string& line = oss.str();
        std::cout << line;
        if (_logFile.is_open())
            _logFile << line;
    }

    std::queue<LogEntry>    _queue;
    std::mutex              _mutex;
    std::condition_variable _cv;
    std::thread             _worker;
    std::ofstream           _logFile;
    bool                    _running = true;
};

// 사용 매크로 — __FILE__, __LINE__ 자동 삽입
#define LOG_DEBUG(msg) Logger::Instance().Log(LogLevel::DEBUG, __FILE__, __LINE__, msg)
#define LOG_INFO(msg)  Logger::Instance().Log(LogLevel::INFO,  __FILE__, __LINE__, msg)
#define LOG_WARN(msg)  Logger::Instance().Log(LogLevel::WARN,  __FILE__, __LINE__, msg)
#define LOG_ERR(msg)   Logger::Instance().Log(LogLevel::ERR,   __FILE__, __LINE__, msg)
