cmake_minimum_required(VERSION 3.16)
project(PanelsTestBeamSim)

# Trova Geant4 nell'ambiente Conda
find_package(Geant4 REQUIRED ui_all vis_all)

# Include le macro di Geant4
include(${Geant4_USE_FILE})

# Elenca i file sorgente 
include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

# Crea l'eseguibile
add_executable(PanelsTestBeamSim sim.cc ${sources} ${headers})
target_link_libraries(PanelsTestBeamSim ${Geant4_LIBRARIES})

# Copia i file macro (.mac) nella cartella di build
file(GLOB macros ${PROJECT_SOURCE_DIR}/*.mac)
foreach(_file ${macros})
  configure_file(${_file} ${PROJECT_BINARY_DIR}/${_file_NAME} COPYONLY)
endforeach()
