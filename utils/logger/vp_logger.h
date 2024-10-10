#pragma once

#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <chrono>
#include <assert.h>
#include <map>

#include "../vp_semaphore.h"
#include "../vp_utils.h"
#include "vp_log_file_writer.h"

namespace vp_utils {
    // log levels
    enum vp_log_level {
        ERROR = 1,
        WARN = 2,
        INFO = 3,
        DEBUG = 4
    };

    // a lightweight logger for VideoPipe, architecture: N producer * 1 consumer.
    // 1. support 3 types of devices (console, file, kafka)
    // 2. multithread safe
    // 3. use Macros directly
    class vp_logger
    {
    private:
        std::queue<std::string> log_cache;
        vp_utils::vp_semaphore log_cache_semaphore;
        std::mutex log_cache_mutex;
        std::thread log_writer_th;

        // initialized or not
        bool inited = false;

        vp_logger(/* args */);

        // write to devices
        void write_to_console(const std::string& log);
        void write_to_file(const std::string& log);
        void write_to_kafka(const std::string& log);

        // writing log func
        void log_write_run();

        // file writer
        vp_utils::vp_log_file_writer file_writer;

        const std::map<vp_log_level, std::string> log_level_names = {{ERROR, "Error"},
                                                                    {WARN, "Warn "},
                                                                    {INFO, "Info "},
                                                                    {DEBUG, "Debug"}};
    public:
        // non-copable
        vp_logger(const vp_logger&) = delete;
        vp_logger& operator=(const vp_logger&) = delete;

        // singleton
        static vp_logger& get_logger() {
            static vp_logger logger;
            return logger;
        }
        ~vp_logger();

        // CONFIG
        vp_log_level log_level = vp_log_level::DEBUG; // filter
        std::string log_dir = "./log";                // folder saving log file
        const std::string log_file_name_template = "<year>-<mon>-<day>.txt";
        const std::string log_time_templete = "[<year>-<mon>-<day> <hour>:<min>:<sec>.<mili>]";

        // watch
        int log_cache_warn_threshold = 100;           // warning if cache size greater than threshold
        
        // where
        bool log_to_console = true;                   // to console
        bool log_to_file = true;                      // to file
        bool log_to_kafka = false;                    // to kafka

        // how
        const bool include_time = true;               // `log time` part in log content (readonly)
        bool include_level = true;                    // `log level` part in log content
        bool include_code_location = true;            // `log from` part in log content
        bool include_thread_id = true;                // 'thread id' part in log content
        // END of CONFIG

        // better Never call directly
        void log(vp_log_level level, const std::string& message, const char* code_file, int code_line);

        // init for vp_logger, ready to go
        void init();
    };

    // config Macros
    #define VP_SET_LOG_LEVEL(_log_level) vp_utils::vp_logger::get_logger().log_level = _log_level
    #define VP_SET_LOG_DIR(_log_dir) vp_utils::vp_logger::get_logger().log_dir = _log_dir
    #define VP_SET_LOG_TO_CONSOLE(_log_to_console) vp_utils::vp_logger::get_logger().log_to_console = _log_to_console
    #define VP_SET_LOG_TO_FILE(_log_to_file) vp_utils::vp_logger::get_logger().log_to_console = _log_to_file
    #define VP_SET_LOG_TO_KAFKA(_log_to_kafka) vp_utils::vp_logger::get_logger().log_to_kafka = _log_to_kafka
    #define VP_SET_LOG_INCLUDE_LEVEL(_include_level) vp_utils::vp_logger::get_logger().include_level = _include_level
    #define VP_SET_LOG_INCLUDE_CODE_LOCATION(_include_code_location) vp_utils::vp_logger::get_logger().include_code_location = _include_code_location
    #define VP_SET_LOG_INCLUDE_THREAD_ID(_include_thread_id) vp_utils::vp_logger::get_logger().include_thread_id = _include_thread_id
    #define VP_SET_LOG_CACHE_WARN_THRES(_log_cache_warn_threshold) vp_utils::vp_logger::get_logger().log_cache_warn_threshold = _log_cache_warn_threshold

    // log Macros
    // use vp_utils::string_format to format log content first if need
    // example: 
    // VP_ERROR(vp_utils::string_format("message is %s at %d", s, d));
    #define VP_ERROR(message) vp_utils::vp_logger::get_logger().log(vp_utils::vp_log_level::ERROR, message, __FILE__, __LINE__)
    #define VP_WARN(message) vp_utils::vp_logger::get_logger().log(vp_utils::vp_log_level::WARN, message, __FILE__, __LINE__)
    #define VP_INFO(message) vp_utils::vp_logger::get_logger().log(vp_utils::vp_log_level::INFO, message, __FILE__, __LINE__)
    #define VP_DEBUG(message) vp_utils::vp_logger::get_logger().log(vp_utils::vp_log_level::DEBUG, message, __FILE__, __LINE__)

    // init Macros
    #define VP_LOGGER_INIT() vp_utils::vp_logger::get_logger().init()

    #define NOW std::chrono::system_clock::now()
}