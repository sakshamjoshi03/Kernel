cmake_minimum_required(VERSION 3.15)
project(WATCH_OS VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /EHsc /utf-8)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite3
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# SQLite3 static library
add_library(sqlite3_lib STATIC
    third_party/sqlite3/sqlite3.c
)
if(NOT MSVC)
    target_compile_definitions(sqlite3_lib PRIVATE -DSQLITE_THREADSAFE=0)
endif()

# Core WatchOS library
set(WATCHOS_SOURCES
    src/core/id_generator.cpp
    src/core/event_queue.cpp
    src/core/simulation_engine.cpp
    src/database/db_connection.cpp
    src/database/schema.cpp
    src/database/session_repository.cpp
    src/database/run_repository.cpp
    src/process/process.cpp
    src/process/workload_validator.cpp
    src/process/lifecycle_simulator.cpp
    src/session/session.cpp
    src/session/session_manager.cpp
    src/session/configuration_manager.cpp
    src/scheduler/scheduling_metrics.cpp
    src/scheduler/gantt_chart.cpp
    src/scheduler/fcfs.cpp
    src/scheduler/sjf.cpp
    src/scheduler/srtf.cpp
    src/scheduler/priority_scheduler.cpp
    src/scheduler/round_robin.cpp
    src/scheduler/recommendation_engine.cpp
    src/scheduler/aging_analyzer.cpp
    src/memory/page_replacement.cpp
    src/memory/memory_manager.cpp
    src/deadlock/resource_manager.cpp
    src/deadlock/bankers_algorithm.cpp
    src/deadlock/deadlock_detector.cpp
    src/deadlock/deadlock_frequency.cpp
    src/io/disk_scheduler.cpp
    src/io/io_manager.cpp
    src/synchronization/sync_manager.cpp
    src/security/access_controller.cpp
    src/experiments/experiment_engine.cpp
    src/experiments/fault_injector.cpp
    src/reporting/jarvis_explainer.cpp
    src/reporting/report_generator.cpp
    src/cli/ansi_terminal.cpp
    src/cli/console_ui.cpp
    src/cli/analysis_menu.cpp
    src/cli/cli_app.cpp
)

add_library(watchos_lib STATIC ${WATCHOS_SOURCES})
target_link_libraries(watchos_lib PUBLIC sqlite3_lib)

# Main Executable
add_executable(watchos src/main.cpp)
target_link_libraries(watchos PRIVATE watchos_lib)

# Enable Testing
enable_testing()

# Unit tests
add_executable(test_id_generator tests/unit/test_id_generator.cpp)
target_link_libraries(test_id_generator PRIVATE watchos_lib)
add_test(NAME TestIdGenerator COMMAND test_id_generator)

add_executable(test_workload_validator tests/unit/test_workload_validator.cpp)
target_link_libraries(test_workload_validator PRIVATE watchos_lib)
add_test(NAME TestWorkloadValidator COMMAND test_workload_validator)

add_executable(test_db_persistence tests/unit/test_db_persistence.cpp)
target_link_libraries(test_db_persistence PRIVATE watchos_lib)
add_test(NAME TestDbPersistence COMMAND test_db_persistence)

add_executable(test_cpu_schedulers tests/unit/test_cpu_schedulers.cpp)
target_link_libraries(test_cpu_schedulers PRIVATE watchos_lib)
add_test(NAME TestCpuSchedulers COMMAND test_cpu_schedulers)

add_executable(test_deadlock_bankers tests/unit/test_deadlock_bankers.cpp)
target_link_libraries(test_deadlock_bankers PRIVATE watchos_lib)
add_test(NAME TestDeadlockBankers COMMAND test_deadlock_bankers)

add_executable(test_memory_paging tests/unit/test_memory_paging.cpp)
target_link_libraries(test_memory_paging PRIVATE watchos_lib)
add_test(NAME TestMemoryPaging COMMAND test_memory_paging)

add_executable(test_disk_scheduling tests/unit/test_disk_scheduling.cpp)
target_link_libraries(test_disk_scheduling PRIVATE watchos_lib)
add_test(NAME TestDiskScheduling COMMAND test_disk_scheduling)

# Integration tests
add_executable(test_session_lifecycle tests/integration/test_session_lifecycle.cpp)
target_link_libraries(test_session_lifecycle PRIVATE watchos_lib)
add_test(NAME TestSessionLifecycle COMMAND test_session_lifecycle)

add_executable(test_complete_kernel_analysis tests/integration/test_complete_kernel_analysis.cpp)
target_link_libraries(test_complete_kernel_analysis PRIVATE watchos_lib)
add_test(NAME TestCompleteKernelAnalysis COMMAND test_complete_kernel_analysis)

# Additional Audit Test Suites
add_executable(test_db_schema_audit tests/unit/test_db_schema_audit.cpp)
target_link_libraries(test_db_schema_audit PRIVATE watchos_lib)
add_test(NAME TestDbSchemaAudit COMMAND test_db_schema_audit)

add_executable(test_edge_cases_schedulers tests/unit/test_edge_cases_schedulers.cpp)
target_link_libraries(test_edge_cases_schedulers PRIVATE watchos_lib)
add_test(NAME TestEdgeCasesSchedulers COMMAND test_edge_cases_schedulers)

add_executable(test_end_to_end_workflows tests/integration/test_end_to_end_workflows.cpp)
target_link_libraries(test_end_to_end_workflows PRIVATE watchos_lib)
add_test(NAME TestEndToEndWorkflows COMMAND test_end_to_end_workflows)

add_executable(test_discovered_regressions tests/regression/test_discovered_regressions.cpp)
target_link_libraries(test_discovered_regressions PRIVATE watchos_lib)
add_test(NAME TestDiscoveredRegressions COMMAND test_discovered_regressions)

