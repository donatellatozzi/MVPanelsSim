#include "SIPMSD.hh"


SIPMSD::SIPMSD(G4String name) : G4VSensitiveDetector(name) {}

SIPMSD::~SIPMSD() {}

void SIPMSD::Initialize(G4HCofThisEvent*) {}

G4bool SIPMSD::ProcessHits(G4Step*, G4TouchableHistory*) {
  return false;
}


void SIPMSD::EndOfEvent(G4HCofThisEvent*) {}
