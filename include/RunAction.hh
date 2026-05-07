#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "G4String.hh"

class RunAction : public G4UserRunAction {
public:
    RunAction(G4String outputFileName);
    virtual ~RunAction();

    virtual void BeginOfRunAction(const G4Run*);
    virtual void EndOfRunAction(const G4Run*);

private:
    G4String fOutputFileName;
};

#endif
