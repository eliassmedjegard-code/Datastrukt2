cmake_minimum_required(VERSION 3.10)
project(sortlab CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Vi defaultar till Release-build eftersom hela poängen är att mäta
# realistiska tider. Med -O0 blir t.ex. quicksort 10-20x långsammare,
# och då säger mätningarna mer om kompilatorn än om algoritmen.
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# -O2 ger bra optimering utan aggressiva tricks som kan dölja
# algoritmens beteende. -DNDEBUG stänger av asserts.
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

add_executable(sortlab
    src/main.cpp
)

target_include_directories(sortlab PRIVATE src)
