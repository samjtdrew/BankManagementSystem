cmake_minimum_required(VERSION 3.20)
project(BankManagementSystem C)

set(CMAKE_C_STANDARD 99)

set(source_files
        main.c
        )

add_executable(BankManagementSystem main.c)

#WIN SDK Libraries
target_link_libraries(BankManagementSystem ws2_32)
target_link_libraries(BankManagementSystem C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.19041.0/um/x86/odbc32.lib)
