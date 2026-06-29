#include "SteppingAction.hh"
#include "EventAction.hh"
#include "G4Step.hh"
#include "G4RunManager.hh"
#include "G4MuonMinus.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4ProcessManager.hh"

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
        // A. CONTEGGIO FOTONI NEL CORE DELLA FIBRA (Prima entrata)
        // =========================================================
        // Conta ogni traccia fotonica la prima volta che attraversa il confine
        // da un volume non-core a _LCore (il core WLS dove avviene l'assorbimento
        // UV e la riemissione verde). I fotoni WLS nati nel core non attivano
        // questa condizione (il loro primo step ha pre-step gia' nel core).
        // IsPhotonUnique garantisce il conteggio singolo anche per rientri.
        {
            auto preVol  = step->GetPreStepPoint()->GetPhysicalVolume();
            auto postVol = step->GetPostStepPoint()->GetPhysicalVolume();
            if (preVol && postVol) {
                G4String preName  = preVol->GetLogicalVolume()->GetName();
                G4String postName = postVol->GetLogicalVolume()->GetName();

                bool preInCore  = (preName.find("_LCore")  != G4String::npos);
                bool postInCore = (postName.find("_LCore") != G4String::npos);

                if (!preInCore && postInCore) {
                    if (fEventAction->IsPhotonUnique(track->GetTrackID())) {
                        fEventAction->AddPhotonEntrato();
                    }
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

                        // Controlla se il fotone e' stato effettivamente rivelato (PDE)
                        // G4OpBoundaryProcess setta Status==Detection quando EFFICIENCY>0
                        // assorbe il fotone con probabilita' (1-EFFICIENCY)
                        G4bool detected = false;
                        G4ProcessManager* pm = track->GetDefinition()->GetProcessManager();
                        if (pm) {
                            G4ProcessVector* pv = pm->GetPostStepProcessVector();
                            for (G4int j = 0; j < (G4int)pv->entries(); ++j) {
                                if ((*pv)[j]->GetProcessName() == "OpBoundary") {
                                    auto* bp = dynamic_cast<G4OpBoundaryProcess*>((*pv)[j]);
                                    if (bp) detected = (bp->GetStatus() == Detection);
                                    break;
                                }
                            }
                        }

                        fEventAction->RegisterSiPMHit(hitTime, detected);
                        fEventAction->AddPhotonData(track->GetTrackID(), energy, hitTime, sipmID, detected);

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
