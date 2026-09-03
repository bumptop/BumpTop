# Post-processes a protoc-generated C++ header so legacy BumpTop classes can
# inherit from the generated messages (AppSettings : Settings,
# ProAuthorization : ProConfig, ...).
#
# protoc marks generated classes `final` (and, since protoc 36, destructors
# `PROTOBUF_FINAL` too); both must come off. Run as:
#   cmake -DPB_HEADER=<path/to/AllMessages.pb.h> -P unfinal_protobuf_header.cmake

if(NOT PB_HEADER)
    message(FATAL_ERROR "PB_HEADER not set")
endif()

file(READ ${PB_HEADER} _content)
string(REPLACE " final :" " :" _content "${_content}")
string(REPLACE "PROTOBUF_FINAL" "" _content "${_content}")
file(WRITE ${PB_HEADER} "${_content}")
