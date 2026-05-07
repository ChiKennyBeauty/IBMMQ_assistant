cmake_minimum_required(VERSION 3.16)

project(IBMMQDebugAssistant LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets Test)

set(MQ_SDK_ROOT "${CMAKE_SOURCE_DIR}/IBMMQ_SDK")
set(MQ_INCLUDE_DIR "${MQ_SDK_ROOT}/cplus/include")
set(MQ_C_INCLUDE_DIR "${MQ_SDK_ROOT}/c/include")
set(MQ_LIB_DIR "${MQ_SDK_ROOT}/Lib64")

add_executable(ibmmq_debug_assistant
    src/main.cpp
    src/app/MainWindow.cpp
    src/app/MetaTypeRegistry.cpp
    src/diagnosis/DiagnosisRuleEngine.cpp
    src/mq/MqWorker.cpp
    src/mq/MessageFormatter.cpp
    src/report/ReportBuilder.cpp
    src/session/ReplayService.cpp
    src/session/SessionRecorder.cpp
)

target_include_directories(ibmmq_debug_assistant PRIVATE src)
target_include_directories(ibmmq_debug_assistant PRIVATE "${MQ_INCLUDE_DIR}")
target_include_directories(ibmmq_debug_assistant PRIVATE "${MQ_C_INCLUDE_DIR}")
if(MSVC)
    target_compile_options(ibmmq_debug_assistant PRIVATE /utf-8)
endif()
target_link_directories(ibmmq_debug_assistant PRIVATE "${MQ_LIB_DIR}")
target_link_libraries(ibmmq_debug_assistant PRIVATE Qt5::Widgets imqb23vnvs2019 imqs23vnvs2019 mqic)

enable_testing()

add_executable(connection_config_test
    tests/test_connection_config.cpp
)
target_include_directories(connection_config_test PRIVATE src)
target_link_libraries(connection_config_test PRIVATE Qt5::Test Qt5::Core)
add_test(NAME connection_config_test COMMAND connection_config_test)

add_executable(message_formatter_test
    tests/test_message_formatter.cpp
    src/mq/MessageFormatter.cpp
)
target_include_directories(message_formatter_test PRIVATE src)
target_link_libraries(message_formatter_test PRIVATE Qt5::Core)
add_test(NAME message_formatter_test COMMAND message_formatter_test)

add_executable(metatype_registration_test
    tests/test_metatype_registration.cpp
    src/app/MetaTypeRegistry.cpp
)
target_include_directories(metatype_registration_test PRIVATE src)
target_link_libraries(metatype_registration_test PRIVATE Qt5::Test Qt5::Core)
add_test(NAME metatype_registration_test COMMAND metatype_registration_test)

add_executable(diagnosis_engine_test
    tests/test_diagnosis_engine.cpp
    src/diagnosis/DiagnosisRuleEngine.cpp
)
target_include_directories(diagnosis_engine_test PRIVATE src)
target_link_libraries(diagnosis_engine_test PRIVATE Qt5::Test Qt5::Core)
add_test(NAME diagnosis_engine_test COMMAND diagnosis_engine_test)

add_executable(session_recorder_test
    tests/test_session_recorder.cpp
    src/session/SessionRecorder.cpp
)
target_include_directories(session_recorder_test PRIVATE src)
target_link_libraries(session_recorder_test PRIVATE Qt5::Test Qt5::Core)
add_test(NAME session_recorder_test COMMAND session_recorder_test)

add_executable(report_builder_test
    tests/test_report_builder.cpp
    src/report/ReportBuilder.cpp
)
target_include_directories(report_builder_test PRIVATE src)
target_link_libraries(report_builder_test PRIVATE Qt5::Test Qt5::Core)
add_test(NAME report_builder_test COMMAND report_builder_test)
