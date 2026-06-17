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
        
        // Dimensioni del pannello Nested (vedi DetectorConstruction::ConstructNestedPanel:
        // scintX/Y sono semi-estensioni, blackZ = scintZ + thTyvek + thAl + thBlack è la
        // semi-estensione totale lungo Z, cioè la quota della faccia superiore del pannello).
        const G4double kPanelHalfX = 625.0 * mm;
        const G4double kPanelHalfY = 275.0 * mm;
        const G4double kPanelTopZ  = 25.7  * mm; // 25.0 (scint) + 0.4 + 0.1 + 0.2 (involucri)
        const G4double kMarginZ    = 50.0  * mm; // margine d'aria sopra il pannello
        const G4double kStartZ     = kPanelTopZ + kMarginZ;

        G4ThreeVector pos;
        G4ThreeVector dir;
        bool hit_panel = false;

        // Loop: continuiamo a generare muoni in memoria finché non ne
        // troviamo uno che è garantito per colpire il pannello.
        while (!hit_panel) {
            fMuonGen->Generate();

            // 1. Spariamo da SOPRA tutto il pannello (rettangolo pari alla sua
            // estensione XY), con un piccolo margine d'aria sopra la faccia superiore.
            G4double x_pos = (G4UniformRand() - 0.5) * 2.0 * kPanelHalfX;
            G4double y_pos = (G4UniformRand() - 0.5) * 2.0 * kPanelHalfY;
            pos = G4ThreeVector(x_pos, y_pos, kStartZ);

            G4double theta = fMuonGen->GetGenerationTheta();
            G4double phi   = fMuonGen->GetGenerationPhi();

            // 2. Calcolo Direzione
            dir.setX( std::sin(theta) * std::cos(phi) );
            dir.setY( std::sin(theta) * std::sin(phi) );

            // Assicuriamoci che il muone viaggi verso il basso (-Z)
            G4double dir_z = std::cos(theta);
            if (dir_z > 0) dir_z = -dir_z;
            dir.setZ(dir_z);

            // 3. Ray-tracing matematico per verificare l'impatto sulla faccia superiore del pannello
            if (dir.z() != 0) {
                G4double t_top = (kPanelTopZ - kStartZ) / dir.z();

                if (t_top > 0) { // Il muone va nella direzione corretta
                    G4double x_top = x_pos + t_top * dir.x();
                    G4double y_top = y_pos + t_top * dir.y();

                    // Controlla se colpisce l'area del pannello
                    if (std::abs(x_top) <= kPanelHalfX &&
                        std::abs(y_top) <= kPanelHalfY) {

                        hit_panel = true; // Impatto confermato, esci dal loop!
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
