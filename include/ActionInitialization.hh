#ifndef ActionInitialization_h
#define ActionInitialization_h 1

#include "G4VUserActionInitialization.hh"
#include "G4String.hh"

/// Classe di inizializzazione delle azioni utente.
/// Registra:
/// - PrimaryGeneratorAction (Generazione muoni)
/// - RunAction (Gestione file ROOT e Ntuple)
/// - EventAction (Raccolta dati evento)
/// - SteppingAction (Opzionale, se serve tracciare passo-passo)

class ActionInitialization : public G4VUserActionInitialization
{
  public:
    // Costruttore: accetta il nome del file di output (es. "output.root") e un
    // seed opzionale (0 = automatico/indipendente, >0 = riproducibile)
  ActionInitialization(G4String outputFileName, G4String mode, G4long seed = 0);
    virtual ~ActionInitialization();

    // Metodo chiamato solo dal thread Master in modalità Multi-Threading
    // Qui si istanzia SOLO la RunAction (per gestire il merging dei file/Ntuple)
    virtual void BuildForMaster() const;

    // Metodo chiamato dai thread Worker (o dal main in sequenziale)
    // Qui si istanziano TUTTE le action
    virtual void Build() const;

  private:
    // Variabile per conservare il nome del file passato dal main
    G4String fOutputFileName;
    G4String fMode;
    G4long fSeed;
};

#endif
