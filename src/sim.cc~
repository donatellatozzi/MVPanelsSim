#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "ActionInitialization.hh"
#include "G4AnalysisManager.hh"

int main(int argc, char** argv) {
    
    // --- 1. GESTIONE ARGOMENTI DA TERMINALE ---
    G4int nEvents = 0;
    G4String fileName = "output.root"; // Nome di default
    G4bool useMylar = false;  // Default: NO Mylar

    if (argc >= 2) nEvents = std::atoi(argv[1]); // Il primo argomento è il numero di eventi
    if (argc >= 3) fileName = argv[2]; // Il secondo argomento è il nome del file
    if (argc >= 4) {
      // Se il terzo argomento = 1 -> Attivo Mylar
      if (std::atoi(argv[3]) == 1) useMylar = true;
    }

    // --- 2. SETUP GEANT4 ---
    G4RunManager* runManager = new G4RunManager;
    runManager->SetUserInitialization(new DetectorConstruction(useMylar));
    runManager->SetUserInitialization(new PhysicsList());
    runManager->SetUserInitialization(new ActionInitialization());

    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

    // --- 3. APERTURA FILE ANALISI ---
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->OpenFile(fileName);

    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    
    if (nEvents > 0) {
        // --- MODALITÀ BATCH (Da terminale) ---
        // Se hai inserito un numero di eventi, Geant4 non apre la grafica
        // ma esegue i calcoli il più velocemente possibile.
        UImanager->ApplyCommand("/run/initialize");
        UImanager->ApplyCommand("/run/beamOn " + std::to_string(nEvents));
    }
    else {
        // --- MODALITÀ INTERATTIVA (Grafica) ---
        // Se non metti argomenti, si apre la finestra come prima.
        G4UIExecutive* ui = new G4UIExecutive(argc, argv);
        UImanager->ApplyCommand("/control/execute init_vis.mac");
        ui->SessionStart();
        delete ui;
    }

    // --- 4. CHIUSURA E SALVATAGGIO ---
    analysisManager->Write();
    analysisManager->CloseFile();

    delete visManager;
    delete runManager;

    return 0;
}
