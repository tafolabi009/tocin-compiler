if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Tocin")
endif()

message(STATUS "Uninstalling from ${CMAKE_INSTALL_PREFIX}")

file(GLOB_RECURSE INSTALLED_FILES
    "${CMAKE_INSTALL_PREFIX}/bin/*"
    "${CMAKE_INSTALL_PREFIX}/include/tocin/*"
    "${CMAKE_INSTALL_PREFIX}/share/tocin/stdlib/*"
    "${CMAKE_INSTALL_PREFIX}/share/tocin/docs/*"
    "${CMAKE_INSTALL_PREFIX}/doc/*"
)

foreach(file ${INSTALLED_FILES})
    if(EXISTS "${file}")
        file(REMOVE "${file}")
        message(STATUS "Removed: ${file}")
    endif()
endforeach()

# Remove empty directories
file(REMOVE_RECURSE
    "${CMAKE_INSTALL_PREFIX}/bin"
    "${CMAKE_INSTALL_PREFIX}/include/tocin"
    "${CMAKE_INSTALL_PREFIX}/share/tocin/stdlib"
    "${CMAKE_INSTALL_PREFIX}/share/tocin/docs"
    "${CMAKE_INSTALL_PREFIX}/doc"
)
message(STATUS "Uninstall complete.") 
