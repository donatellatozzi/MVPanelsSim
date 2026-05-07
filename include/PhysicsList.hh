#ifndef PhysicsList_h
#define PhysicsList_h 1

#include "G4VModularPhysicsList.hh"

class PhysicsList : public G4VModularPhysicsList { // <-- Cambia qui
public:
    PhysicsList();
    virtual ~PhysicsList();

    // Metodi necessari per G4VModularPhysicsList
    //virtual void SetCuts() override; 
};

#endif
