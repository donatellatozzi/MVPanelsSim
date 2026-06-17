#include "DetectorConstruction.hh"
#include "DetectorMessenger.hh"
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
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4SubtractionSolid.hh"

DetectorConstruction::DetectorConstruction() : G4VUserDetectorConstruction() {
  fWrappingType = "Tyvek"; 
  fGeometryType = "Standard";
  fMessenger = new DetectorMessenger(this);
}

DetectorConstruction::~DetectorConstruction(){
  delete fMessenger;
}

void DetectorConstruction::SetGeometryType(G4String val) {
    fGeometryType = val;
    G4cout << "----> ORA LA GEOMETRIA E': " << fGeometryType << G4endl;
}

void DetectorConstruction::SetWrappingType(G4String val) {
    fWrappingType = val;
    G4cout << "----> ORA IL WRAPPING OTTICO E': " << fWrappingType << G4endl;
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    DefineMaterials();

    if (fGeometryType == "Standard") {
        return ConstructStandardPanel();
    }
    else if (fGeometryType == "Spiral") {
        return ConstructSpiralPanel();
    }
    else if (fGeometryType == "Nested") {
        return ConstructNestedPanel();
    }
    else {
        G4cerr << "Errore: Tipo di geometria sconosciuto!" << G4endl;
        return nullptr;
    }
}

