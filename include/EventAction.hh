#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"
#include <set>

class EventAction : public G4UserEventAction {
public:
    EventAction();
    virtual ~EventAction();

    virtual void BeginOfEventAction(const G4Event*);
    virtual void EndOfEventAction(const G4Event*);

    void AddSiPMHit(G4int id) { if(id>=0 && id<4) fSiPMHits[id]++; }
    void AddEdep(G4double edep) { fEdep += edep; }
    void AddTrackLength(G4double L) { fTrackLength += L; }
    void AddScintProd() { fNScintProd++; }
    void AddPhotonEntrato() { fNPhotonsEntrati++; } 
    void SetInitialEnergy(G4double e) { fEinit = e; }
    void RegisterSiPMHit(G4double time);

    G4bool IsPhotonUnique(G4int tid) {
      if (fCountedPhotons.find(tid) == fCountedPhotons.end()) {
        fCountedPhotons.insert(tid);
        return true;
      }
      return false;
    }

private:
    G4double fEinit, fEdep, fTrackLength, fFirstHitTime;
    G4int fNScintProd;
    G4int fSiPMHits[4];
    G4int fNPhotonsEntrati;
    std::set<G4int> fCountedPhotons;
};
#endif
