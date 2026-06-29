#include "RunAction.hh"
#include "G4AnalysisManager.hh"
#include "G4Run.hh"

RunAction::RunAction(G4String outputFileName)
: G4UserRunAction(), fOutputFileName(outputFileName)
{
    auto am = G4AnalysisManager::Instance();
    am->SetDefaultFileType("root");
    
    // FONDAMENTALE PER MULTITHREADING
    am->SetNtupleMerging(true);
    am->SetVerboseLevel(1);

    // --- NTUPLE 0: Sommario ---
    am->CreateNtuple("Events", "Sommario Eventi");
    am->CreateNtupleDColumn("E_init_MeV");      // 0
    am->CreateNtupleDColumn("Edep_MeV");        // 1
    am->CreateNtupleDColumn("TrackLength_mm");  // 2
    am->CreateNtupleIColumn("N_Scint");         // 3
    am->CreateNtupleDColumn("T_First_ns");      // 4
    am->CreateNtupleIColumn("PhotonsInFiber");  // 5
    am->CreateNtupleIColumn("Hits_SiPM_Total"); // 6
    am->CreateNtupleDColumn("Vtx_X_mm");        // 7
    am->CreateNtupleDColumn("Vtx_Y_mm");        // 8
    am->CreateNtupleDColumn("Vtx_Z_mm");        // 9
    am->CreateNtupleDColumn("Dir_X");           // 10
    am->CreateNtupleDColumn("Dir_Y");           // 11
    am->CreateNtupleDColumn("Dir_Z");               // 12
    am->CreateNtupleIColumn("Hits_SiPM_Detected");  // 13: fotoni rivelati dopo PDE
    am->FinishNtuple(0);

    // --- NTUPLE 1: Fotoni ---
    am->CreateNtuple("Photons", "Dettaglio Fotoni");
    am->CreateNtupleIColumn("EventID");         // 0
    am->CreateNtupleIColumn("TrackID");         // 1
    // Salviamo la lunghezza d'onda in nanometri
    am->CreateNtupleDColumn("Wavelength_nm");   // 2
    am->CreateNtupleDColumn("Arrival_Time_ns"); // 3
    am->CreateNtupleIColumn("SiPM_ID");         // 4
    am->CreateNtupleIColumn("Detected");        // 5: 1=rivelato (PDE), 0=assorbito senza segnale
    am->FinishNtuple(1);
}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*) {
    auto am = G4AnalysisManager::Instance();
    am->OpenFile(fOutputFileName);
}

void RunAction::EndOfRunAction(const G4Run*) {
    auto am = G4AnalysisManager::Instance();
    am->Write();
    am->CloseFile();
}
