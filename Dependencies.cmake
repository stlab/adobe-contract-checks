Include(FetchContent)

if(BUILD_TESTING)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/ff233bdd4cac0a0bf6e5cd45bda3406814cb2796.zip
    DOWNLOAD_EXTRACT_TIMESTAMP "ON"
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  FetchContent_Declare(
    icm
    GIT_REPOSITORY https://github.com/iboB/icm.git
    GIT_TAG        v1.5.1
  )

  FetchContent_MakeAvailable(googletest icm)
  list(APPEND CMAKE_MODULE_PATH "${icm_SOURCE_DIR}")
endif()
