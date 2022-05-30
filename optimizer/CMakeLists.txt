cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 17)
project(schedopt-optimizer LANGUAGES CXX)
# find_package(aws-lambda-runtime CONFIG REQUIRED)
find_package(doctest CONFIG REQUIRED)
find_package(Boost REQUIRED)

#add_executable(main "main.cpp")
#target_link_libraries(main PRIVATE AWS::aws-lambda-runtime)
#target_compile_features(main PRIVATE "cxx_std_17")
#target_compile_options(main PRIVATE "-Wall" "-Wextra")
#
#aws_lambda_package_target(main)
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -march=native -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")

add_executable(test_schedule "test_schedule.cpp")
target_link_libraries(test_schedule PRIVATE doctest::doctest Boost::boost)

add_executable(test_validators "test_validators.cpp")
target_link_libraries(test_validators PRIVATE doctest::doctest Boost::boost)

add_executable(test_requirements "test_requirements.cpp")
target_link_libraries(test_requirements PRIVATE doctest::doctest Boost::boost)

enable_testing()
add_test(tests test_schedule)
add_test(tests test_validators)
add_test(tests test_requirements)