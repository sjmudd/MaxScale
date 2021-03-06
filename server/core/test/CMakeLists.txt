execute_process(COMMAND ${CMAKE_COMMAND} -E copy ${ERRMSG} ${CMAKE_CURRENT_BINARY_DIR})
add_executable(test_mysql_users test_mysql_users.c)
add_executable(test_hash testhash.c)
add_executable(test_hint testhint.c)
add_executable(test_spinlock testspinlock.c)
add_executable(test_filter testfilter.c)
add_executable(test_buffer testbuffer.c)
add_executable(test_dcb testdcb.c)
add_executable(test_modutil testmodutil.c)
add_executable(test_poll testpoll.c)
add_executable(test_service testservice.c)
add_executable(test_server testserver.c)
add_executable(test_users testusers.c)
add_executable(test_adminusers testadminusers.c)
add_executable(testmemlog testmemlog.c)
add_executable(testfeedback testfeedback.c)
target_link_libraries(test_mysql_users MySQLClient fullcore)
target_link_libraries(test_hash fullcore log_manager)
target_link_libraries(test_hint fullcore log_manager)
target_link_libraries(test_spinlock fullcore log_manager)
target_link_libraries(test_filter fullcore)
target_link_libraries(test_buffer fullcore log_manager)
target_link_libraries(test_dcb fullcore)
target_link_libraries(test_modutil fullcore utils log_manager)
target_link_libraries(test_poll fullcore)
target_link_libraries(test_service fullcore)
target_link_libraries(test_server fullcore)
target_link_libraries(test_users fullcore)
target_link_libraries(test_adminusers fullcore)
target_link_libraries(testmemlog fullcore log_manager)
target_link_libraries(testfeedback fullcore)
add_test(Internal-TestMySQLUsers test_mysql_users)
add_test(Internal-TestHash test_hash)
add_test(Internal-TestHint test_hint)
add_test(Internal-TestSpinlock test_spinlock)
add_test(Internal-TestFilter test_filter)
add_test(Internal-TestBuffer test_buffer)
add_test(Internal-TestDCB test_dcb)
add_test(Internal-TestModutil test_modutil)
add_test(Internal-TestPoll test_poll)
add_test(Internal-TestService test_service)
add_test(Internal-TestServer test_server)
add_test(Internal-TestUsers test_users)
add_test(Internal-TestAdminUsers test_adminusers)
add_test(Internal-TestMemlog testmemlog)
add_test(TestFeedback testfeedback)
set_tests_properties(TestFeedback PROPERTIES TIMEOUT 30)