G4VPhysicalVolume* DetectorConstruction::ConstructStandardPanel() {
    G4Material* air       = G4Material::GetMaterial("G4_AIR");
    G4Material* ej200     = G4Material::GetMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    G4Material* tyvekMat  = G4Material::GetMaterial("G4_POLYETHYLENE");
    G4Material* blackMat  = G4Material::GetMaterial("G4_POLYETHYLENE"); // Black HDPE film
    G4Material* sipmMat   = G4Material::GetMaterial("G4_Si");
    G4Material* coreMat   = G4Material::GetMaterial("G4_POLYSTYRENE");
    G4Material* cladMat   = G4Material::GetMaterial("G4_PLEXIGLASS");
    G4Material* glueMat   = G4Material::GetMaterial("G4_PLEXIGLASS");

    G4VisAttributes* scintVis = new G4VisAttributes(G4Colour(0.2, 0.4, 0.6, 0.2));
    G4VisAttributes* coreVis  = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0, 1.0));
    coreVis->SetVisibility(true); coreVis->SetForceSolid(true);
    G4VisAttributes* cladVis  = new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.1));
    cladVis->SetVisibility(false);
    G4VisAttributes* glueVis  = new G4VisAttributes(G4Colour(0.7, 0.7, 0.7, 0.0));
    glueVis->SetVisibility(false);
    G4VisAttributes* sipmVis  = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 1.0));
    G4VisAttributes* tyvekVis = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0, 0.3));
    G4VisAttributes* blackVis = new G4VisAttributes(G4Colour(0.1, 0.1, 0.1, 0.3));

    G4double scintX = 625.0*mm, scintY = 275.0*mm, scintZ = 25.0*mm;
    G4double thTyvek = 0.72*mm, thBlack = 0.42*mm;

    G4double blackX = scintX + thTyvek + thBlack;
    G4double blackY = scintY + thTyvek + thBlack;
    G4double blackZ = scintZ + thTyvek + thBlack;
    G4VSolid* sBlack = new G4Box("BlackTape_Solid", blackX, blackY, blackZ);

    G4double tyX = scintX + thTyvek;
    G4double tyY = scintY + thTyvek;
    G4double tyZ = scintZ + thTyvek;
    G4VSolid* sTyvek = new G4Box("Tyvek_Solid", tyX, tyY, tyZ);

    G4double holeR = 0.500*mm; 
    G4double holeLen = 2.0*mm; 
    G4Tubs* sHole = new G4Tubs("Hole_Solid", 0, holeR, holeLen, 0, 360*deg);
    G4RotationMatrix* rotHole = new G4RotationMatrix();
    rotHole->rotateY(90*deg);

    G4double yExitPairs[4][2] = { {210*mm, 90*mm}, {150*mm, 30*mm}, {-30*mm, -150*mm}, {-90*mm, -210*mm} };
    G4double xScintEnd = -625.0*mm; 

    for (int f = 0; f < 4; f++) {
        G4double zFib = (f % 2 == 0) ? 21.0*mm : 19.5*mm; 
        for (int leg = 0; leg < 2; leg++) {
            G4double yExit = yExitPairs[f][leg];
            G4ThreeVector holePos(xScintEnd, yExit, zFib);
            sTyvek = new G4SubtractionSolid("Tyvek_Hole", sTyvek, sHole, rotHole, holePos);
            sBlack = new G4SubtractionSolid("Black_Hole", sBlack, sHole, rotHole, holePos);
        }
    }

    G4Box* solidWorld = new G4Box("World", 3*m, 3*m, 3*m);
    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, air, "World");
    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());
    G4VPhysicalVolume* physWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0);

    G4LogicalVolume* lBlack = new G4LogicalVolume(sBlack, blackMat, "LogicBlack");
    lBlack->SetVisAttributes(blackVis);
    new G4PVPlacement(0, G4ThreeVector(), lBlack, "PhysBlack", logicWorld, false, 0, true);

    G4LogicalVolume* lTyvek = new G4LogicalVolume(sTyvek, tyvekMat, "LogicTyvek");
    lTyvek->SetVisAttributes(tyvekVis);
    new G4PVPlacement(0, G4ThreeVector(), lTyvek, "PhysTyvek", lBlack, false, 0, true);
    new G4LogicalSkinSurface("TyvekSkin", lTyvek, fOpWrapping);

    // --- SUPERFICIE OTTICA PER IL NASTRO NERO (Standard Panel) ---
    G4int nEntries = 8;
    G4double energy[8] = {2.00*eV, 2.30*eV, 2.50*eV, 2.70*eV, 2.90*eV, 3.10*eV, 3.30*eV, 3.50*eV};

    G4OpticalSurface* opBlack = new G4OpticalSurface("BlackSurface");
    opBlack->SetType(dielectric_metal);
    opBlack->SetFinish(ground);
    opBlack->SetModel(unified);
    G4MaterialPropertiesTable* mptBlack = new G4MaterialPropertiesTable();
    G4double refBlack[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; // Assorbitore perfetto
    mptBlack->AddProperty("REFLECTIVITY", energy, refBlack, nEntries);
    opBlack->SetMaterialPropertiesTable(mptBlack);
    new G4LogicalSkinSurface("BlackSkin", lBlack, opBlack);

    G4Box* solidScint = new G4Box("ScintBox", scintX, scintY, scintZ);
    G4LogicalVolume* logicScint = new G4LogicalVolume(solidScint, ej200, "ScintLogic");
    logicScint->SetVisAttributes(scintVis);
    new G4PVPlacement(0, G4ThreeVector(), logicScint, "ScintPhys", lTyvek, false, 0, true);

    G4double glueR = 0.600*mm, cladR = 0.495*mm, coreR = 0.480*mm;

    G4Tubs* sPlug = new G4Tubs("Plug_Solid", cladR, holeR, holeLen/2.0, 0, 360*deg);
    G4LogicalVolume* lPlug = new G4LogicalVolume(sPlug, blackMat, "Plug_Logic");
    lPlug->SetVisAttributes(blackVis);

    for (int f = 0; f < 4; f++) {
        G4double zFib = (f % 2 == 0) ? 21.0*mm : 19.5*mm; 
        for (int leg = 0; leg < 2; leg++) {
            G4double yExit = yExitPairs[f][leg];
            G4ThreeVector holePos(xScintEnd, yExit, zFib);
            new G4PVPlacement(rotHole, holePos, lPlug, "PhysPlug", logicWorld, false, 0, true);
        }
    }

    auto makeStraight = [&](G4double len, G4String name) {
        G4Tubs* sGlue = new G4Tubs(name+"_SGlue", 0, glueR, len/2.0, 0, 360*deg);
        G4Tubs* sClad = new G4Tubs(name+"_SClad", 0, cladR, len/2.0, 0, 360*deg); 
        G4Tubs* sCore = new G4Tubs(name+"_SCore", 0, coreR, len/2.0, 0, 360*deg);     
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
        G4Torus* sClad = new G4Torus(name+"_SClad", 0, cladR, bendR, startAngle, sweepAngle);
        G4Torus* sCore = new G4Torus(name+"_SCore", 0, coreR, bendR, startAngle, sweepAngle);
        G4LogicalVolume* lGlue = new G4LogicalVolume(sGlue, glueMat, name+"_LGlue");
        G4LogicalVolume* lClad = new G4LogicalVolume(sClad, cladMat, name+"_LClad");
        G4LogicalVolume* lCore = new G4LogicalVolume(sCore, coreMat, name+"_LCore");
        lGlue->SetVisAttributes(glueVis); lClad->SetVisAttributes(cladVis); lCore->SetVisAttributes(coreVis);
        new G4PVPlacement(0, G4ThreeVector(), lCore, name+"_PCore", lClad, false, 0, true);
        new G4PVPlacement(0, G4ThreeVector(), lClad, name+"_PClad", lGlue, false, 0, true);
        return lGlue;
    };

    auto makeNudeArc = [&](G4double bendR, G4double startAngle, G4double sweepAngle, G4String name) {
        G4Torus* sClad = new G4Torus(name+"_SClad", 0, cladR, bendR, startAngle, sweepAngle);
        G4Torus* sCore = new G4Torus(name+"_SCore", 0, coreR, bendR, startAngle, sweepAngle);
        G4LogicalVolume* lClad = new G4LogicalVolume(sClad, cladMat, name+"_LClad");
        G4LogicalVolume* lCore = new G4LogicalVolume(sCore, coreMat, name+"_LCore");
        lClad->SetVisAttributes(cladVis); lCore->SetVisAttributes(coreVis);
        new G4PVPlacement(0, G4ThreeVector(), lCore, name+"_PCore", lClad, false, 0, true);
        return lClad; 
    };

    auto makeNudeStraight = [&](G4double len, G4String name) {
        G4Tubs* sClad = new G4Tubs(name+"_SClad", 0, cladR, len/2.0, 0, 360*deg);
        G4Tubs* sCore = new G4Tubs(name+"_SCore", 0, coreR, len/2.0, 0, 360*deg);
        G4LogicalVolume* lClad = new G4LogicalVolume(sClad, cladMat, name+"_LClad");
        G4LogicalVolume* lCore = new G4LogicalVolume(sCore, coreMat, name+"_LCore");
        lClad->SetVisAttributes(cladVis); lCore->SetVisAttributes(coreVis);
        new G4PVPlacement(0, G4ThreeVector(), lCore, name+"_PCore", lClad, false, 0, true);
        return lClad;
    };

    G4RotationMatrix* rotY90  = new G4RotationMatrix(); rotY90->rotateY(90*deg);
    G4RotationMatrix* rotX180 = new G4RotationMatrix(); rotX180->rotateX(180*deg);

    // Distanza pannello -> SiPM = bridgeLen + dxBundle + straightFinal = 200.0 mm (20 cm), come richiesto.
    G4double dxBundle         = 170.0*mm, straightFinal    = 20.0*mm;
    G4double sipmThick        = 0.5*mm, bridgeLen        = 10.0*mm;

    G4double xBundleEnd = xScintEnd - bridgeLen - dxBundle; 
    G4double xSiPM      = xBundleEnd - straightFinal - sipmThick;

    G4Box* sSiPM = new G4Box("SiPM", sipmThick, 3.0*mm, 3.0*mm); 
    G4LogicalVolume* lSiPM = new G4LogicalVolume(sSiPM, sipmMat, "LogicSiPM_All");
    lSiPM->SetVisAttributes(sipmVis);

    // --- SUPERFICIE OTTICA SiPM (Standard Panel) ---
    G4OpticalSurface* opSiPM = new G4OpticalSurface("SiPMSurface");
    opSiPM->SetType(dielectric_metal);
    opSiPM->SetFinish(polished);
    opSiPM->SetModel(unified);
    G4MaterialPropertiesTable* mptSiPM = new G4MaterialPropertiesTable();
    G4double refSiPM[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
    G4double effSiPM[8] = {0.15, 0.25, 0.30, 0.38, 0.40, 0.35, 0.25, 0.10}; // PDE Hamamatsu S13360
    mptSiPM->AddProperty("REFLECTIVITY", energy, refSiPM, nEntries);
    mptSiPM->AddProperty("EFFICIENCY", energy, effSiPM, nEntries);
    opSiPM->SetMaterialPropertiesTable(mptSiPM);
    new G4LogicalSkinSurface("SiPMSkin", lSiPM, opSiPM);

    new G4PVPlacement(0, G4ThreeVector(xSiPM, 0, 20.25*mm), lSiPM, "pSiPM", logicWorld, false, 0, true);

    int fiberCount = 0;

    for (int f = 0; f < 4; f++) {
        G4double yUp  = yExitPairs[f][0]; 
        G4double yLow = yExitPairs[f][1];
        G4double zFib = (f % 2 == 0) ? 21.0*mm : 19.5*mm; 
        
        G4double rU = (yUp - yLow) / 2.0; 
        G4double xBackOfU = 625.0*mm - 20.0*mm; 
        G4double xUturnCenter = xBackOfU - rU;

        G4LogicalVolume* lUturn = makeArc(rU, 270*deg, 180*deg, "F_UTurn_" + std::to_string(f));
        new G4PVPlacement(0, G4ThreeVector(xUturnCenter, (yUp+yLow)/2.0, zFib), lUturn, "pCur", logicScint, false, f, true);

        G4double straightLen = xUturnCenter - xScintEnd;
        G4LogicalVolume* lF_Str_Dyn = makeStraight(straightLen, "F_Str_" + std::to_string(f));
        G4double xStraightCenter = (xUturnCenter + xScintEnd) / 2.0;
        new G4PVPlacement(rotY90, G4ThreeVector(xStraightCenter, yUp, zFib), lF_Str_Dyn, "pStrUp", logicScint, false, f, true);
        new G4PVPlacement(rotY90, G4ThreeVector(xStraightCenter, yLow, zFib), lF_Str_Dyn, "pStrLow", logicScint, false, f, true);

        for (int leg = 0; leg < 2; leg++) {
            G4double yStart = (leg == 0) ? yUp : yLow;
            G4double yTarget = (3.5 - fiberCount) * 1.0*mm; 
            fiberCount++;

            G4LogicalVolume* lBridge = makeNudeStraight(bridgeLen, "Bridge");
            new G4PVPlacement(rotY90, G4ThreeVector(xScintEnd - bridgeLen/2.0, yStart, zFib), lBridge, "pBridge", logicWorld, false, f*2+leg, true);

            G4double dy = std::abs(yStart - yTarget);
            G4double dx = dxBundle;
            G4double rBend = (dx*dx + dy*dy) / (4.0 * dy);
            G4double theta = std::acos(1.0 - dy / (2.0 * rBend));

            G4LogicalVolume *lArc1, *lArc2;
            G4double xBendStart = xScintEnd - bridgeLen; 

            G4RotationMatrix* rotArc1 = new G4RotationMatrix(); 
            G4RotationMatrix* rotArc2 = new G4RotationMatrix(); 
            rotArc2->rotateY(180*deg);

            if (yStart < yTarget) { 
                rotArc1->rotateX(180*deg);
                rotArc2->rotateX(180*deg); 
            }

            lArc1 = makeNudeArc(rBend, 90*deg, theta, "NA1");
            G4double yCenter1 = (yStart > yTarget) ? (yStart - rBend) : (yStart + rBend);
            new G4PVPlacement(rotArc1, G4ThreeVector(xBendStart, yCenter1, zFib), lArc1, "pNA1", logicWorld, false, 0, true);

            lArc2 = makeNudeArc(rBend, 270*deg - theta, theta, "NA2");
            G4double yCenter2 = (yStart > yTarget) ? (yTarget + rBend) : (yTarget - rBend);
            new G4PVPlacement(rotArc2, G4ThreeVector(xBundleEnd, yCenter2, zFib), lArc2, "pNA2", logicWorld, false, 0, true);

            G4LogicalVolume* lFinal = makeNudeStraight(straightFinal, "Final");
            G4double xFinalCenter = xBundleEnd - straightFinal/2.0;
            new G4PVPlacement(rotY90, G4ThreeVector(xFinalCenter, yTarget, zFib), lFinal, "pFinal", logicWorld, false, 0, true);
        }
    }

    return physWorld;
}

G4VPhysicalVolume* DetectorConstruction::ConstructNestedPanel() {
    // --- 1. MATERIALI (identici allo Standard) ---
    G4Material* air       = G4Material::GetMaterial("G4_AIR");
    G4Material* ej200     = G4Material::GetMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    G4Material* tyvekMat  = G4Material::GetMaterial("G4_POLYETHYLENE");
    G4Material* blackMat  = G4Material::GetMaterial("G4_POLYETHYLENE"); // Black HDPE film
    G4Material* sipmMat   = G4Material::GetMaterial("G4_Si");
    G4Material* coreMat   = G4Material::GetMaterial("G4_POLYSTYRENE");
    G4Material* cladMat   = G4Material::GetMaterial("G4_PLEXIGLASS");
    G4Material* glueMat   = G4Material::GetMaterial("G4_PLEXIGLASS");

    // --- 2. VIS ATTRIBUTES (identici allo Standard) ---
    G4VisAttributes* scintVis = new G4VisAttributes(G4Colour(0.2, 0.4, 0.6, 0.2));
    G4VisAttributes* coreVis  = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0, 1.0));
    coreVis->SetVisibility(true); coreVis->SetForceSolid(true);
    G4VisAttributes* cladVis  = new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.1));
    cladVis->SetVisibility(false);
    G4VisAttributes* glueVis  = new G4VisAttributes(G4Colour(0.7, 0.7, 0.7, 0.0));
    glueVis->SetVisibility(false);
    G4VisAttributes* sipmVis  = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 1.0));
    G4VisAttributes* tyvekVis = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0, 0.3));
    G4VisAttributes* blackVis = new G4VisAttributes(G4Colour(0.1, 0.1, 0.1, 0.3));

    // --- 3. GEOMETRIA BASE: stesse dimensioni del pannello Standard ---
    G4double scintX = 625.0*mm, scintY = 275.0*mm, scintZ = 25.0*mm;
    G4double thTyvek = 0.72*mm, thBlack = 0.42*mm;

    G4double blackX = scintX + thTyvek + thBlack;
    G4double blackY = scintY + thTyvek + thBlack;
    G4double blackZ = scintZ + thTyvek + thBlack;
    G4VSolid* sBlack = new G4Box("BlackTape_Solid_N", blackX, blackY, blackZ);

    G4double tyX = scintX + thTyvek;
    G4double tyY = scintY + thTyvek;
    G4double tyZ = scintZ + thTyvek;
    G4VSolid* sTyvek = new G4Box("Tyvek_Solid_N", tyX, tyY, tyZ);

    G4double holeR = 0.500*mm;
    G4double holeLen = 2.0*mm;
    G4Tubs* sHole = new G4Tubs("Hole_Solid_N", 0, holeR, holeLen, 0, 360*deg);
    G4RotationMatrix* rotHole = new G4RotationMatrix();
    rotHole->rotateY(90*deg);

    G4double xScintEnd = -625.0*mm;

    // --- 4. LE 4 FIBRE (A,B in meta' superiore; C,D in meta' inferiore) ---
    // Ogni fibra ha 2 capi ("out" = lontano dal centro, "in" = vicino al centro)
    // collegati da una curva a U lontana (raggio 60mm, lato destro, come nello Standard).
    // Le posizioni nominali (prima di ogni curva di convergenza) sono spaziate 60mm
    // l'una dall'altra: 210, 150, 90, 30 mm (e i mirror -30,-90,-150,-210) -- gli
    // stessi valori dello Standard (yExitPairs), che danno rU = 60mm per ciascuna
    // delle 4 curve a U lontane.
    //
    // I 4 fori sono allineati al centro del pannello, distanziati 20mm l'uno
    // dall'altro, con il primo a 245mm dal bordo superiore e l'ultimo a 245mm dal
    // bordo inferiore: Y = +30, +10, -10, -30 mm. Connessioni confermate dal disegno:
    //   foro1 (+30) = B_in (dritto, coincide col foro) + C_in (curva, dall'altra meta')
    //   foro4 (-30) = D_in (dritto, coincide col foro) + A_in (curva, dall'altra meta')
    //   foro2 (+10) = A_out (curva) + B_out (curva) -- stessa meta' (superiore)
    //   foro3 (-10) = C_out (curva) + D_out (curva) -- stessa meta' (inferiore)
    // A_in/C_in raggiungono il loro foro con dy=120mm, raggio 60mm -> dx=120mm esatti
    // (caso pulito). A_out/C_out (dy=200mm) e B_out/D_out (dy=140mm) usano lo stesso
    // raggio fisso 60mm applicato direttamente al foro finale (senza una tappa
    // intermedia separata, per semplicita' di costruzione).
    //
    // PROBLEMA GEOMETRICO: B_out e D_out devono "scavalcare" la propria compagna di
    // fibra (B_in/D_in), che resta dritta esattamente sulla Y che B_out/D_out devono
    // attraversare per tutta la lunghezza del pannello. A_out/C_out invece NON hanno
    // questo problema (l'ordine con A_in/C_in si mantiene naturalmente). Soluzione per
    // B_out/D_out (confermata): un breve salto diagonale in profondita' (Z) PRIMA di
    // iniziare la curva in Y, cosi' la curva avviene a una Z diversa da quella della
    // compagna (zHome = Z alla curva a U; z = Z effettiva al foro, dopo il salto).
    struct FiberLeg {
        G4String name;
        G4double yNominal; // Y lungo la corsa rettilinea principale (dopo la curva a U)
        G4double yHole;    // Y effettiva al foro condiviso con la fibra "compagna"
        G4double zHome;    // Z alla curva a U lontana (= Z della fibra, condivisa coi due capi)
        G4double z;        // Z effettiva al foro (= zHome se non serve saltare)
        G4double yTarget;  // posizione finale nel bundle esterno verso il SiPM
    };

    // Z base delle 4 fibre, centrate vicino alla mezzeria del pannello (spessore 50mm,
    // quindi +-25mm) per avere margine di salto sufficiente.
    G4double zA = 3.0*mm, zB = 1.0*mm, zC = -1.0*mm, zD = -3.0*mm;
    // Z "di salto" dei due soli capi che devono scavalcare la propria compagna.
    G4double zBOutJog = -13.0*mm, zDOutJog = 13.0*mm;

    // yTarget e' assegnato per gruppo di foro, mantenendo l'ordine decrescente
    // foro1(+30) > foro2(+10) > foro3(-10) > foro4(-30): cosi' i due capi della
    // STESSA fibra che condividono la stessa Z (A_out/A_in e C_out/C_in -- B e D
    // sono gia' protette dal salto in Z) non si incrociano nel bundle esterno.
    std::vector<FiberLeg> legs = {
        {"A_out", 210.0*mm,   10.0*mm, zA, zA,         1.5*mm},  // -> foro2 (= B_out)
        {"A_in",   90.0*mm,  -30.0*mm, zA, zA,        -2.5*mm},  // -> foro4 (= D_in, dritto)
        {"B_out", 150.0*mm,   10.0*mm, zB, zBOutJog,   0.5*mm},  // -> foro2 (= A_out), salta in Z
        {"B_in",   30.0*mm,   30.0*mm, zB, zB,         3.5*mm},  // -> foro1 (dritto, = C_in)
        {"C_out",-210.0*mm,  -10.0*mm, zC, zC,        -0.5*mm},  // -> foro3 (= D_out)
        {"C_in", -90.0*mm,    30.0*mm, zC, zC,         2.5*mm},  // -> foro1 (= B_in, dritto)
        {"D_out",-150.0*mm,  -10.0*mm, zD, zDOutJog, -1.5*mm},  // -> foro3 (= C_out), salta in Z
        {"D_in", -30.0*mm,  -30.0*mm, zD, zD,         -3.5*mm},  // -> foro4 (dritto, = A_in)
    };

    // --- Fori (8 canali fisici, raggruppati in 4 posizioni X,Y condivise tra fibre diverse) ---
    for (const auto& leg : legs) {
        G4ThreeVector holePos(xScintEnd, leg.yHole, leg.z);
        sTyvek = new G4SubtractionSolid("Tyvek_Hole_N_"+leg.name, sTyvek, sHole, rotHole, holePos);
        sBlack = new G4SubtractionSolid("Black_Hole_N_"+leg.name, sBlack, sHole, rotHole, holePos);
    }

    G4Box* solidWorld = new G4Box("World", 3*m, 3*m, 3*m);
    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, air, "World");
    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());
    G4VPhysicalVolume* physWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0);

    G4LogicalVolume* lBlack = new G4LogicalVolume(sBlack, blackMat, "LogicBlack_N");
    lBlack->SetVisAttributes(blackVis);
    new G4PVPlacement(0, G4ThreeVector(), lBlack, "PhysBlack_N", logicWorld, false, 0, true);

    G4LogicalVolume* lTyvek = new G4LogicalVolume(sTyvek, tyvekMat, "LogicTyvek_N");
    lTyvek->SetVisAttributes(tyvekVis);
    new G4PVPlacement(0, G4ThreeVector(), lTyvek, "PhysTyvek_N", lBlack, false, 0, true);
    new G4LogicalSkinSurface("TyvekSkin_N", lTyvek, fOpWrapping);

    // --- SUPERFICIE OTTICA PER IL NASTRO NERO (identica allo Standard) ---
    G4int nEntries = 8;
    G4double energy[8] = {2.00*eV, 2.30*eV, 2.50*eV, 2.70*eV, 2.90*eV, 3.10*eV, 3.30*eV, 3.50*eV};

    G4OpticalSurface* opBlack = new G4OpticalSurface("BlackSurface_N");
    opBlack->SetType(dielectric_metal);
    opBlack->SetFinish(ground);
    opBlack->SetModel(unified);
    G4MaterialPropertiesTable* mptBlack = new G4MaterialPropertiesTable();
    G4double refBlack[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    mptBlack->AddProperty("REFLECTIVITY", energy, refBlack, nEntries);
    opBlack->SetMaterialPropertiesTable(mptBlack);
    new G4LogicalSkinSurface("BlackSkin_N", lBlack, opBlack);

    G4Box* solidScint = new G4Box("ScintBox_N", scintX, scintY, scintZ);
    G4LogicalVolume* logicScint = new G4LogicalVolume(solidScint, ej200, "ScintLogic_N");
    logicScint->SetVisAttributes(scintVis);
    new G4PVPlacement(0, G4ThreeVector(), logicScint, "ScintPhys_N", lTyvek, false, 0, true);

    // --- TAPPI ("Plug") di materiale nero in ogni foro (identici allo Standard) ---
    G4double glueR = 0.600*mm, cladR = 0.495*mm, coreR = 0.480*mm;
    G4Tubs* sPlug = new G4Tubs("Plug_Solid_N", cladR, holeR, holeLen/2.0, 0, 360*deg);
    G4LogicalVolume* lPlug = new G4LogicalVolume(sPlug, blackMat, "Plug_Logic_N");
    lPlug->SetVisAttributes(blackVis);
    for (const auto& leg : legs) {
        G4ThreeVector holePos(xScintEnd, leg.yHole, leg.z);
        new G4PVPlacement(rotHole, holePos, lPlug, "PhysPlug_N", logicWorld, false, 0, true);
    }

    // --- LAMBDA DI COSTRUZIONE FIBRA (stesso stile dello Standard) ---
    auto makeStraight = [&](G4double len, G4String name) {
        G4Tubs* sGlue = new G4Tubs(name+"_SGlue", 0, glueR, len/2.0, 0, 360*deg);
        G4Tubs* sClad = new G4Tubs(name+"_SClad", 0, cladR, len/2.0, 0, 360*deg);
        G4Tubs* sCore = new G4Tubs(name+"_SCore", 0, coreR, len/2.0, 0, 360*deg);
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
        G4Torus* sClad = new G4Torus(name+"_SClad", 0, cladR, bendR, startAngle, sweepAngle);
        G4Torus* sCore = new G4Torus(name+"_SCore", 0, coreR, bendR, startAngle, sweepAngle);
        G4LogicalVolume* lGlue = new G4LogicalVolume(sGlue, glueMat, name+"_LGlue");
        G4LogicalVolume* lClad = new G4LogicalVolume(sClad, cladMat, name+"_LClad");
        G4LogicalVolume* lCore = new G4LogicalVolume(sCore, coreMat, name+"_LCore");
        lGlue->SetVisAttributes(glueVis); lClad->SetVisAttributes(cladVis); lCore->SetVisAttributes(coreVis);
        new G4PVPlacement(0, G4ThreeVector(), lCore, name+"_PCore", lClad, false, 0, true);
        new G4PVPlacement(0, G4ThreeVector(), lClad, name+"_PClad", lGlue, false, 0, true);
        return lGlue;
    };

    auto makeNudeArc = [&](G4double bendR, G4double startAngle, G4double sweepAngle, G4String name) {
        G4Torus* sClad = new G4Torus(name+"_SClad", 0, cladR, bendR, startAngle, sweepAngle);
        G4Torus* sCore = new G4Torus(name+"_SCore", 0, coreR, bendR, startAngle, sweepAngle);
        G4LogicalVolume* lClad = new G4LogicalVolume(sClad, cladMat, name+"_LClad");
        G4LogicalVolume* lCore = new G4LogicalVolume(sCore, coreMat, name+"_LCore");
        lClad->SetVisAttributes(cladVis); lCore->SetVisAttributes(coreVis);
        new G4PVPlacement(0, G4ThreeVector(), lCore, name+"_PCore", lClad, false, 0, true);
        return lClad;
    };

    auto makeNudeStraight = [&](G4double len, G4String name) {
        G4Tubs* sClad = new G4Tubs(name+"_SClad", 0, cladR, len/2.0, 0, 360*deg);
        G4Tubs* sCore = new G4Tubs(name+"_SCore", 0, coreR, len/2.0, 0, 360*deg);
        G4LogicalVolume* lClad = new G4LogicalVolume(sClad, cladMat, name+"_LClad");
        G4LogicalVolume* lCore = new G4LogicalVolume(sCore, coreMat, name+"_LCore");
        lClad->SetVisAttributes(cladVis); lCore->SetVisAttributes(coreVis);
        new G4PVPlacement(0, G4ThreeVector(), lCore, name+"_PCore", lClad, false, 0, true);
        return lClad;
    };

    G4RotationMatrix* rotY90 = new G4RotationMatrix(); rotY90->rotateY(90*deg);

    // --- CURVA A U LONTANA (raggio 60mm, lato destro, identica in struttura allo Standard) ---
    G4double rU = 60.0*mm;
    G4double xBackOfU = scintX - 60.0*mm;     // margine di 60mm dal bordo destro (come nel disegno)
    G4double xUturnCenter = xBackOfU - rU;    // 505mm
    G4double straightLen = xUturnCenter - xScintEnd; // 1130mm, come nel disegno tecnico

    auto placeUturn = [&](G4double yOut, G4double yIn, G4double zLeg, G4String name) {
        G4double yMid = (yOut + yIn) / 2.0;
        G4LogicalVolume* lU = makeArc(rU, 270*deg, 180*deg, name);
        new G4PVPlacement(0, G4ThreeVector(xUturnCenter, yMid, zLeg), lU, name+"_P", logicScint, false, 0, true);
    };

    placeUturn( 210.0*mm,   90.0*mm, zA, "Nest_UA");
    placeUturn( 150.0*mm,   30.0*mm, zB, "Nest_UB");
    placeUturn(-210.0*mm,  -90.0*mm, zC, "Nest_UC");
    placeUturn(-150.0*mm,  -30.0*mm, zD, "Nest_UD");

    // --- GAMBE: tratto dritto principale + (se serve) salto in Z + curva di convergenza ---
    // I capi piu' interni (B_in, D_in) coincidono gia' con la Y del loro foro e restano
    // completamente dritti. A_in/C_in convergono con una curva pulita (raggio 60mm).
    // I capi esterni (A_out,B_out,C_out,D_out) devono invece "scavalcare" la propria
    // compagna di fibra (che occupa, dritta o in curva, la Y che loro stessi devono
    // attraversare): prima di curvare in Y fanno un breve salto diagonale in Z (zHome ->
    // z, "jogDx" di estensione orizzontale), poi la curva in Y avviene gia' alla Z
    // sicura, e restano a quella Z fino al foro.
    G4double rMerge = 60.0*mm; // raggio della curva di "incontro", come indicato sul disegno
    G4double jogDx  = 40.0*mm; // estensione orizzontale del salto diagonale in Z (angolo dolce)
    for (const auto& leg : legs) {
        G4double yStart = leg.yNominal, yTarget = leg.yHole;
        G4double dy = std::abs(yStart - yTarget);
        bool needsJog = std::abs(leg.z - leg.zHome) > 1.0e-9;

        if (dy < 1.0e-9 && !needsJog) {
            // Capo gia' allineato al foro e nessuna compagna da scavalcare: tutto dritto.
            G4LogicalVolume* lFull = makeStraight(straightLen, "Nest_"+leg.name);
            G4double xCenter = (xUturnCenter + xScintEnd) / 2.0;
            new G4PVPlacement(rotY90, G4ThreeVector(xCenter, yTarget, leg.z), lFull, "Nest_P"+leg.name, logicScint, false, 0, true);
            continue;
        }

        // Per un raggio fisso rMerge, la lunghezza (dx) della curva a S che produce
        // esattamente questo dy si ottiene invertendo dy = dx^2/(4*rMerge - dy)... cioe'
        // dalla stessa relazione usata altrove (rBend = (dx^2+dy^2)/(4*dy)) con rBend=rMerge.
        G4double dxCurve = (dy > 1.0e-9) ? std::sqrt(dy * (4.0*rMerge - dy)) : 0.0;
        G4double dxTotalEnd = dxCurve + (needsJog ? jogDx : 0.0);

        G4double preLen = straightLen - dxTotalEnd;
        if (preLen > 0.0) {
            G4LogicalVolume* lPre = makeStraight(preLen, "Nest_"+leg.name+"_Pre");
            G4double xPreCenter = xUturnCenter - preLen/2.0;
            new G4PVPlacement(rotY90, G4ThreeVector(xPreCenter, yStart, leg.zHome), lPre, "Nest_P"+leg.name+"_Pre", logicScint, false, 0, true);
        }

        if (needsJog) {
            // Salto diagonale dalla Z di casa (condivisa con la compagna) alla Z di
            // salto (libera), PRIMA che la curva in Y attraversi la zona occupata
            // dalla compagna: cosi' quella curva avviene gia' a una Z sicura.
            G4double xAfterPre = xUturnCenter - preLen; // = xScintEnd + dxTotalEnd
            G4double dz = leg.z - leg.zHome;
            G4double jogLen = std::sqrt(jogDx*jogDx + dz*dz);
            G4double jogAngle = std::atan2(-jogDx, dz);
            G4RotationMatrix* rotJog = new G4RotationMatrix();
            rotJog->rotateY(jogAngle);
            G4LogicalVolume* lJog = makeStraight(jogLen, "Nest_"+leg.name+"_ZJog");
            G4double xJogCenter = xAfterPre - jogDx/2.0;
            G4double zJogCenter = (leg.zHome + leg.z) / 2.0;
            new G4PVPlacement(rotJog, G4ThreeVector(xJogCenter, yStart, zJogCenter), lJog, "Nest_P"+leg.name+"_ZJog", logicScint, false, 0, true);
        }

        if (dy < 1.0e-9) continue; // solo salto in Z, nessuna curva in Y da fare

        G4double rBend = rMerge;
        G4double theta = std::acos(1.0 - dy/(2.0*rBend));
        G4double xCurveStart = xScintEnd + dxCurve;

        G4RotationMatrix* rotArc1 = new G4RotationMatrix();
        G4RotationMatrix* rotArc2 = new G4RotationMatrix();
        rotArc2->rotateY(180*deg);
        if (yStart < yTarget) {
            rotArc1->rotateX(180*deg);
            rotArc2->rotateX(180*deg);
        }

        G4LogicalVolume* lArc1 = makeArc(rBend, 90*deg, theta, "Nest_"+leg.name+"_M1");
        G4double yCenter1 = (yStart > yTarget) ? (yStart - rBend) : (yStart + rBend);
        new G4PVPlacement(rotArc1, G4ThreeVector(xCurveStart, yCenter1, leg.z), lArc1, "Nest_P"+leg.name+"_M1", logicScint, false, 0, true);

        G4LogicalVolume* lArc2 = makeArc(rBend, 270*deg - theta, theta, "Nest_"+leg.name+"_M2");
        G4double yCenter2 = (yStart > yTarget) ? (yTarget + rBend) : (yTarget - rBend);
        new G4PVPlacement(rotArc2, G4ThreeVector(xScintEnd, yCenter2, leg.z), lArc2, "Nest_P"+leg.name+"_M2", logicScint, false, 0, true);
    }

    // --- SiPM E SUA SUPERFICIE OTTICA (identici allo Standard) ---
    G4double sipmThick = 0.5*mm;
    // Distanza pannello -> SiPM = bridgeLen + dxBundle + straightFinal = 200.0 mm (20 cm).
    G4double bridgeLen = 10.0*mm, dxBundle = 170.0*mm, straightFinal = 20.0*mm;

    // L'estensione Z del SiPM deve coprire tutte le Z usate dagli 8 capi nel bundle
    // esterno (qui da -15mm a +15mm, per via dei salti in Z): 16mm di mezza altezza
    // basta, con un po' di margine. Il SiPM e' centrato su Z=0 (non piu' a 20.25mm
    // come nello Standard, che aveva fibre concentrate intorno a quella Z).
    G4Box* sSiPM = new G4Box("SiPM_N", sipmThick, 3.0*mm, 16.0*mm);
    G4LogicalVolume* lSiPM = new G4LogicalVolume(sSiPM, sipmMat, "LogicSiPM_All");
    lSiPM->SetVisAttributes(sipmVis);

    G4OpticalSurface* opSiPM = new G4OpticalSurface("SiPMSurface_N");
    opSiPM->SetType(dielectric_metal);
    opSiPM->SetFinish(polished);
    opSiPM->SetModel(unified);
    G4MaterialPropertiesTable* mptSiPM = new G4MaterialPropertiesTable();
    G4double refSiPM[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    G4double effSiPM[8] = {0.15, 0.25, 0.30, 0.38, 0.40, 0.35, 0.25, 0.10};
    mptSiPM->AddProperty("REFLECTIVITY", energy, refSiPM, nEntries);
    mptSiPM->AddProperty("EFFICIENCY", energy, effSiPM, nEntries);
    opSiPM->SetMaterialPropertiesTable(mptSiPM);
    new G4LogicalSkinSurface("SiPMSkin_N", lSiPM, opSiPM);

    G4double xBundleEnd = xScintEnd - bridgeLen - dxBundle;
    G4double xSiPM      = xBundleEnd - straightFinal - sipmThick;
    new G4PVPlacement(0, G4ThreeVector(xSiPM, 0, 0.0*mm), lSiPM, "pSiPM_N", logicWorld, false, 0, true);

    // --- BUNDLE ESTERNO: gli 8 capi (gia' accoppiati a 2 a 2 nei 4 fori) confluiscono nel SiPM ---
    for (size_t i = 0; i < legs.size(); i++) {
        G4double yStart  = legs[i].yHole;
        G4double zFib    = legs[i].z;
        G4double yTarget = legs[i].yTarget;
        G4String tag = "Nest_" + legs[i].name;

        G4LogicalVolume* lBridge = makeNudeStraight(bridgeLen, tag+"_Bridge");
        new G4PVPlacement(rotY90, G4ThreeVector(xScintEnd - bridgeLen/2.0, yStart, zFib), lBridge, "p"+tag+"_Bridge", logicWorld, false, 0, true);

        G4double dy = std::abs(yStart - yTarget);
        G4double dx = dxBundle;
        G4double rBend = (dx*dx + dy*dy) / (4.0*dy);
        G4double theta = std::acos(1.0 - dy/(2.0*rBend));
        G4double xBendStart = xScintEnd - bridgeLen;

        G4RotationMatrix* rotArc1 = new G4RotationMatrix();
        G4RotationMatrix* rotArc2 = new G4RotationMatrix();
        rotArc2->rotateY(180*deg);
        if (yStart < yTarget) {
            rotArc1->rotateX(180*deg);
            rotArc2->rotateX(180*deg);
        }

        G4LogicalVolume* lArc1 = makeNudeArc(rBend, 90*deg, theta, tag+"_NA1");
        G4double yCenter1 = (yStart > yTarget) ? (yStart - rBend) : (yStart + rBend);
        new G4PVPlacement(rotArc1, G4ThreeVector(xBendStart, yCenter1, zFib), lArc1, "p"+tag+"_NA1", logicWorld, false, 0, true);

        G4LogicalVolume* lArc2 = makeNudeArc(rBend, 270*deg - theta, theta, tag+"_NA2");
        G4double yCenter2 = (yStart > yTarget) ? (yTarget + rBend) : (yTarget - rBend);
        new G4PVPlacement(rotArc2, G4ThreeVector(xBundleEnd, yCenter2, zFib), lArc2, "p"+tag+"_NA2", logicWorld, false, 0, true);

        G4LogicalVolume* lFinal = makeNudeStraight(straightFinal, tag+"_Final");
        G4double xFinalCenter = xBundleEnd - straightFinal/2.0;
        new G4PVPlacement(rotY90, G4ThreeVector(xFinalCenter, yTarget, zFib), lFinal, "p"+tag+"_Final", logicWorld, false, 0, true);
    }

    return physWorld;
}

G4VPhysicalVolume* DetectorConstruction::ConstructSpiralPanel() {
    
    // --- 1. RECUPERO MATERIALI ---
    G4Material* air       = G4Material::GetMaterial("G4_AIR");
    G4Material* ej200     = G4Material::GetMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    G4Material* tyvekMat  = G4Material::GetMaterial("G4_POLYETHYLENE");
    G4Material* alMat     = G4Material::GetMaterial("G4_Al");
    G4Material* blackMat  = G4Material::GetMaterial("G4_POLYVINYL_CHLORIDE"); 
    G4Material* sipmMat   = G4Material::GetMaterial("G4_Si");

    // --- 2. VIS ATTRIBUTES ---
    G4VisAttributes* scintVis = new G4VisAttributes(G4Colour(0.2, 0.4, 0.6, 0.2));
    G4VisAttributes* sipmVis  = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 1.0));
    G4VisAttributes* tyvekVis = new G4VisAttributes(G4Colour(1.0, 1.0, 1.0, 0.3));
    G4VisAttributes* alVis    = new G4VisAttributes(G4Colour(0.7, 0.7, 0.7, 0.3));
    G4VisAttributes* blackVis = new G4VisAttributes(G4Colour(0.1, 0.1, 0.1, 0.3));
  
    // --- 3. GEOMETRIA BASE ---
    G4double scintX = 1000.0 * mm, scintY = 500.0 * mm, scintZ = 25.0 * mm;
    G4double thTyvek = 0.412 * mm, thAl = 0.1 * mm, thBlack = 0.2 * mm;

    G4double blackX = scintX + 2*(thTyvek + thAl + thBlack);
    G4double blackY = scintY + 2*(thTyvek + thAl + thBlack);
    G4double blackZ = scintZ + 2*(thTyvek + thAl + thBlack);

    G4double alX = scintX + 2*(thTyvek + thAl);
    G4double alY = scintY + 2*(thTyvek + thAl);
    G4double alZ = scintZ + 2*(thTyvek + thAl);

    G4double tyX = scintX + 2*thTyvek;
    G4double tyY = scintY + 2*thTyvek;
    G4double tyZ = scintZ + 2*thTyvek;

    G4Box* solidWorld = new G4Box("World", 3*m, 3*m, 3*m);
    G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, air, "World");
    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());
    G4VPhysicalVolume* physWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0);

    G4VSolid* sBlack = new G4Box("BlackTape_Solid", blackX/2.0, blackY/2.0, blackZ/2.0);
    G4LogicalVolume* lBlack = new G4LogicalVolume(sBlack, blackMat, "LogicBlack");
    lBlack->SetVisAttributes(blackVis);
    new G4PVPlacement(0, G4ThreeVector(), lBlack, "PhysBlack", logicWorld, false, 0, true);

    G4VSolid* sAl = new G4Box("AlTape_Solid", alX/2.0, alY/2.0, alZ/2.0);
    G4LogicalVolume* lAl = new G4LogicalVolume(sAl, alMat, "LogicAl");
    lAl->SetVisAttributes(alVis);
    new G4PVPlacement(0, G4ThreeVector(), lAl, "PhysAl", lBlack, false, 0, true);

    G4VSolid* sTyvek = new G4Box("Tyvek_Solid", tyX/2.0, tyY/2.0, tyZ/2.0);
    G4LogicalVolume* lTyvek = new G4LogicalVolume(sTyvek, tyvekMat, "LogicTyvek");
    lTyvek->SetVisAttributes(tyvekVis);
    new G4PVPlacement(0, G4ThreeVector(), lTyvek, "PhysTyvek", lAl, false, 0, true);
    new G4LogicalSkinSurface("TyvekSkin", lTyvek, fOpWrapping);

    // --- SUPERFICI OTTICHE PER ALLUMINIO E NASTRO NERO (Spiral Panel) ---
    G4int nEntries = 8;
    G4double energy[8] = {2.00*eV, 2.30*eV, 2.50*eV, 2.70*eV, 2.90*eV, 3.10*eV, 3.30*eV, 3.50*eV};

    G4OpticalSurface* opAl = new G4OpticalSurface("AlSurface");
    opAl->SetType(dielectric_metal);
    opAl->SetFinish(polished);
    opAl->SetModel(unified);
    G4MaterialPropertiesTable* mptAl = new G4MaterialPropertiesTable();
    G4double refAl[8] = {0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90}; 
    mptAl->AddProperty("REFLECTIVITY", energy, refAl, nEntries);
    opAl->SetMaterialPropertiesTable(mptAl);
    new G4LogicalSkinSurface("AlSkin", lAl, opAl);

    G4OpticalSurface* opBlack = new G4OpticalSurface("BlackSurface");
    opBlack->SetType(dielectric_metal);
    opBlack->SetFinish(ground);
    opBlack->SetModel(unified);
    G4MaterialPropertiesTable* mptBlack = new G4MaterialPropertiesTable();
    G4double refBlack[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
    mptBlack->AddProperty("REFLECTIVITY", energy, refBlack, nEntries);
    opBlack->SetMaterialPropertiesTable(mptBlack);
    new G4LogicalSkinSurface("BlackSkin", lBlack, opBlack);

    G4Box* solidScint = new G4Box("ScintBox", scintX/2.0, scintY/2.0, scintZ/2.0);
    G4LogicalVolume* logicScint = new G4LogicalVolume(solidScint, ej200, "ScintLogic");
    logicScint->SetVisAttributes(scintVis);
    new G4PVPlacement(0, G4ThreeVector(), logicScint, "ScintPhys", lTyvek, false, 0, true);

    // --- 4. PIAZZAMENTO SiPM E SUPERFICIE OTTICA PDE ---
    G4double sipmThick = 1.0 * mm; 
    G4double sipmSize  = 3.0 * mm; 
    G4LogicalVolume* lSiPM = new G4LogicalVolume(new G4Box("SiPM", sipmSize/2.0, sipmThick/2.0, sipmSize/2.0), sipmMat, "LogicSiPM_All");
    lSiPM->SetVisAttributes(sipmVis);

    G4OpticalSurface* opSiPM = new G4OpticalSurface("SiPMSurface");
    opSiPM->SetType(dielectric_metal);
    opSiPM->SetFinish(polished);
    opSiPM->SetModel(unified);
    G4MaterialPropertiesTable* mptSiPM = new G4MaterialPropertiesTable();
    G4double refSiPM[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}; 
    G4double effSiPM[8] = {0.15, 0.25, 0.30, 0.38, 0.40, 0.35, 0.25, 0.10}; // PDE Hamamatsu S13360-3050
    mptSiPM->AddProperty("REFLECTIVITY", energy, refSiPM, nEntries);
    mptSiPM->AddProperty("EFFICIENCY", energy, effSiPM, nEntries);
    opSiPM->SetMaterialPropertiesTable(mptSiPM);
    new G4LogicalSkinSurface("SiPMSkin", lSiPM, opSiPM);

    G4double sipmZ = 0.65 * mm; 

    std::vector<G4ThreeVector> sipmPositions(8);
    G4double bY = scintY + thTyvek + thAl + thBlack; 

    for(int i=0; i<8; i++) {
        int board = i / 4; 
        int indexInBoard = i % 4; 
        G4double boardX = (board == 0) ? -250.0*mm : 250.0*mm;
        G4double sipmX = boardX + (indexInBoard - 1.5) * 5.0 * mm; 
        G4double sipmY = -blackY/2.0 - sipmThick/2.0 - 0.1*mm;
        
        sipmPositions[i] = G4ThreeVector(sipmX, sipmY, sipmZ);
        new G4PVPlacement(0, sipmPositions[i], lSiPM, "pSiPM", logicWorld, false, i, true);
    }

    // --- 5. GENERAZIONE SETTORI ---
    G4double sectorSize = 250.0 * mm;
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 2; row++) {
            
            if (row == 0 || row == 1) {
                
                G4double cX = -375.0*mm + col * sectorSize;
                G4double cY =  125.0*mm - row * sectorSize; 
                int sectorID = col + row * 4;
                
                int sipmIndex = 0;
                if (col == 0 && row == 1) sipmIndex = 0; 
                if (col == 0 && row == 0) sipmIndex = 1; 
                if (col == 1 && row == 0) sipmIndex = 2; 
                if (col == 1 && row == 1) sipmIndex = 3; 

                if (col == 2 && row == 1) sipmIndex = 4; 
                if (col == 2 && row == 0) sipmIndex = 5; 
                if (col == 3 && row == 0) sipmIndex = 6; 
                if (col == 3 && row == 1) sipmIndex = 7; 

                BuildContinuousSpiralSector(logicScint, G4ThreeVector(cX, cY, 0), sectorID, sipmPositions[sipmIndex]);
            }
        }
    }
    return physWorld;
}

