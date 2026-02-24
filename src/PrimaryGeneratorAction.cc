#include "PrimaryGeneratorAction.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4RunManager.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction() : G4VUserPrimaryGeneratorAction(), fParticleGun(nullptr)
{
    fParticleGun = new G4ParticleGun(1); // 1 particella per evento
    
    // Settiamo il Muone negativo
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("mu-");
    fParticleGun->SetParticleDefinition(particle);
    
    // Energia tipica media dei muoni a livello del mare
    fParticleGun->SetParticleEnergy(4.0 * GeV);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    // 1. POSIZIONE (Sopra il rivelatore)
    // Creiamo un'area di generazione (tetto) di 2 metri x 2 metri
    // a un'altezza Z = +1 metro rispetto al centro del rivelatore.
    G4double halfSize = 1.0 * m; 
    G4double x0 = halfSize * 2.0 * (G4UniformRand() - 0.5); // Da -1m a +1m
    G4double y0 = halfSize * 2.0 * (G4UniformRand() - 0.5); // Da -1m a +1m
    G4double z0 = 1.0 * m;                                  // Altezza
    
    fParticleGun->SetParticlePosition(G4ThreeVector(x0, y0, z0));

    // 2. DIREZIONE (Distribuzione angolare cosmica cos^2(theta))
    // La matematica per generare una distribuzione cos^2(theta)
    G4double u = std::cbrt(G4UniformRand()); // u = cos(theta). La radice cubica genera la PDF cos^2(theta)
    G4double sinTheta = std::sqrt(1.0 - u * u);
    G4double phi = 2.0 * M_PI * G4UniformRand(); // Angolo azimutale uniforme

    // Calcoliamo le componenti del vettore momento
    G4double px = sinTheta * std::cos(phi);
    G4double py = sinTheta * std::sin(phi);
    G4double pz = -u; // -u perché i muoni piovono verso il BASSO (-Z)

    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(px, py, pz));

    // Generiamo la particella!
    fParticleGun->GeneratePrimaryVertex(anEvent);
}
