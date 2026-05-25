cmake_minimum_required(VERSION 4.2)
project(FileWatcher C)

set(CMAKE_C_STANDARD 11)

add_executable(FileWatcher
        watcher.c)
