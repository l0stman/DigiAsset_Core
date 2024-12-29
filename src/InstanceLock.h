//
// Created by mctrivia on 21/12/24.
//

#ifndef INSTANCELOCK_H
#define INSTANCELOCK_H

#include <string>

class InstanceLock {
public:
    explicit InstanceLock(const std::string &lockName);
    ~InstanceLock();

    // Attempts to acquire the lock, returns true if successful
    bool acquire();

    // Releases the lock (optional, auto-cleans on exit)
    void release();

private:
    std::string _lockFilePath;
    static int _lockFileDescriptor;
    void setupSignalHandlers();
    static void cleanupOnSignal(int sig);
};

#endif // INSTANCELOCK_H
