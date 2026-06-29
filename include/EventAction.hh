#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"
#include <vector>
#include <unordered_set>

struct PhotonHitInfo {
    G4int    eventID;
    G4int    trackID;
    G4double wavelength;
    G4double time;
    G4int    sipmID;
    G4bool   detected;  // true se il fotone ha passato la selezione PDE del SiPM
};

class EventAction : public G4UserEventAction {
public:
    EventAction();
    virtual ~EventAction();

    virtual void BeginOfEventAction(const G4Event*);
    virtual void EndOfEventAction(const G4Event*);

    // Metodi per accumulare dati
    void SetEinit(G4double e) { fEinit = e; }
    void AddEdep(G4double edep) { fEdep += edep; }
    void AddTrackLength(G4double len) { fTrackLength += len; }
    
    void AddScintProd() { fNScintProd++; }
    void AddPhotonEntrato() { fNPhotonsEntrati++; }
    
    bool IsPhotonUnique(G4int trackID);

    // Hit su SiPM: total (tutti gli arrivi) + detected (solo quelli che passano PDE)
    void RegisterSiPMHit(G4double time, G4bool detected);

    void AddPhotonData(G4int trackID, G4double energy, G4double time, G4int sipmID, G4bool detected);
    void SetEventStartTime(G4double t) { if (fEventStartTime < 0.) fEventStartTime = t; }
    G4double GetEventStartTime() const { return fEventStartTime; }

private:
    // Dati per Ntuple 0
    G4double fEinit;
    G4double fEdep;
    G4double fTrackLength;
    G4int    fNScintProd;
    G4double fFirstHitTime;
    G4int    fNPhotonsEntrati;
    
    G4int    fSiPMHits;           // tutti i fotoni che arrivano al SiPM
    G4int    fSiPMHitsDetected;   // solo quelli effettivamente rivelati (PDE applicata)
    
    std::unordered_set<G4int> fCountedPhotons;
    std::vector<PhotonHitInfo> fPhotonBuffer;
    G4double fEventStartTime;
};

#endif
