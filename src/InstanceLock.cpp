//
// Created by mctrivia on 21/12/24.
//

#include "InstanceLock.h"
#include <iostream>
#include <unistd.h>
#include <csignal>
#include <sys/file.h>
#include <limits.h>
#include <fstream>

int InstanceLock::_lockFileDescriptor = -1;  // Track lock globally for cleanup

// Determine executable path to place the lock file in the same directory
std::string getExecutablePath() {
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count != -1) {
        std::string exePath = std::string(path, count);
        return exePath.substr(0, exePath.find_last_of('/'));
    }
    return ".";
}

// Constructor: Initialize lock file path
InstanceLock::InstanceLock(const std::string &lockName) {
    _lockFilePath = getExecutablePath() + "/" + lockName + ".lock";
}

// Attempt to acquire the lock
bool InstanceLock::acquire() {
    _lockFileDescriptor = open(_lockFilePath.c_str(), O_CREAT | O_RDWR, 0666);
    if (_lockFileDescriptor < 0) {
        std::cerr << "Failed to open or create lock file: " << _lockFilePath << std::endl;
        return false;
    }

    // Read existing PID to check if an instance is already running
    pid_t existingPid = 0;
    std::ifstream inFile(_lockFilePath);
    inFile >> existingPid;
    inFile.close();

    if (existingPid > 0 && kill(existingPid, 0) == 0) {
        std::cerr << "Another instance is already running (PID " << existingPid << ")." << std::endl;
        return false;
    }

    // Try to acquire an exclusive lock
    if (flock(_lockFileDescriptor, LOCK_EX | LOCK_NB) == -1) {
        std::cerr << "Another instance is already running (active lock)." << std::endl;
        return false;
    }

    // Write current PID to the lock file
    ftruncate(_lockFileDescriptor, 0);
    dprintf(_lockFileDescriptor, "%d\n", getpid());

    // Setup cleanup on crash
    setupSignalHandlers();
    return true;
}

// Cleanup lock file on termination signals
void InstanceLock::setupSignalHandlers() {
    signal(SIGINT, cleanupOnSignal);  // Ctrl + C
    signal(SIGTERM, cleanupOnSignal); // Termination signal
    signal(SIGSEGV, cleanupOnSignal); // Segmentation fault
    signal(SIGABRT, cleanupOnSignal); // Abort
}

// Static function to remove lock file on crash
void InstanceLock::cleanupOnSignal(int sig) {
    if (_lockFileDescriptor != -1) {
        unlink(getExecutablePath().append("/digiasset_core.lock").c_str());
    }
    exit(sig);
}

// Destructor: Release lock when the object is destroyed
InstanceLock::~InstanceLock() {
    release();
}

// Manually release lock (optional)
void InstanceLock::release() {
    if (_lockFileDescriptor != -1) {
        unlink(_lockFilePath.c_str());
        close(_lockFileDescriptor);
    }
}
