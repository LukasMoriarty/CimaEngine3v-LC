project(CimaEngine3v)

#todos los archivos agregados
file(GLOB_RECURSE gl_cpp RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(CimaEngine3v PRIVATE ${gl_cpp})

add_subdirectory(Naves)
add_subdirectory(Bosses)
add_subdirectory(Pies)
add_subdirectory(Cuerpo)
add_subdirectory(Zombie)
add_subdirectory(FX)
add_subdirectory(Balas)
