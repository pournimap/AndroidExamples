# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# configure import libs
set(distribution_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../distribution)

add_library( # Sets the name of the library.
        lib_assimp

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        IMPORTED )

#configure each library binary location(using `set_target_properties`)
set_target_properties(lib_assimp PROPERTIES IMPORTED_LOCATION
        ${distribution_DIR}/assimp/lib/${ANDROID_ABI}/libassimpd.so)

# build application's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             native-lib.cpp )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# configure each library headers location (using `target_include_directories`)
target_include_directories(native-lib
        PRIVATE
        ${distribution_DIR}/assimp/include)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib
                       EGL
                       GLESv3

                       android
                       lib_assimp
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )