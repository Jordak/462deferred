set( SRCS "scene.cpp" "objmodel.cpp" "mesh.cpp")
set( INCS "scene.hpp" "objmodel.hpp" "mesh.hpp")

add_library(scene ${SRCS} ${INCS})
source_group(headers FILES ${INCS})