void DetectorConstruction::ConstructSDandField() {
    G4SDManager* sdM = G4SDManager::GetSDMpointer();
    SIPMSD* sd = new SIPMSD("SiPM_SD_All");
    sdM->AddNewDetector(sd);
    SetSensitiveDetector("LogicSiPM_All", sd);
}

void DetectorConstruction::DefineMaterials() {
    G4NistManager* nist = G4NistManager::Instance();

    G4Material* air       = nist->FindOrBuildMaterial("G4_AIR");
    G4Material* ej200     = nist->FindOrBuildMaterial("G4_PLASTIC_SC_VINYLTOLUENE"); 
    G4Material* sipmMat   = nist->FindOrBuildMaterial("G4_Si");
    G4Material* glueMat   = nist->FindOrBuildMaterial("G4_PLEXIGLASS"); 
    G4Material* tyvekMat  = nist->FindOrBuildMaterial("G4_POLYETHYLENE");
    G4Material* alMat     = nist->FindOrBuildMaterial("G4_Al");
    G4Material* blackMat  = nist->FindOrBuildMaterial("G4_POLYVINYL_CHLORIDE");

    G4Material* coreMat   = nist->FindOrBuildMaterial("G4_POLYSTYRENE");
    G4Material* cladMat = nist->FindOrBuildMaterial("G4_PLEXIGLASS"); 

    G4int nEntries = 8;
    G4double energy[8] = {2.00*eV, 2.30*eV, 2.50*eV, 2.70*eV, 2.90*eV, 3.10*eV, 3.30*eV, 3.50*eV};

    G4double rindexCore[8]   = {1.60, 1.60, 1.60, 1.60, 1.60, 1.60, 1.60, 1.60};
    G4double absCore[8]      = {3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m, 3.5*m}; 
    G4double wlsAbsCore[8]   = {3.5*m, 3.5*m, 3.5*m, 0.1*mm, 0.1*mm, 0.1*mm, 0.1*mm, 0.1*mm}; 
    G4double emissionBCF92[8]= {0.00, 0.10, 1.00, 0.20, 0.00, 0.00, 0.00, 0.00}; 

    G4MaterialPropertiesTable* mptCore = new G4MaterialPropertiesTable();
    mptCore->AddProperty("RINDEX", energy, rindexCore, nEntries);
    mptCore->AddProperty("ABSLENGTH", energy, absCore, nEntries);
    mptCore->AddProperty("WLSABSLENGTH", energy, wlsAbsCore, nEntries);
    mptCore->AddProperty("WLSCOMPONENT", energy, emissionBCF92, nEntries);
    mptCore->AddConstProperty("WLSTIMECONSTANT", 2.7*ns);
    mptCore->AddConstProperty("WLSMEANNUMBERPHOTONS", 0.86);
    coreMat->SetMaterialPropertiesTable(mptCore);

    G4double rindexClad[8] = {1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49};
    G4double absClad[8]    = {10.0*m, 10.0*m, 10.0*m, 10.0*m, 10.0*m, 10.0*m, 10.0*m, 10.0*m};
    G4MaterialPropertiesTable* mptClad = new G4MaterialPropertiesTable();
    mptClad->AddProperty("RINDEX", energy, rindexClad, nEntries);
    mptClad->AddProperty("ABSLENGTH", energy, absClad, nEntries);
    cladMat->SetMaterialPropertiesTable(mptClad);

    G4double rindexScint[8]  = {1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58, 1.58};
    G4double absScint[8]     = {3.8*m, 3.8*m, 3.8*m, 3.8*m, 3.8*m, 3.8*m, 3.8*m, 3.8*m};
    G4double emissionEJ200[8]= {0.00, 0.05, 0.20, 0.60, 1.00, 0.50, 0.10, 0.00}; 

    G4MaterialPropertiesTable* mptScint = new G4MaterialPropertiesTable();
    mptScint->AddProperty("RINDEX", energy, rindexScint, nEntries);
    mptScint->AddProperty("ABSLENGTH", energy, absScint, nEntries);
    mptScint->AddProperty("SCINTILLATIONCOMPONENT1", energy, emissionEJ200, nEntries);
    mptScint->AddConstProperty("SCINTILLATIONYIELD", 10000./MeV);
    mptScint->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 2.1*ns);
    mptScint->AddConstProperty("SCINTILLATIONRISETIME1", 0.9*ns); 
    mptScint->AddConstProperty("SCINTILLATIONYIELD1", 1.0);
    mptScint->AddConstProperty("RESOLUTIONSCALE", 1.0); 
    ej200->SetMaterialPropertiesTable(mptScint);

    G4double rindexGlue[8] = {1.57, 1.57, 1.57, 1.57, 1.57, 1.57, 1.57, 1.57};
    G4double absGlue[8]    = {5.0*m, 5.0*m, 5.0*m, 5.0*m, 5.0*m, 5.0*m, 5.0*m, 5.0*m};
    G4MaterialPropertiesTable* mptGlue = new G4MaterialPropertiesTable();
    mptGlue->AddProperty("RINDEX", energy, rindexGlue, nEntries);
    mptGlue->AddProperty("ABSLENGTH", energy, absGlue, nEntries);
    glueMat->SetMaterialPropertiesTable(mptGlue);

    G4double rindexAir[8] = {1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00};
    G4MaterialPropertiesTable* mptAir = new G4MaterialPropertiesTable();
    mptAir->AddProperty("RINDEX", energy, rindexAir, nEntries);
    air->SetMaterialPropertiesTable(mptAir);

    G4double rindexBlack[8] = {1.50, 1.50, 1.50, 1.50, 1.50, 1.50, 1.50, 1.50};
    G4double absBlack[8]    = {0.001*mm, 0.001*mm, 0.001*mm, 0.001*mm, 0.001*mm, 0.001*mm, 0.001*mm, 0.001*mm};
    G4MaterialPropertiesTable* mptBlack = new G4MaterialPropertiesTable();
    mptBlack->AddProperty("RINDEX", energy, rindexBlack, nEntries);
    mptBlack->AddProperty("ABSLENGTH", energy, absBlack, nEntries);
    blackMat->SetMaterialPropertiesTable(mptBlack);

    fOpWrapping = new G4OpticalSurface("WrappingSurface");
    fOpWrapping->SetType(dielectric_metal); 
    fOpWrapping->SetModel(unified);
    
    G4MaterialPropertiesTable* mptWrapSurf = new G4MaterialPropertiesTable();

    if (fWrappingType == "Tyvek") {
        fOpWrapping->SetFinish(ground); 
        fOpWrapping->SetSigmaAlpha(0.2);
        // Tyvek 8740D a DOPPIO strato raggiunge il 98% di riflettività
        G4double reflectivityTyvek[8] = {0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98};
        mptWrapSurf->AddProperty("REFLECTIVITY", energy, reflectivityTyvek, nEntries);
    } 
    else if (fWrappingType == "Mylar") {
        fOpWrapping->SetFinish(polished);
        G4double reflectivityMylar[8] = {0.97, 0.97, 0.97, 0.97, 0.97, 0.97, 0.97, 0.97}; 
        mptWrapSurf->AddProperty("REFLECTIVITY", energy, reflectivityMylar, nEntries);
    }
    fOpWrapping->SetMaterialPropertiesTable(mptWrapSurf);
}

