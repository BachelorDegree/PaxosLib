cmake_minimum_required(VERSION 3.11...3.16)
project(PaxosLib
    VERSION 0.1
    DESCRIPTION "paxoslib"
    LANGUAGES CXX)
add_compile_definitions(SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_ERROR)
add_subdirectory(src)