project(rosa-central-panel-ui)

find_library(PTHREAD_LIBRARY pthread)
find_package(OpenSSL REQUIRED)

include_directories(include)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wall -Wextra -lcrypto -lssl")

add_executable(${PROJECT_NAME}
    src/DataStorage.cpp
    src/Filter.cpp
    src/Query.cpp
    src/ConfigParser.cpp
    src/LogMaker.cpp
    src/LogLine.cpp
    src/Utils.cpp
    src/DataWorker.cpp
    src/main.cpp
    )

configure_file(./conf.conf ./${PROJECT_NAME}.conf COPYONLY)

target_link_libraries(${PROJECT_NAME} ${PTHREAD_LIBRARY})

#INSTALL(FILES "${CMAKE_BINARY_DIR}/tui/${PROJECT_NAME}" DESTINATION ${CMAKE_INSTALL_SBINDIR} PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
#INSTALL(FILES ${CMAKE_BINARY_DIR}/tui/${PROJECT_NAME}.conf DESTINATION ${CMAKE_INSTALL_SYSCONFDIR} PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
