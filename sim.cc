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
    // Sintassi: ./PanelsTestBeamSim [nEventi] [nomeFile.root]
    G4int nEvents = 0;
    G4String fileName = "outputPanels.root"; // Nome di default

    if (argc >= 2) {
        nEvents = std::atoi(argv[1]);
    }
    if (argc >= 3) {
        fileName = argv[2];
    }

    // --- 2. SETUP UI (Solo se nEvents == 0) ---
    G4UIExecutive* ui = nullptr;
    if (nEvents == 0) {
        ui = new G4UIExecutive(argc, argv);
    }

    // --- 3. SETUP RUN MANAGER ---
    G4RunManager* runManager = new G4RunManager;

    // Inizializzazione classi obbligatorie
    runManager->SetUserInitialization(new DetectorConstruction());
    runManager->SetUserInitialization(new PhysicsList());
    runManager->SetUserInitialization(new ActionInitialization());

    // --- 4. SETUP VISUALIZZAZIONE ---
    G4VisManager* visManager = new G4VisExecutive;
    visManager->Initialize();

    // --- 5. APERTURA FILE ANALISI (ROOT) ---
    auto analysisManager = G4AnalysisManager::Instance();
    analysisManager->OpenFile(fileName);

    G4UImanager* UImanager = G4UImanager::GetUIpointer();
    
    if (ui) {
        // --- MODALITÀ INTERATTIVA (Grafica) ---
        UImanager->ApplyCommand("/control/execute init_vis.mac");
        ui->SessionStart();
        delete ui;
    }
    else {
        // --- MODALITÀ BATCH (Veloce da terminale) ---
        UImanager->ApplyCommand("/run/initialize");
        UImanager->ApplyCommand("/run/beamOn " + std::to_string(nEvents));
    }

    // --- 6. CHIUSURA E SALVATAGGIO DATI ---
    // Fondamentale per scrivere fisicamente il file .root sul disco
    analysisManager->Write();
    analysisManager->CloseFile();

    // Pulizia memoria
    delete visManager;
    delete runManager;

    return 0;
}
