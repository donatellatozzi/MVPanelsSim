#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"

ActionInitialization::ActionInitialization() : G4VUserActionInitialization()
{}

ActionInitialization::~ActionInitialization()
{}

// Questa funzione viene chiamata per il "Master" thread (utile in multi-threading)
void ActionInitialization::BuildForMaster() const
{
  SetUserAction(new EventAction());
}

// Questa funzione viene chiamata per ogni "Worker" thread (dove avviene la simulazione vera)
void ActionInitialization::Build() const
{
  // 1. Registra il generatore primario (l'elettrone da 500 MeV)
  SetUserAction(new PrimaryGeneratorAction);

  // 2. Registra l'EventAction (gestisce l'inizio/fine evento e i dati dei PMT)
  EventAction* eventAction = new EventAction();
  SetUserAction(eventAction);

  // 3. Registra la SteppingAction (osserva ogni passo per contare i fotoni Cherenkov)
  SetUserAction(new SteppingAction());
}
