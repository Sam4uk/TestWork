cmake_minimum_required(VERSION 3.14)

project(TEST VERSION 0.0.0.1)

# Compiler options
add_compile_options(
  -Werror
  -pedantic-errors

  -Wpedantic

  -Wcast-align
  -Wcast-qual
  -Wconversion
  -Wextra-semi
  -Wfloat-equal

  -Wnon-virtual-dtor
  -Wold-style-cast
  -Woverloaded-virtual
  -Wredundant-decls
  -Wsign-promo
  -Wall
  -Wextra  
)


file(WRITE ${PROJECT_BINARY_DIR}/.gitignore [=[
# this .gitignore file
*.*
*
]=])

find_package(spdlog REQUIRED)
include_directories(csv2/include
                    args)

set(PROJECTNAME ${PROJECT_NAME}${PROJECT_VERSION})

add_executable(${PROJECTNAME} main.cpp)
target_compile_features(${PROJECTNAME} PRIVATE cxx_std_17)
set_target_properties(${PROJECTNAME} 
                      PROPERTIES
                      CXX_STANDARD 17
                      CXX_STANDARD_REQUIRED ON)

target_link_libraries(${PROJECTNAME} spdlog::spdlog)
