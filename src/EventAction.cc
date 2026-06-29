#include "EventAction.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh" // Fondamentale per h_Planck e c_light
#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4PrimaryVertex.hh"
#include "G4PrimaryParticle.hh"
#include <algorithm> 

EventAction::EventAction() : G4UserEventAction() {}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event*) {
    // Reset variabili
    fEinit = 0.;
    fEdep = 0.;
    fTrackLength = 0.;
    fNScintProd = 0;
    fFirstHitTime = 999999.*ns;
    fNPhotonsEntrati = 0;
    fSiPMHits = 0;
    fSiPMHitsDetected = 0;

    fEventStartTime = -1.0;
    
    fCountedPhotons.clear();
    fPhotonBuffer.clear();
}

bool EventAction::IsPhotonUnique(G4int trackID) {
    // insert() ritorna una coppia: un iteratore e un booleano. 
    // Il booleano è 'true' se l'elemento è stato inserito (cioè non c'era),
    // 'false' se l'elemento era già presente. Fa tutto in un colpo solo!
    return fCountedPhotons.insert(trackID).second;
}

void EventAction::RegisterSiPMHit(G4double time, G4bool detected) {
    if (time < fFirstHitTime) fFirstHitTime = time;
    fSiPMHits++;
    if (detected) fSiPMHitsDetected++;
}

void EventAction::AddPhotonData(G4int trackID, G4double energy, G4double time, G4int sipmID, G4bool detected) {
    PhotonHitInfo hit;
    hit.trackID  = trackID;
    G4double lambda = (h_Planck * c_light) / energy;
    hit.wavelength  = lambda / nm;
    hit.time        = time;
    hit.sipmID      = sipmID;
    hit.detected    = detected;
    fPhotonBuffer.push_back(hit);
}

void EventAction::EndOfEventAction(const G4Event* event) {
    auto am = G4AnalysisManager::Instance();

    G4ThreeVector vtxPos(0,0,0);
    G4ThreeVector dir(0,0,0);

    if (event->GetNumberOfPrimaryVertex() > 0) {
        G4PrimaryVertex* vertex = event->GetPrimaryVertex(0);
	vtxPos = vertex->GetPosition();
	
        if (vertex->GetNumberOfParticle() > 0) {
	  G4PrimaryParticle* primary = vertex->GetPrimary(0);
	  fEinit = primary->GetTotalEnergy();
	  dir = primary->GetMomentumDirection();
        }
    }

    // --- NTUPLE 0 (Sommario) ---
    am->FillNtupleDColumn(0, 0, fEinit/MeV);
    am->FillNtupleDColumn(0, 1, fEdep/MeV);
    am->FillNtupleDColumn(0, 2, fTrackLength/mm);
    am->FillNtupleIColumn(0, 3, fNScintProd);
    am->FillNtupleDColumn(0, 4, (fFirstHitTime > 90000.*ns) ? -1.0 : fFirstHitTime/ns);
    am->FillNtupleIColumn(0, 5, fNPhotonsEntrati);
    am->FillNtupleIColumn(0, 6, fSiPMHits);          // colonna 6: fotoni arrivati al SiPM
    am->FillNtupleIColumn(0, 13, fSiPMHitsDetected); // colonna 13: fotoni rivelati (PDE)
    am->FillNtupleDColumn(0, 7, vtxPos.x()/mm);
    am->FillNtupleDColumn(0, 8, vtxPos.y()/mm);
    am->FillNtupleDColumn(0, 9, vtxPos.z()/mm);
    am->FillNtupleDColumn(0, 10, dir.x());
    am->FillNtupleDColumn(0, 11, dir.y());
    am->FillNtupleDColumn(0, 12, dir.z());
    
    am->AddNtupleRow(0); 

    // --- NTUPLE 1 (Dettaglio) ---
    G4int evtID = event->GetEventID();
    
    for (const auto& hit : fPhotonBuffer) {
        am->FillNtupleIColumn(1, 0, evtID);
        am->FillNtupleIColumn(1, 1, hit.trackID);
        // hit.wavelength è già un numero puro in nm (convertito in AddPhotonData)
        am->FillNtupleDColumn(1, 2, hit.wavelength); 
        am->FillNtupleDColumn(1, 3, hit.time/ns);
        am->FillNtupleIColumn(1, 4, hit.sipmID);
        am->FillNtupleIColumn(1, 5, hit.detected ? 1 : 0);
        am->AddNtupleRow(1);
    }
}
