#include "PrimaryGeneratorAction.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ParticleTable.hh"
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include <cmath>

#include "EcoMug.h" 

PrimaryGeneratorAction::PrimaryGeneratorAction(G4String mode)
: G4VUserPrimaryGeneratorAction(),
  fParticleGun(nullptr),
  fGPS(nullptr),
  fMuonGen(nullptr),
  fMode(mode)
{
    if (fMode == "muons") {
        // Inizializza tutto per EcoMug
        fParticleGun = new G4ParticleGun(1);
        fMuonGen = new EcoMug();
        fMuonGen->SetUseSky(); 
    } else {
        // Inizializza il General Particle Source per i Gamma
        fGPS = new G4GeneralParticleSource();
    }
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    if (fParticleGun) delete fParticleGun;
    if (fGPS) delete fGPS;
    if (fMuonGen) delete fMuonGen;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{                  
    if (fMode == "muons") {
        // --- LOGICA MUONI (ECOMUG) CON REJECTION SAMPLING ---
        
        G4ThreeVector pos;
        G4ThreeVector dir;
        bool hit_telescope = false;
        
        // Loop: continuiamo a generare muoni in memoria finché non ne 
        // troviamo uno che è garantito per colpire entrambe le palette.
        while (!hit_telescope) {
            fMuonGen->Generate();

            // 1. Spariamo da SOPRA la paletta superiore (es. Z = 260 mm)
            // da un'area di 8x8 cm centrata sul Canale 32
            G4double x_pos = -375.0 * mm + (G4UniformRand() - 0.5) * 80.0 * mm; 
            G4double y_pos = -125.0 * mm + (G4UniformRand() - 0.5) * 80.0 * mm; 
            G4double z_pos = 220.0 * mm; 
            pos = G4ThreeVector(x_pos, y_pos, z_pos);
                            
            G4double theta = fMuonGen->GetGenerationTheta();
            G4double phi   = fMuonGen->GetGenerationPhi();
            
            // 2. Calcolo Direzione
            dir.setX( std::sin(theta) * std::cos(phi) );
            dir.setY( std::sin(theta) * std::sin(phi) );
            
            // Assicuriamoci che il muone viaggi verso il basso (-Z)
            G4double dir_z = std::cos(theta);
            if (dir_z > 0) dir_z = -dir_z; 
            dir.setZ(dir_z); 
            
            // 3. Ray-tracing matematico per verificare l'impatto sulla paletta inferiore (Z = 50 mm)
            if (dir.z() != 0) {
                G4double t_low = (13.5 * mm - z_pos) / dir.z();
                
                if (t_low > 0) { // Il muone va nella direzione corretta
                    G4double x_low = x_pos + t_low * dir.x();
                    G4double y_low = y_pos + t_low * dir.y();
                    
                    // Controlla se colpisce l'area 70x70 mm del Canale 32
                    if (std::abs(x_low - (-375.0 * mm)) <= 35.0 * mm && 
                        std::abs(y_low - (-125.0 * mm)) <= 35.0 * mm) {
                        
                        hit_telescope = true; // Impatto confermato, esci dal loop!
                    }
                }
            }
        }
                        
        // Energia e Massa (applicate SOLO al muone vincente)
        G4double p = fMuonGen->GetGenerationMomentum() * GeV; 
        G4double mass = G4MuonMinus::MuonMinus()->GetPDGMass();
        G4double ekin = std::sqrt(p*p + mass*mass) - mass;

        if (fMuonGen->GetCharge() < 0) {
            fParticleGun->SetParticleDefinition(G4MuonMinus::MuonMinus());
        } else {
            fParticleGun->SetParticleDefinition(G4MuonPlus::MuonPlus());
        }

        fParticleGun->SetParticlePosition(pos);
        fParticleGun->SetParticleMomentumDirection(dir);
        fParticleGun->SetParticleEnergy(ekin);

        // Geant4 ora simulerà la fisica ottica SOLO per questo muone perfetto
        fParticleGun->GeneratePrimaryVertex(anEvent);
    } 
    else {
        // --- LOGICA GAMMA (GPS VIA MACRO) ---
        fGPS->GeneratePrimaryVertex(anEvent);
    }
}
