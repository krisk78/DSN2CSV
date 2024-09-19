cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Activez Rechargement à chaud pour les compilateurs MSVC si cela est pris en charge.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Set project name and its dependencies
set(PROJECT_NAME "dsn2csv")
set(${PROJECT_NAME}_DEPENDENCIES ";libconfig;xlnt;consoleapp-static;str-utils-static;file-utils-static")
set(${PROJECT_NAME}_INTERFACES "")

# Get the project version from vcpkg.json
file(READ "${CMAKE_SOURCE_DIR}/vcpkg.json" VCPKG_JSON_CONTENT)
string(REGEX MATCH "\"version\": \"([^\"]+)\"" _ ${VCPKG_JSON_CONTENT})
set(PROJECT_VERSION ${CMAKE_MATCH_1})
# Split into major, minor, patch and pre-release (if any)
string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)(-.+)?" _ ${PROJECT_VERSION})
set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})
set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_2})
set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_3})
set(PROJECT_VERSION_PRERELEASE ${CMAKE_MATCH_4})

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

add_subdirectory(src)
