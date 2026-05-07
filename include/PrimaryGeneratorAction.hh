#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"

class G4ParticleGun;
class G4GeneralParticleSource;
class G4Event;
class EcoMug; 

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    // Aggiungiamo il parametro "mode" al costruttore
    PrimaryGeneratorAction(G4String mode);
    virtual ~PrimaryGeneratorAction();

    virtual void GeneratePrimaries(G4Event*);

  private:
    G4ParticleGun* fParticleGun;
    G4GeneralParticleSource* fGPS;
    EcoMug* fMuonGen; 
    G4String fMode; // Salverà "muons" o "gamma"
};

#endif
