#include "server.h"
#include "common/logger.h"
#include "common/types.h"

#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <cstdlib>
#include <getopt.h>

using namespace android_server;

static Server* g_server = nullptr;

void signalHandler(int signal) {
    if (g_server) {
        g_server->stop();
    }
}

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n"
              << "\n"
              << "Android Native Streaming Server\n"
              << "\n"
              << "Options:\n"
              << "  -p, --port PORT       Server port (default: 8080)\n"
              << "  -w, --width WIDTH     Video width (default: 1280)\n"
              << "  -h, --height HEIGHT   Video height (default: 720)\n"
              << "  -b, --bitrate BITRATE Video bitrate in bps (default: 4000000)\n"
              << "  -f, --fps FPS         Video fps (default: 30)\n"
              << "  -v, --verbose         Enable verbose logging\n"
              << "  -d, --debug           Enable debug logging\n"
              << "  --help                Show this help message\n"
              << "\n"
              << "Examples:\n"
              << "  " << programName << " -p 8080 -w 1920 -h 1080 -b 8000000 -f 60\n"
              << "  " << programName << " --port 9000 --verbose\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    ServerConfig config;
    config.port = 8080;
    config.videoConfig.resolution = Resolution(1280, 720);
    config.videoConfig.bitrate = 4000000;
    config.videoConfig.fps = 30;
    
    bool verbose = false;
    bool debug = false;
    
    printf("Android Server starting...\n");
    fflush(stdout);
    
    static struct option long_options[] = {
        {"port",     required_argument, 0, 'p'},
        {"width",    required_argument, 0, 'w'},
        {"height",   required_argument, 0, 'h'},
        {"bitrate",  required_argument, 0, 'b'},
        {"fps",      required_argument, 0, 'f'},
        {"verbose",  no_argument,       0, 'v'},
        {"debug",    no_argument,       0, 'd'},
        {"help",     no_argument,       0, 0},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    while ((c = getopt_long(argc, argv, "p:w:h:b:f:vd", long_options, &option_index)) != -1) {
        switch (c) {
            case 'p':
                config.port = static_cast<uint16_t>(std::atoi(optarg));
                break;
            case 'w':
                config.videoConfig.resolution.width = static_cast<uint32_t>(std::atoi(optarg));
                break;
            case 'h':
                config.videoConfig.resolution.height = static_cast<uint32_t>(std::atoi(optarg));
                break;
            case 'b':
                config.videoConfig.bitrate = static_cast<uint32_t>(std::atoi(optarg));
                break;
            case 'f':
                config.videoConfig.fps = static_cast<uint32_t>(std::atoi(optarg));
                break;
            case 'v':
                verbose = true;
                break;
            case 'd':
                debug = true;
                break;
            case 0:
                if (strcmp(long_options[option_index].name, "help") == 0) {
                    printUsage(argv[0]);
                    return 0;
                }
                break;
            case '?':
                printUsage(argv[0]);
                return 1;
            default:
                break;
        }
    }
    
    if (debug) {
        Logger::setMinLevel(LogLevel::LOG_DEBUG);
    } else if (verbose) {
        Logger::setMinLevel(LogLevel::LOG_INFO);
    } else {
        Logger::setMinLevel(LogLevel::LOG_WARN);
    }
    
    printf("Configuration: port=%d, resolution=%dx%d, bitrate=%d, fps=%d\n", 
           config.port, config.videoConfig.resolution.width, config.videoConfig.resolution.height,
           config.videoConfig.bitrate, config.videoConfig.fps);
    fflush(stdout);
    
    if (config.port == 0 || config.port > 65535) {
        printf("Error: Invalid port %d\n", config.port);
        fflush(stdout);
        return 1;
    }
    
    if (!config.videoConfig.resolution.isValid()) {
        printf("Error: Invalid resolution %dx%d\n", config.videoConfig.resolution.width, config.videoConfig.resolution.height);
        fflush(stdout);
        return 1;
    }
    
    if (config.videoConfig.fps == 0 || config.videoConfig.fps > 120) {
        printf("Error: Invalid fps %d\n", config.videoConfig.fps);
        fflush(stdout);
        return 1;
    }
    
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGPIPE, SIG_IGN);
    
    printf("Initializing server...\n");
    fflush(stdout);
    
    try {
        Server server(config);
        g_server = &server;
        
        printf("Server created, initializing...\n");
        fflush(stdout);
        
        Result result = server.initialize();
        if (result != Result::SUCCESS) {
            printf("Error: Server initialization failed with result %d\n", static_cast<int>(result));
            fflush(stdout);
            return 1;
        }
        
        printf("Server initialized, starting...\n");
        fflush(stdout);
        
        result = server.start();
        if (result != Result::SUCCESS) {
            printf("Error: Server start failed with result %d\n", static_cast<int>(result));
            fflush(stdout);
            return 1;
        }
        
        printf("Server started successfully, running on port %d\n", config.port);
        fflush(stdout);
        
        while (server.isRunning()) {
            usleep(100000);
        }
        
        printf("Server stopped\n");
        fflush(stdout);
        
    } catch (const std::exception& e) {
        printf("Exception: %s\n", e.what());
        fflush(stdout);
        return 1;
    } catch (...) {
        printf("Unknown exception\n");
        fflush(stdout);
        return 1;
    }
    
    return 0;
} 