# ASIO_FOUND
# ASIO_INCLUDE_DIR

find_path(ASIO_INCLUDE_DIR NAMES asio.hpp)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(asio DEFAULT_MSG ASIO_INCLUDE_DIR)

mark_as_advanced(ASIO_INCLUDE_DIR)
