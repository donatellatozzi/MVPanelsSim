#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "Randomize.hh"

#include "DetectorConstruction.hh"
#include "PhysicsList.hh" 
#include "ActionInitialization.hh"

int main(int argc, char** argv)
{
    // --- VARIABILI DI DEFAULT ---
    G4String runMode      = "gui";     // gui oppure batch
    G4String particleType = "muons";   // muons oppure gamma
    G4String geometry     = "Spiral";  // Spiral oppure Standard
    G4String wrapping     = "Tyvek";   // Tyvek oppure Mylar
    G4String outputFile   = "output.root";
    G4String extraArg     = "";        // Numero eventi (muons) o nome macro (gamma)

    // --- PARSER DEGLI ARGOMENTI DA RIGA DI COMANDO ---
    if (argc == 1) {
        G4cout << "----> Avvio GUI con valori di default (Muoni, Spiral, Tyvek)" << G4endl;
    } 
    else if (G4String(argv[1]) == "gui") {
        // ---> LA MAGIA PER LA GUI PERSONALIZZATA <---
        runMode = "gui";
        if (argc > 2) particleType = argv[2];
        if (argc > 3) geometry     = argv[3];
        if (argc > 4) wrapping     = argv[4];
        G4cout << "----> Avvio GUI personalizzata: " << particleType << " | " << geometry << " | " << wrapping << G4endl;
    }
    else if (argc >= 6) {
        // Avvio BATCH silenzioso
        runMode      = "batch";
        particleType = argv[1]; // muons o gamma
        geometry     = argv[2];
        wrapping     = argv[3];
        outputFile   = argv[4];
        extraArg     = argv[5];
    } 
    else {
        G4cerr << "ERRORE: Numero di argomenti non valido!" << G4endl;
        G4cerr << "USO GUI DEFAULT : ./sim" << G4endl;
        G4cerr << "USO GUI CUSTOM  : ./sim gui <gamma/muons> <Spiral/Standard/Nested> <Tyvek/Mylar>" << G4endl;
        G4cerr << "USO BATCH MUONI : ./sim muons <Geometry> <Wrapping> <Output.root> <nEvents>" << G4endl;
        G4cerr << "USO BATCH GAMMA : ./sim gamma <Geometry> <Wrapping> <Output.root> <macro.mac>" << G4endl;
        return 1;
    }

    // --- INIZIALIZZAZIONE RANDOM ENGINE ---
    G4Random::setTheEngine(new CLHEP::RanecuEngine);
    G4Random::setTheSeed(time(NULL));

    // --- CREAZIONE RUN MANAGER ---
    auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

    if (runMode != "gui") {
        runManager->SetNumberOfThreads(4);
    }

    // --- DETECTOR CONSTRUCTION ---
    DetectorConstruction* detector = new DetectorConstruction();
    detector->SetGeometryType(geometry);
    detector->SetWrappingType(wrapping);
    runManager->SetUserInitialization(detector);

    // --- PHYSICS LIST ---
    runManager->SetUserInitialization(new PhysicsList());
    
    // --- ACTION INITIALIZATION (Ora usa particleType coerente) ---
    runManager->SetUserInitialization(new ActionInitialization(outputFile, particleType));

    // --- INIZIALIZZAZIONE KERNEL ---
    runManager->Initialize(); 

    G4UImanager* UImanager = G4UImanager::GetUIpointer();

    // --- GESTIONE ESECUZIONE ---
    if (runMode == "gui") {
        // Modalità INTERATTIVA
        G4UIExecutive* ui = new G4UIExecutive(argc, argv);
        G4VisManager* visManager = new G4VisExecutive;
        visManager->Initialize();
        
        UImanager->ApplyCommand("/vis/open OGL");
        UImanager->ApplyCommand("/vis/drawVolume");
        UImanager->ApplyCommand("/vis/viewer/set/style surface");
        UImanager->ApplyCommand("/vis/scene/add/trajectories smooth");
        
        ui->SessionStart();
        
        delete ui;
        delete visManager;
    } 
    else {
        // Modalità BATCH
        G4cout << "========== INIZIO RUN BATCH ==========" << G4endl;
        G4cout << "Particelle: " << particleType << G4endl;
        G4cout << "Geometria : " << geometry << G4endl;
        G4cout << "Wrapping  : " << wrapping << G4endl;
        G4cout << "File Root : " << outputFile << G4endl;

        if (particleType == "gamma") {
            G4cout << "Esecuzione Macro: " << extraArg << G4endl;
            UImanager->ApplyCommand("/control/execute " + extraArg);
        } 
        else if (particleType == "muons") {
            int nEvents = std::stoi(extraArg);
            G4cout << "Eventi richiesti: " << nEvents << G4endl;
            UImanager->ApplyCommand("/run/beamOn " + std::to_string(nEvents));
        }
        G4cout << "========== FINE RUN BATCH ==========" << G4endl;
    }

    delete runManager;
    return 0;
}
