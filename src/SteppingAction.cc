#include "SteppingAction.hh"
#include "EventAction.hh"
#include "G4Step.hh"
#include "G4RunManager.hh"
#include "G4MuonMinus.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"

// Inizializziamo il puntatore fEventAction
SteppingAction::SteppingAction(EventAction* eventAction)
: G4UserSteppingAction(),
  fEventAction(eventAction)
{}

SteppingAction::~SteppingAction() {}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    if (!fEventAction) return; 

    auto track = step->GetTrack();
    G4double edep = step->GetTotalEnergyDeposit();

    // --- 1. DEPOSITO DI ENERGIA ---
    if (edep > 0.) {
        auto volume = step->GetPreStepPoint()->GetPhysicalVolume();
        if (volume) { 
            G4String volName = volume->GetLogicalVolume()->GetName();
            
            // Controlla se siamo in parti attive/passive del rivelatore
            if (volName.find("Scint") != std::string::npos ||
                volName.find("G")  != std::string::npos ||
                volName.find("Co")  != std::string::npos || 
                volName.find("C")  != std::string::npos) 
            {
                fEventAction->AddEdep(edep);

                if (fEventAction->GetEventStartTime() < 0.) {
                  fEventAction->SetEventStartTime(step->GetPreStepPoint()->GetGlobalTime());
                }
                
                // Track Length solo per il Muone primario
                if (track->GetTrackID() == 1 && track->GetDefinition() == G4MuonMinus::Definition()) {
                     fEventAction->AddTrackLength(step->GetStepLength());
                }
            }
        }
    }

    // --- 2. FOTONI OTTICI ---
    if (track->GetDefinition() == G4OpticalPhoton::Definition()) {
        
        // =========================================================
        // A. CONTEGGIO FOTONI NELLE FIBRE (Logica della Presenza)
        // =========================================================
        // Invece di guardare i confini (pre/post), guardiamo semplicemente
        // in che volume si trova il fotone in QUESTO momento.
        auto currentVol = track->GetVolume();
        if (currentVol) {
            G4String currentLogName = currentVol->GetLogicalVolume()->GetName();
            
            // Se in questo istante il fotone sta volando nel Core o nel Clad...
            if (currentLogName.find("C") != std::string::npos || currentLogName.find("Co") != std::string::npos) {
                // ...e non lo avevamo mai contato prima, Aggiungiamolo!
                if (fEventAction->IsPhotonUnique(track->GetTrackID())) {
                    fEventAction->AddPhotonEntrato(); 
                }
            }
        }

        // =========================================================
        // B. HIT SUL SiPM
        // =========================================================
        auto postPoint = step->GetPostStepPoint();
        if (postPoint->GetPhysicalVolume()) {
            
            G4String postPhysName = postPoint->GetPhysicalVolume()->GetName();
            G4String postLogName  = postPoint->GetPhysicalVolume()->GetLogicalVolume()->GetName();
            
            // Se il passo *finisce* sul SiPM
            if (postPhysName.find("SiPM") != std::string::npos || postLogName.find("SiPM") != std::string::npos) {
                
                // Assicuriamoci che nel passo prima fosse ancora FUORI dal SiPM
                auto prePoint = step->GetPreStepPoint();
                if (prePoint->GetPhysicalVolume()) {
                    G4String prePhysName = prePoint->GetPhysicalVolume()->GetName();
                    if (prePhysName.find("SiPM") == std::string::npos) {
                        
                        G4int sipmID = postPoint->GetPhysicalVolume()->GetCopyNo();
                        G4double globalHitTime = postPoint->GetGlobalTime();
                        G4double eventStartTime = fEventAction->GetEventStartTime();
                        
                        G4double hitTime = globalHitTime; 
                        if (eventStartTime > 0.) {
                            hitTime = globalHitTime - eventStartTime;
                        }
                        
                        G4double energy = track->GetKineticEnergy(); 
                        
                        // Registriamo il segnale
                        fEventAction->RegisterSiPMHit(hitTime);
                        fEventAction->AddPhotonData(track->GetTrackID(), energy, hitTime, sipmID);

                        // Spegniamo il fotone
                        track->SetTrackStatus(fStopAndKill);
                    }
                }
            }
        }
    }

    // --- 3. PRODUZIONE SCINTILLAZIONE ---
    auto secondaries = step->GetSecondaryInCurrentStep();
    for (auto sec : *secondaries) {
        if (sec->GetCreatorProcess()) {
            if (sec->GetCreatorProcess()->GetProcessName() == "Scintillation") {
                fEventAction->AddScintProd();
            }
        }
    }
}
