#include "DetectorConstruction.hh"
#include "G4VisAttributes.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Torus.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4NistManager.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4SDManager.hh"
#include "G4RotationMatrix.hh"
#include "SIPMSD.hh"
#include <cmath>

DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction() {}
DetectorConstruction::~DetectorConstruction(){}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    G4NistManager* nist = G4NistManager::Instance();

    // --- 1. MATERIALI ---
    G4Material* air       = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* ej200     = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    G4Material* coreMat   = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
    G4Material* cladMat   = nist->FindOrBuildMaterial("G4_PLEXIGLASS");
    G4Material* sipmMat   = nist->FindOrBuildMaterial("G4_Si");
    G4Material* blackHDPE = nist->FindOrBuildMaterial("G4_POLYETHYLENE");
    G4Material* glueMat = nist->FindOrBuildMaterial("G4_PLEXIGLASS");

    // --- 2. PROPRIETÀ OTTICHE (EJ-200 & BCF-92) ---
    
    // Definiamo 8 punti di energia (da circa 620 nm a 350 nm)
    G4int nEntries = 8;
    G4double energy[8] = {2.00*eV, 2.30*eV, 2.50*eV, 2.70*eV, 2.90*eV, 3.10*eV, 3.30*eV, 3.50*eV};

    // -----------------------------------------------------------
    // A. Scintillatore: ELJEN EJ-200
    // Emissione picco: 425 nm (~2.9 eV). Attenuation length: 380 cm.
    // -----------------------------------------------------------
    G4double rindexScint[8]  = {1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58};
    G4double absScint[8]     = {3.8*m, 3.8*m, 3.8*m, 3.8*m, 3.8*m, 3.8*m, 3.8*m, 3.8*m};
    G4double emissionEJ200[8]= {0.00, 0.05, 0.20, 0.60, 1.00, 0.50, 0.10, 0.00}; // Curva centrata a 2.9 eV
    
    G4MaterialPropertiesTable* mptScint = new G4MaterialPropertiesTable();
    mptScint->AddProperty("RINDEX", energy, rindexScint, nEntries);
    mptScint->AddProperty("ABSLENGTH", energy, absScint, nEntries);
    mptScint->AddProperty("SCINTILLATIONCOMPONENT1", energy, emissionEJ200, nEntries);
    mptScint->AddConstProperty("SCINTILLATIONYIELD", 10000./MeV);
    mptScint->AddConstProperty("RESOLUTIONSCALE", 1.0);
    mptScint->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 2.1*ns);
    ej200->SetMaterialPropertiesTable(mptScint);

    // -----------------------------------------------------------
    // B. Fibra WLS (Core): SAINT-GOBAIN BCF-92
    // Assorbe a ~410nm (>2.8eV), Emette a ~492nm (~2.5eV).
    // -----------------------------------------------------------
    G4double rindexCore[8]   = {1.60, 1.60, 1.60, 1.60, 1.60, 1.60, 1.60, 1.60};
    // WLSABSLENGTH: Trasparente al verde/rosso, opaco (assorbe forte) al blu/violetto
    G4double wlsAbsCore[8]   = {3.5*m, 3.5*m, 3.5*m, 1.0*cm, 0.1*mm, 0.1*mm, 0.1*mm, 0.1*mm}; 
    // Emissione BCF-92 (Picco sul verde 2.5 eV)
    G4double emissionBCF92[8]= {0.00, 0.20, 1.00, 0.30, 0.00, 0.00, 0.00, 0.00}; 
    // Attenuation length standard del polistirene alla propria luce
    G4double absCore[8]      = {3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m}; 

    G4MaterialPropertiesTable* mptCore = new G4MaterialPropertiesTable();
    mptCore->AddProperty("RINDEX", energy, rindexCore, nEntries);
    mptCore->AddProperty("ABSLENGTH", energy, absCore, nEntries);
    mptCore->AddProperty("WLSABSLENGTH", energy, wlsAbsCore, nEntries);
    mptCore->AddProperty("WLSCOMPONENT", energy, emissionBCF92, nEntries);
    mptCore->AddConstProperty("WLSMEANNUMBERPHOTONS", 0.86); // Resa quantica tipica (86%)
    mptCore->AddConstProperty("WLSTIMECONSTANT", 2.7*ns);
    coreMat->SetMaterialPropertiesTable(mptCore);

    // -----------------------------------------------------------
    // C. Fibra WLS (Cladding): PMMA (Acrilico)
    // -----------------------------------------------------------
    G4double rindexClad[8] = {1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49};
    G4double absClad[8]    = {10.0*m, 10.0*m, 10.0*m, 10.0*m, 10.0*m, 10.0*m, 10.0*m, 10.0*m};
    
    G4MaterialPropertiesTable* mptClad = new G4MaterialPropertiesTable();
    mptClad->AddProperty("RINDEX", energy, rindexClad, nEntries);
    mptClad->AddProperty("ABSLENGTH", energy, absClad, nEntries);
    cladMat->SetMaterialPropertiesTable(mptClad);

    G4double rindexGlue[8] = {1.56, 1.56, 1.56, 1.56, 1.56, 1.56, 1.56, 1.56};
    G4double absGlue[8]    = {5.0*m, 5.0*m, 5.0*m, 5.0*m, 5.0*m, 5.0*m, 5.0*m, 5.0*m};
    
    G4MaterialPropertiesTable* mptGlue = new G4MaterialPropertiesTable();
    mptGlue->AddProperty("RINDEX", energy, rindexGlue, nEntries);
    mptGlue->AddProperty("ABSLENGTH", energy, absGlue, nEntries);
    glueMat->SetMaterialPropertiesTable(mptGlue);

    // -----------------------------------------------------------
    // D. Aria (Riflessione esterna per la Total Internal Reflection)
    // -----------------------------------------------------------
    G4double rindexAir[8] = {1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00};
    G4MaterialPropertiesTable* mptAir = new G4MaterialPropertiesTable();
    mptAir->AddProperty("RINDEX", energy, rindexAir, nEntries);
    air->SetMaterialPropertiesTable(mptAir);

    // --- 3. ATTRIBUTI VISIVI ---
    G4VisAttributes* scintVis  = new G4VisAttributes(G4Colour(0.2, 0.4, 0.6, 0.3));
    G4VisAttributes* cladVis   = new G4VisAttributes(G4Colour(1.0, 0.5, 0.0, 1.0));
    cladVis->SetForceSolid(true);
    G4VisAttributes* coreVis   = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0, 1.0));
    G4VisAttributes* sipmVis   = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 1.0));
    G4VisAttributes* connVis   = new G4VisAttributes(G4Colour(0.3, 0.3, 0.3, 0.8));
    G4VisAttributes* glueVis = new G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.3));

    // --- 4. GEOMETRIA BASE ---
    G4Box* solidWorld = new G4Box("World", 2*m, 2*m, 2*m);
    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, air, "World");
    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());
    G4VPhysicalVolume* physWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0);

    G4double scintHalfZ = 25.0*mm; 
    G4Box* solidScint = new G4Box("ScintBox", 625*mm, 275*mm, scintHalfZ);
    G4LogicalVolume* logicScint = new G4LogicalVolume(solidScint, ej200, "ScintLogic");
    logicScint->SetVisAttributes(scintVis);
    new G4PVPlacement(0, G4ThreeVector(), logicScint, "ScintPhys", logicWorld, false, 0, true);

    // --- 5. PARAMETRI E GENERATORI FIBRE ---
    G4double glueR  = 0.6*mm;
    G4double fiberR = 0.5*mm; 
    G4double coreR  = 0.47*mm;

    auto makeStraight = [&](G4double len, G4String name) {
        G4Tubs* sGlue = new G4Tubs(name+"_SGlue", 0, glueR, len/2.0, 0, 360*deg);
        G4Tubs* sClad = new G4Tubs(name+"_S", 0, fiberR, len/2.0, 0, 360*deg);
        G4Tubs* sCore = new G4Tubs(name+"_C", 0, coreR, len/2.0, 0, 360*deg);
	G4LogicalVolume* lGlue = new G4LogicalVolume(sGlue, glueMat, name+"_LGlue");
        G4LogicalVolume* lClad = new G4LogicalVolume(sClad, cladMat, name+"_LClad");
        G4LogicalVolume* lCore = new G4LogicalVolume(sCore, coreMat, name+"_LCore");
        lGlue->SetVisAttributes(glueVis); lClad->SetVisAttributes(cladVis); lCore->SetVisAttributes(coreVis);
        new G4PVPlacement(0, G4ThreeVector(), lCore, name+"_PCore", lClad, false, 0, true);
	new G4PVPlacement(0, G4ThreeVector(), lClad, name+"_PClad", lGlue, false, 0, true);
        return lGlue;
    };

    auto makeArc = [&](G4double bendR, G4double startAngle, G4double sweepAngle, G4String name) {
        G4Torus* sGlue = new G4Torus(name+"_SGlue", 0, glueR, bendR, startAngle, sweepAngle);
        G4Torus* sClad = new G4Torus(name+"_S", 0, fiberR, bendR, startAngle, sweepAngle);
        G4Torus* sCore = new G4Torus(name+"_C", 0, coreR,  bendR, startAngle, sweepAngle);
	G4LogicalVolume* lGlue = new G4LogicalVolume(sGlue, glueMat, name+"_LGlue");
        G4LogicalVolume* lClad = new G4LogicalVolume(sClad, cladMat, name+"_LClad");
        G4LogicalVolume* lCore = new G4LogicalVolume(sCore, coreMat, name+"_LCore");
        lGlue->SetVisAttributes(glueVis); lClad->SetVisAttributes(cladVis); lCore->SetVisAttributes(coreVis);
        new G4PVPlacement(0, G4ThreeVector(), lCore, name+"_PCore", lClad, false, 0, true);
	new G4PVPlacement(0, G4ThreeVector(), lClad, name+"_PClad", lGlue, false, 0, true);
        return lGlue;
    };

    G4RotationMatrix* rotY90  = new G4RotationMatrix(); rotY90->rotateY(90*deg);
    G4RotationMatrix* rotX180 = new G4RotationMatrix(); rotX180->rotateX(180*deg);

    G4double straightL = 1010.0*mm; 
    G4LogicalVolume* lF_Str = makeStraight(straightL, "F_Str");
    G4LogicalVolume* lNude  = makeStraight(50*mm, "NudeFiber");

    // Coordinate chiave
    G4double yPairs[4][2] = {
        { 210*mm,   90*mm}, // Fibra 0
        { 150*mm,   30*mm}, // Fibra 1
        {-30*mm,  -150*mm}, // Fibra 2
        {-90*mm,  -210*mm}  // Fibra 3
    };
    G4double ySiPM[4]  = {30*mm, 10*mm, -10*mm, -30*mm};
    G4double xRight    = 505*mm;
    G4double xLeft     = -505*mm;
    G4double xScintEnd = -625*mm;

    // --- 6. CONNECTOR BOX ESTESA ---
    G4Box* sConn = new G4Box("Connector", 25*mm, 60*mm, 15*mm); 
    G4LogicalVolume* lConn = new G4LogicalVolume(sConn, blackHDPE, "lConn");
    lConn->SetVisAttributes(connVis);
    new G4PVPlacement(0, G4ThreeVector(-650*mm, 0, 20.5*mm), lConn, "pConn", logicWorld, false, 0, true);

    // --- 7. ROUTING CON STRATIFICAZIONE Z ---
    for (int f = 0; f < 4; f++) {
        G4double yUp  = yPairs[f][0]; 
        G4double yLow = yPairs[f][1];
        G4double zFib = (f % 2 == 0) ? 21.0*mm : 19.5*mm; 

        // A. DESTRA: U-Turns
        new G4PVPlacement(rotY90, G4ThreeVector(0, yUp, zFib), lF_Str, "pStrUp", logicScint, false, f, true);
        new G4PVPlacement(rotY90, G4ThreeVector(0, yLow, zFib), lF_Str, "pStrLow", logicScint, false, f, true);
        
        G4double rU = (yUp - yLow) / 2.0; 
        G4LogicalVolume* lUturn = makeArc(rU, 270*deg, 180*deg, "F_UTurn_" + std::to_string(f));
        new G4PVPlacement(0, G4ThreeVector(xRight, (yUp+yLow)/2.0, zFib), lUturn, "pCur", logicScint, false, f, true);

        // B. SINISTRA: S-BENDS
        for (int leg = 0; leg < 2; leg++) {
            G4double yStart = (leg == 0) ? yUp : yLow;
            G4double yEnd   = ySiPM[f] + ((leg == 0) ? 0.5*mm : -0.5*mm); 
            
            G4double dx = std::abs(xScintEnd - xLeft); 
            G4double dy = yEnd - yStart;

            if (std::abs(dy) < 0.1*mm) {
                G4LogicalVolume* lTail = makeStraight(dx, "F_Tail");
                new G4PVPlacement(rotY90, G4ThreeVector(xLeft - dx/2.0, yEnd, zFib), lTail, "pTail", logicScint, false, f, true);
            } else {
                G4double rBend = (dx*dx + dy*dy) / (4.0 * std::abs(dy));
                G4double theta = std::acos(1.0 - std::abs(dy) / (2.0 * rBend));

                G4LogicalVolume *lArc1, *lArc2;
                G4RotationMatrix *rot1 = nullptr, *rot2 = nullptr;
                G4double yCenter1, yCenter2;

                if (dy > 0) { 
                    lArc1 = makeArc(rBend, 90*deg, theta, "Arc1");
                    rot1 = rotX180; 
                    yCenter1 = yStart + rBend;

                    lArc2 = makeArc(rBend, 90*deg - theta, theta, "Arc2");
                    rot2 = nullptr;
                    yCenter2 = yEnd - rBend;
                } else { 
                    lArc1 = makeArc(rBend, 90*deg, theta, "Arc1");
                    rot1 = nullptr;
                    yCenter1 = yStart - rBend;

                    lArc2 = makeArc(rBend, 90*deg - theta, theta, "Arc2");
                    rot2 = rotX180; 
                    yCenter2 = yEnd + rBend;
                }

                new G4PVPlacement(rot1, G4ThreeVector(xLeft, yCenter1, zFib), lArc1, "pArc1", logicScint, false, f, true);
                new G4PVPlacement(rot2, G4ThreeVector(xScintEnd, yCenter2, zFib), lArc2, "pArc2", logicScint, false, f, true);
            }

            // C. Inseriamo la fibra nuda nel connettore
            new G4PVPlacement(rotY90, G4ThreeVector(0, yEnd, zFib - 20.5*mm), lNude, "pNude", lConn, false, f*2+leg, true);
        }
    }

    // --- 8. SiPMs ---
    G4Box* sSiPM = new G4Box("SiPM", 0.5*mm, 1.5*mm, 1.5*mm); 
    for (int f = 0; f < 4; f++) {
        G4LogicalVolume* lSiPM = new G4LogicalVolume(sSiPM, sipmMat, "LogicSiPM_" + std::to_string(f));
        lSiPM->SetVisAttributes(sipmVis);
        new G4PVPlacement(0, G4ThreeVector(-675.5*mm, ySiPM[f], 20.25*mm), lSiPM, "pSiPM", logicWorld, false, f, true);
    }

    return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
    G4SDManager* sdM = G4SDManager::GetSDMpointer();
    for (int i = 0; i < 4; i++) {
        SIPMSD* sd = new SIPMSD("SiPM_SD_" + std::to_string(i));
        sdM->AddNewDetector(sd);
        SetSensitiveDetector("LogicSiPM_" + std::to_string(i), sd);
    }
}
