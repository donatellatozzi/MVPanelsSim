#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"

class G4ParticleGun;
class G4GeneralParticleSource;
class G4Event;
class EcoMug;
namespace CLHEP { class RanecuEngine; }

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    // Aggiungiamo il parametro "mode" al costruttore. "seed" (>0) rende
    // riproducibile la generazione del primario (posizione/angolo/momento/carica);
    // 0 = automatico/indipendente (usato per accumulare statistica in parallelo).
    PrimaryGeneratorAction(G4String mode, G4long seed = 0);
    virtual ~PrimaryGeneratorAction();

    virtual void GeneratePrimaries(G4Event*);

  private:
    G4ParticleGun* fParticleGun;
    G4GeneralParticleSource* fGPS;
    EcoMug* fMuonGen;
    G4String fMode; // Salverà "muons" o "gamma"

    // Motore random dedicato SOLO alla generazione del primario (posizione,
    // carica), separato dal motore globale di Geant4 usato dal tracking/fisica
    // ottica. Così, a parità di seed, la sequenza di muoni primari generati è
    // identica indipendentemente da quanta "casualità" consuma la fisica a
    // valle (che dipende dalla geometria, cioè da quello che vogliamo confrontare).
    CLHEP::RanecuEngine* fGenEngine;
};

#endif