void DetectorConstruction::BuildContinuousSpiralSector(G4LogicalVolume* motherVolume, G4ThreeVector center, G4int sectorID, G4ThreeVector sipmPos) {
    
    int col = sectorID % 4;
    G4double scaleX = (col % 2 == 0) ? 1.0 : -1.0; 

    G4Material* coreMat = G4Material::GetMaterial("G4_POLYSTYRENE");
    G4Material* cladMat = G4Material::GetMaterial("G4_PLEXIGLASS"); 
    G4Material* glueMat = G4Material::GetMaterial("G4_PLEXIGLASS");

    G4double glueR = 0.600*mm, cladR = 0.495*mm, coreR = 0.480*mm; 
    G4double rBend = 10.0 * mm;  
    G4double pitch = 22.0 * mm; 

    G4VisAttributes* coreVis = new G4VisAttributes(G4Colour(0.0, 1.0, 0.0, 1.0)); coreVis->SetForceSolid(true);
    G4VisAttributes* cladVis = new G4VisAttributes(G4Colour(0.0, 1.0, 1.0, 0.1)); cladVis->SetVisibility(false);
    G4VisAttributes* glueVis = new G4VisAttributes(G4Colour(0.8, 0.8, 0.0, 0.1)); glueVis->SetVisibility(false);

    double baseSipmX = std::abs(sipmPos.x() - center.x());
    double trunk1X = baseSipmX - 0.65 * mm; 
    double trunk2X = baseSipmX + 0.65 * mm;
    double localSipmY = sipmPos.y() - center.y();

    std::vector<G4ThreeVector> pts1;
    double z1 = 0.0 * mm; 

    pts1.push_back({trunk1X, localSipmY, z1});
    pts1.push_back({trunk1X, 110.0*mm, z1});
    pts1.push_back({-110.0*mm, 110.0*mm, z1});
    pts1.push_back({-110.0*mm, -75.0*mm, z1});
    
    pts1.push_back({trunk1X - 22.0*mm, -75.0*mm, z1});
    pts1.push_back({trunk1X - 22.0*mm, 88.0*mm, z1});
    pts1.push_back({-88.0*mm, 88.0*mm, z1});
    pts1.push_back({-88.0*mm, -53.0*mm, z1});
    
    pts1.push_back({trunk1X - 44.0*mm, -53.0*mm, z1});
    pts1.push_back({trunk1X - 44.0*mm, 66.0*mm, z1});
    pts1.push_back({-66.0*mm, 66.0*mm, z1});
    pts1.push_back({-66.0*mm, -31.0*mm, z1});
    
    pts1.push_back({trunk1X - 66.0*mm, -31.0*mm, z1});
    pts1.push_back({trunk1X - 66.0*mm, 44.0*mm, z1});
    pts1.push_back({-44.0*mm, 44.0*mm, z1});
    
    pts1.push_back({-44.0*mm, -15.0*mm, z1}); 
    pts1.push_back({28.0*mm, -15.0*mm, z1});
    pts1.push_back({28.0*mm, 30.0*mm, z1});
    pts1.push_back({-25.0*mm, 30.0*mm, z1});
    pts1.push_back({-25.0*mm, -5.0*mm, z1});
    pts1.push_back({10.0*mm, -5.0*mm, z1});

    pts1.push_back({10.0*mm, 20.0*mm, z1});
    pts1.push_back({-10.0*mm, 20.0*mm, z1});
    pts1.push_back({-10.0*mm, 0.0*mm, z1});
    pts1.push_back({0.0*mm, 0.0*mm, z1}); 

    double z2 = 1.3 * mm; 
    std::vector<G4ThreeVector> pts2;
    
    pts2.push_back({0.0*mm, 0.0*mm, z2});      
    pts2.push_back({trunk2X, 0.0*mm, z2});       
    pts2.push_back({trunk2X, localSipmY, z2});   

    for(auto& p : pts1) p.setX(p.x() * scaleX);
    for(auto& p : pts2) p.setX(p.x() * scaleX);

    auto placeVerticalBridge = [&](G4ThreeVector pos, G4double zA, G4double zB, G4String n) {
        G4double zMid = (zA + zB) / 2.0;
        G4double zLen = std::abs(zA - zB);
        G4LogicalVolume* lGV = new G4LogicalVolume(new G4Tubs(n+"_G", 0, glueR, zLen/2., 0, 360*deg), glueMat, n+"_LG");
        G4LogicalVolume* lCV = new G4LogicalVolume(new G4Tubs(n+"_C", 0, cladR, zLen/2., 0, 360*deg), cladMat, n+"_LC");
        G4LogicalVolume* lCoV= new G4LogicalVolume(new G4Tubs(n+"_Co",0, coreR, zLen/2., 0, 360*deg), coreMat, n+"_LCo");
        lGV->SetVisAttributes(glueVis); lCV->SetVisAttributes(cladVis); lCoV->SetVisAttributes(coreVis);
        new G4PVPlacement(0, G4ThreeVector(), lCoV, n+"_PCorV", lCV, false, 0, false);
        new G4PVPlacement(0, G4ThreeVector(), lCV, n+"_PCladV", lGV, false, 0, false);
        new G4PVPlacement(0, center + G4ThreeVector(pos.x(), pos.y(), zMid), lGV, n, motherVolume, false, sectorID, false);
    };

    placeVerticalBridge(G4ThreeVector(0.0*mm * scaleX, 0.0*mm, 0), z1, z2, "BridgeCenter");

    auto buildPath = [&](const std::vector<G4ThreeVector>& pts, G4String pName) {
        for (size_t i = 0; i < pts.size() - 1; i++) {
            G4ThreeVector p0 = pts[i];
            G4ThreeVector p1 = pts[i+1];
            G4ThreeVector dir = (p1 - p0).unit();
            G4double len = (p1 - p0).mag();

            G4double trimS = (i == 0) ? 0.0 : rBend;
            G4double trimE = (i == pts.size() - 2) ? 0.0 : rBend;
            G4double straightL = len - trimS - trimE;

            if (straightL > 0) {
                G4ThreeVector mid = p0 + dir * (trimS + straightL/2.0);
                G4RotationMatrix* rot = new G4RotationMatrix();
                if (std::abs(dir.x()) > 0.5) rot->rotateY(90*deg);
                else rot->rotateX(90*deg);

                G4LogicalVolume* lGlue = new G4LogicalVolume(new G4Tubs(pName+"Str_G", 0, glueR, straightL/2., 0, 360*deg), glueMat, "LG");
                G4LogicalVolume* lClad = new G4LogicalVolume(new G4Tubs(pName+"Str_C", 0, cladR, straightL/2., 0, 360*deg), cladMat, "LC");
                G4LogicalVolume* lCore = new G4LogicalVolume(new G4Tubs(pName+"Str_Co",0, coreR, straightL/2., 0, 360*deg), coreMat, "LCo");
                lGlue->SetVisAttributes(glueVis); lClad->SetVisAttributes(cladVis); lCore->SetVisAttributes(coreVis);
                
                new G4PVPlacement(0, G4ThreeVector(), lCore, "PCor", lClad, false, 0, false);
                new G4PVPlacement(0, G4ThreeVector(), lClad, "PClad", lGlue, false, 0, false);
                new G4PVPlacement(rot, center + mid, lGlue, pName+"Leg", motherVolume, false, sectorID, false);
            }

            if (i < pts.size() - 2) {
                G4ThreeVector p2 = pts[i+2];
                G4ThreeVector dir2 = (p2 - p1).unit();
                G4ThreeVector cArc = p1 - dir * rBend + dir2 * rBend;
                
                G4ThreeVector vStart = (p1 - dir * rBend) - cArc;
                G4ThreeVector vEnd = (p1 + dir2 * rBend) - cArc;
                
                G4double a1 = std::atan2(vStart.y(), vStart.x());
                G4double a2 = std::atan2(vEnd.y(), vEnd.x());
                if (a1 < 0) a1 += 360*deg;
                if (a2 < 0) a2 += 360*deg;
                
                G4double sAng = std::min(a1, a2);
                if (std::abs(a1 - a2) > 180*deg) sAng = std::max(a1, a2); 

                G4LogicalVolume* lGlue = new G4LogicalVolume(new G4Torus(pName+"Cor_G", 0, glueR, rBend, sAng, 90*deg), glueMat, "LG");
                G4LogicalVolume* lClad = new G4LogicalVolume(new G4Torus(pName+"Cor_C", 0, cladR, rBend, sAng, 90*deg), cladMat, "LC");
                G4LogicalVolume* lCore = new G4LogicalVolume(new G4Torus(pName+"Cor_Co",0, coreR, rBend, sAng, 90*deg), coreMat, "LCo");
                lGlue->SetVisAttributes(glueVis); lClad->SetVisAttributes(cladVis); lCore->SetVisAttributes(coreVis);
                
                new G4PVPlacement(0, G4ThreeVector(), lCore, "PCor", lClad, false, 0, false);
                new G4PVPlacement(0, G4ThreeVector(), lClad, "PClad", lGlue, false, 0, false);
                new G4PVPlacement(0, center + cArc, lGlue, pName+"Cor", motherVolume, false, sectorID, false);
            }
        }
    };

    buildPath(pts1, "P1_");
    buildPath(pts2, "P2_");
}
