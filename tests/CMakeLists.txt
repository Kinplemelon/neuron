link_directories(/usr/local/lib)

aux_source_directory(${CMAKE_SOURCE_DIR}/src/parser SRC_PARSER)
add_executable(json_test json_test.cc)
target_include_directories(json_test PRIVATE 
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(json_test neuron-base gtest_main gtest pthread jansson)

add_executable(json_rw_test json_rw_test.cc ${SRC_PARSER})
target_include_directories(json_rw_test PRIVATE 
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(json_rw_test neuron-base gtest_main gtest pthread jansson)

add_executable(json_datatag_table_test json_datatag_table_test.cc ${SRC_PARSER})
target_include_directories(json_datatag_table_test PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/include       
)
target_link_libraries(json_datatag_table_test neuron-base gtest_main gtest pthread jansson)

add_executable(json_nodes_test json_nodes_test.cc ${SRC_PARSER})
target_include_directories(json_nodes_test PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/include       
)
target_link_libraries(json_nodes_test neuron-base gtest_main gtest pthread jansson)

add_executable(json_plugin_test json_plugin_test.cc ${SRC_PARSER})
target_include_directories(json_plugin_test PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/include       
)
target_link_libraries(json_plugin_test neuron-base gtest_main gtest pthread jansson)

add_executable(json_group_config_test json_group_config_test.cc ${SRC_PARSER})
target_include_directories(json_group_config_test PRIVATE 
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(json_group_config_test neuron-base gtest_main gtest)

add_executable(variable_test variable_test.cc)
target_include_directories(variable_test PRIVATE 
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(variable_test neuron-base gtest_main gtest)

add_executable(datatag_table_test datatag_table_test.cc ${CMAKE_SOURCE_DIR}/src/base/neu_datatag_table.c)
target_include_directories(datatag_table_test PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/include       
)
target_link_libraries(datatag_table_test neuron-base gtest_main gtest pthread jansson)

add_executable(data_val_test data_val_test.cc ${SRC_PARSER})
target_include_directories(data_val_test PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/include       
)
target_link_libraries(data_val_test neuron-base gtest_main gtest pthread jansson)

add_executable(http_test http_test.cc 
	${CMAKE_SOURCE_DIR}/plugins/restful/http.c)
	
target_include_directories(http_test PRIVATE 
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/plugins/restful)
target_link_libraries(http_test neuron-base gtest_main gtest jansson nng)

#file(COPY ${CMAKE_SOURCE_DIR}/plugin_param_schema.json DESTINATION ${CMAKE_BINARY_DIR}/tests)
add_executable(mqtt_test mqtt_test.cc 
	${CMAKE_SOURCE_DIR}/plugins/mqtt/util.c)
target_include_directories(mqtt_test PRIVATE 
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/include       
	${CMAKE_SOURCE_DIR}/src/schema
	${CMAKE_SOURCE_DIR}/plugins/mqtt)
target_link_libraries(mqtt_test neuron-base gtest_main gtest jansson ssl crypto)

aux_source_directory(${CMAKE_SOURCE_DIR}/src/persist/json SRC_PERSIST_JSON)
add_executable(json_persistence_test json_persistence_test.cc ${SRC_PERSIST_JSON})
target_include_directories(json_persistence_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
	${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(json_persistence_test neuron-base gtest_main gtest)

aux_source_directory(${CMAKE_SOURCE_DIR}/src/persist SRC_PERSIST)
add_executable(persistence_test persistence_test.cc ${SRC_PERSIST_JSON} ${SRC_PERSIST})
target_include_directories(persistence_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(persistence_test neuron-base gtest_main gtest)

file(COPY ${CMAKE_SOURCE_DIR}/neuron.key DESTINATION ${CMAKE_BINARY_DIR}/tests/config)
file(COPY ${CMAKE_SOURCE_DIR}/neuron.pem DESTINATION ${CMAKE_BINARY_DIR}/tests/config)
add_executable(jwt_test jwt_test.cc)
target_include_directories(jwt_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/src/utils)
target_link_libraries(jwt_test neuron-base gtest_main gtest jansson ssl crypto jwt)

add_executable(base64_test base64_test.cc ${CMAKE_SOURCE_DIR}/src/utils/base64.c)
target_include_directories(base64_test PRIVATE 
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/include       
)
target_link_libraries(base64_test neuron-base gtest_main gtest crypto)

aux_source_directory(${CMAKE_SOURCE_DIR}/src/base/tag_sort.c SRC_SORT)
add_executable(tag_sort_test tag_sort_test.cc ${SRC_SORT})
target_include_directories(tag_sort_test PRIVATE 
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(tag_sort_test neuron-base gtest_main gtest)
#target_link_directories(modbus_point_test PRIVATE /usr/local/lib)

include(GoogleTest)
gtest_discover_tests(data_val_test)
gtest_discover_tests(datatag_table_test)
gtest_discover_tests(json_test)
gtest_discover_tests(json_rw_test)
gtest_discover_tests(json_datatag_table_test)
gtest_discover_tests(json_nodes_test)
gtest_discover_tests(json_plugin_test)
gtest_discover_tests(json_group_config_test)
gtest_discover_tests(http_test)
gtest_discover_tests(mqtt_test)
gtest_discover_tests(jwt_test)
gtest_discover_tests(json_persistence_test)
gtest_discover_tests(persistence_test)
gtest_discover_tests(base64_test)
gtest_discover_tests(tag_sort_test)
