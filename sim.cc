#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "Randomize.hh"
#include <unistd.h>

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
    G4long   seed         = 0;         // 0 = automatico/indipendente, >0 = riproducibile

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
        if (argc >= 7) seed = std::stol(argv[6]); // seed opzionale (riproducibilità)
    }
    else {
        G4cerr << "ERRORE: Numero di argomenti non valido!" << G4endl;
        G4cerr << "USO GUI DEFAULT : ./sim" << G4endl;
        G4cerr << "USO GUI CUSTOM  : ./sim gui <gamma/muons> <Spiral/Standard/Nested> <Tyvek/Mylar>" << G4endl;
        G4cerr << "USO BATCH MUONI : ./sim muons    <Geometry> <Wrapping> <Output.root> <nEvents> [seed]" << G4endl;
        G4cerr << "USO BATCH GAMMA : ./sim gamma    <Geometry> <Wrapping> <Output.root> <macro.mac>" << G4endl;
        G4cerr << "USO BATCH ELETTRONI: ./sim electron <Geometry> <Wrapping> <Output.root> <macro.mac>" << G4endl;
        G4cerr << "  [seed] omesso o 0 -> run indipendente (multi-thread, per accumulare statistica)." << G4endl;
        G4cerr << "  [seed] > 0        -> run riproducibile (forza 1 thread), utile per confrontare" << G4endl;
        G4cerr << "                       geometrie diverse sugli stessi identici eventi." << G4endl;
        return 1;
    }

    // --- INIZIALIZZAZIONE RANDOM ENGINE ---
    G4Random::setTheEngine(new CLHEP::RanecuEngine);
    if (seed > 0) {
        G4cout << "----> Seed fisso: " << seed << " (run riproducibile, forzato 1 thread)" << G4endl;
        G4Random::setTheSeed(seed);
    } else {
        // time(NULL) da solo ha risoluzione di 1 secondo: due processi lanciati
        // in parallelo nello stesso secondo avrebbero lo stesso seed globale.
        // Aggiungendo il PID il seed resta diverso anche in quel caso.
        G4long autoSeed = static_cast<G4long>(time(NULL)) + static_cast<G4long>(getpid());
        G4Random::setTheSeed(autoSeed);
    }

    // --- CREAZIONE RUN MANAGER ---
    // Con un seed fisso usiamo il run manager Serial vero (non MT con 1 thread):
    // il run manager MT, anche con un solo worker, ri-seeda il motore random
    // globale evento per evento per garantire la sua riproducibilità interna,
    // ma questo non si combina con il generatore di EcoMug (che avanza in modo
    // continuo) e produce uno sfasamento tra run con geometrie diverse. In
    // modalità Serial il motore random avanza in un unico flusso continuo,
    // identico a parità di seed e di codice eseguito -> confronto evento per
    // evento affidabile tra geometrie diverse.
    auto* runManager = (seed > 0)
        ? G4RunManagerFactory::CreateRunManager(G4RunManagerType::SerialOnly)
        : G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

    if (runMode != "gui" && seed == 0) {
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
    runManager->SetUserInitialization(new ActionInitialization(outputFile, particleType, seed));

    // --- INIZIALIZZAZIONE KERNEL ---
    runManager->Initialize();

    // Il controllo di overlap dei volumi (checkOverlaps=true nei G4PVPlacement)
    // consuma numeri casuali dal motore globale durante Initialize(), e lo fa
    // un numero di volte diverso a seconda di quanti volumi vengono piazzati
    // dalla geometria scelta. Per garantire che due geometrie diverse partano
    // dallo stesso identico stato del generatore all'inizio del run, ri-applichiamo
    // il seed qui, dopo la costruzione della geometria e prima del beamOn.
    if (seed > 0) {
        G4Random::setTheSeed(seed);
    }

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

        if (particleType == "muons") {
            int nEvents = std::stoi(extraArg);
            G4cout << "Eventi richiesti: " << nEvents << G4endl;
            UImanager->ApplyCommand("/run/beamOn " + std::to_string(nEvents));
        }
        else {
            // Qualsiasi altra particella (gamma, electron, ...) usa GPS via macro
            G4cout << "Esecuzione Macro: " << extraArg << G4endl;
            UImanager->ApplyCommand("/control/execute " + extraArg);
        }
        G4cout << "========== FINE RUN BATCH ==========" << G4endl;
    }

    delete runManager;
    return 0;
}
