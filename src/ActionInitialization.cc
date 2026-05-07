#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"
#include "TrackingAction.hh" // Se usi ancora TrackingAction per nascondere traiettorie

ActionInitialization::ActionInitialization(G4String outputFileName, G4String mode)
  : G4VUserActionInitialization(), fOutputFileName(outputFileName), fMode(mode)
{}

ActionInitialization::~ActionInitialization()
{}

void ActionInitialization::BuildForMaster() const {
    SetUserAction(new RunAction(fOutputFileName));
}

void ActionInitialization::Build() const {
    SetUserAction(new PrimaryGeneratorAction(fMode));
    SetUserAction(new RunAction(fOutputFileName)); 
    
    // 1. Creiamo EventAction
    auto eventAction = new EventAction();
    SetUserAction(eventAction);
    
    // 2. Lo passiamo al costruttore di SteppingAction
    SetUserAction(new SteppingAction(eventAction));

    // 3. TrackingAction (Opzionale per la RAM in vis)
    SetUserAction(new TrackingAction());
}
