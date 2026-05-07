#include "PhysicsList.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4OpticalPhysics.hh"
#include "G4HadronPhysicsQGSP_BERT_HP.hh" // Per neutroni e adroni accurati

PhysicsList::PhysicsList() : G4VModularPhysicsList() {
    // 1. Fisica Elettromagnetica accurata
    RegisterPhysics(new G4EmStandardPhysics_option4());
    
    // 2. Decadimenti e Radioattività
    RegisterPhysics(new G4DecayPhysics());
    RegisterPhysics(new G4RadioactiveDecayPhysics());
    
    // 3. Fisica Adronica (opzionale ma consigliata per muoni/neutroni)
    RegisterPhysics(new G4HadronPhysicsQGSP_BERT_HP());

    // 4. Fisica Ottica
    RegisterPhysics(new G4OpticalPhysics());
}

PhysicsList::~PhysicsList() {}


