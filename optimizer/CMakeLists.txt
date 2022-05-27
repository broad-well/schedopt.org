cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 17)
project(schedopt-optimizer LANGUAGES CXX)
find_package(aws-lambda-runtime CONFIG REQUIRED)
find_package(doctest CONFIG REQUIRED)
find_package(Boost CONFIG REQUIRED)

#add_executable(main "main.cpp")
#target_link_libraries(main PRIVATE AWS::aws-lambda-runtime)
#target_compile_features(main PRIVATE "cxx_std_17")
#target_compile_options(main PRIVATE "-Wall" "-Wextra")
#
#aws_lambda_package_target(main)

add_executable(test_validator "test_validators.cpp")
target_link_libraries(test_validator PRIVATE doctest::doctest Boost::boost)

add_executable(test_requirements "test_requirements.cpp")
target_link_libraries(test_requirements PRIVATE doctest::doctest Boost::boost)