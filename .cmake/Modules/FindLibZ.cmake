find_path (LIBZ_INCLUDE_DIR
    NAMES
      zlib.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
      ENV CPATH) # PATH and INCLUDE will also work

  find_library (LIBZ_LIBRARIES
    NAMES
      libz.a
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      ENV LIBRARY_PATH   # PATH and LIB will also work
      ENV LD_LIBRARY_PATH)

include (FindPackageHandleStandardArgs)

# handle the QUIETLY and REQUIRED arguments and set LIBZ_FOUND to TRUE
# if all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibZ DEFAULT_MSG
    LIBZ_LIBRARIES
    LIBZ_INCLUDE_DIR)

mark_as_advanced(LIBZ_INCLUDE_DIR LIBZ_LIBRARIES)
