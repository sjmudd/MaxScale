add_library(MySQLClient SHARED mysql_client.c mysql_common.c)
target_link_libraries(MySQLClient log_manager utils)
install(TARGETS MySQLClient DESTINATION ${MAXSCALE_LIBDIR})

add_library(MySQLBackend SHARED mysql_backend.c mysql_common.c)
target_link_libraries(MySQLBackend log_manager utils)
install(TARGETS MySQLBackend DESTINATION ${MAXSCALE_LIBDIR})

add_library(streamclient SHARED streamclient.c streamcommon.c)
target_link_libraries(streamclient log_manager utils)
install(TARGETS streamclient DESTINATION ${MAXSCALE_LIBDIR})

add_library(streambackend SHARED streambackend.c streamcommon.c)
target_link_libraries(streambackend log_manager utils)
install(TARGETS streambackend DESTINATION ${MAXSCALE_LIBDIR})

add_library(telnetd SHARED telnetd.c)
target_link_libraries(telnetd log_manager utils)
install(TARGETS telnetd DESTINATION ${MAXSCALE_LIBDIR})

add_library(HTTPD SHARED httpd.c)
target_link_libraries(HTTPD log_manager utils)
install(TARGETS HTTPD DESTINATION ${MAXSCALE_LIBDIR})

if(BUILD_TESTS)
  add_library(testprotocol SHARED testprotocol.c)
  install(TARGETS testprotocol DESTINATION ${MAXSCALE_LIBDIR})
endif()

add_library(maxscaled SHARED maxscaled.c)
target_link_libraries(maxscaled log_manager utils)
install(TARGETS maxscaled DESTINATION ${MAXSCALE_LIBDIR})





