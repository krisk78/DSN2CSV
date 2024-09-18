set(${PROJECT_NAME}_LINK_PUBLIC_LIBS "")
set(${PROJECT_NAME}_LINK_INTERFACE_LIBS "")

foreach(_DEP_LIB ${${PROJECT_NAME}_DEPENDENCIES})
    if(UNIX AND _DEP_LIB STREQUAL "xlnt")
        find_package(Xlnt CONFIG REQUIRED)
    else()
        find_package(${_DEP_LIB} CONFIG REQUIRED)
    endif()
    if(_DEP_LIB STREQUAL "libconfig")
        if(MSVC)
            list(APPEND ${PROJECT_NAME}_LINK_PUBLIC_LIBS ${_DEP_LIB}::${_DEP_LIB}++)
        else()
            list(APPEND ${PROJECT_NAME}_LINK_PUBLIC_LIBS ${_DEP_LIB}::config++)
        endif()
    else()
        list(APPEND ${PROJECT_NAME}_LINK_PUBLIC_LIBS ${_DEP_LIB}::${_DEP_LIB})
    endif()
endforeach()

foreach(_DEP_LIB ${${PROJECT_NAME}_INTERFACES})
    find_package(${_DEP_LIB} CONFIG REQUIRED)
    list(APPEND ${PROJECT_NAME}_LINK_INTERFACE_LIBS ${_DEP_LIB}::${_DEP_LIB})
endforeach()

if(${PROJECT_NAME}_STATIC_LINK)
    # this will solve library version issues if the executable is run on old targets but it will increase a lot the executable size (600kB -> 3MB)
    list(APPEND CMAKE_PREFIX_PATH "${VCPKG_ROOT}/installed/x64-linux-static")
endif()

add_executable(${PROJECT_NAME} "${PROJECT_NAME}.cpp" "${PROJECT_NAME}-main.cpp" "configuration.cpp")

# Version
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    PROJECT_VERSION_PRERELEASE="${PROJECT_VERSION_PRERELEASE}"
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_LINK_PUBLIC_LIBS})
target_link_libraries(${PROJECT_NAME} INTERFACE ${${PROJECT_NAME}_LINK_INTERFACE_LIBS})

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#    $<INSTALL_INTERFACE:include>
)

if(${PROJECT_NAME}_STATIC_LINK)
    # this will reduce the executable size of 500kB when linked statically
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_STRIP} "$<TARGET_FILE:${PROJECT_NAME}>"
        COMMENT "Stripping debug symbols from the final binary."
    )
endif()

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}_targets
    PUBLIC_HEADER DESTINATION include
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
