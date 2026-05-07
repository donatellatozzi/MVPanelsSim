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
    else {
        G4cerr << "Errore: Tipo di geometria sconosciuto!" << G4endl;
        return nullptr;
    }
}

G4VPhysicalVolume* DetectorConstruction::ConstructStandardPanel() {
    G4Material* air       = G4Material::GetMaterial("G4_AIR");
    G4Material* ej200     = G4Material::GetMaterial("G4_PLASTIC_SC_VINYLTOLUENE");
    G4Material* tyvekMat  = G4Material::GetMaterial("G4_POLYETHYLENE");
    G4Material* alMat     = G4Material::GetMaterial("G4_Al");
    G4Material* blackMat  = G4Material::GetMaterial("G4_POLYVINYL_CHLORIDE"); 
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
    G4VisAttributes* alVis    = new G4VisAttributes(G4Colour(0.7, 0.7, 0.7, 0.3));
    G4VisAttributes* blackVis = new G4VisAttributes(G4Colour(0.1, 0.1, 0.1, 0.3));
  
    G4double scintX = 625.0*mm, scintY = 275.0*mm, scintZ = 25.0*mm;
    G4double thTyvek = 0.4*mm, thAl = 0.1*mm, thBlack = 0.2*mm;

    G4double blackX = scintX + thTyvek + thAl + thBlack;
    G4double blackY = scintY + thTyvek + thAl + thBlack;
    G4double blackZ = scintZ + thTyvek + thAl + thBlack;
    G4VSolid* sBlack = new G4Box("BlackTape_Solid", blackX, blackY, blackZ);

    G4double alX = scintX + thTyvek + thAl;
    G4double alY = scintY + thTyvek + thAl;
    G4double alZ = scintZ + thTyvek + thAl;
    G4VSolid* sAl = new G4Box("AlTape_Solid", alX, alY, alZ);

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
            sAl    = new G4SubtractionSolid("Al_Hole", sAl, sHole, rotHole, holePos);
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

    G4LogicalVolume* lAl = new G4LogicalVolume(sAl, alMat, "LogicAl");
    lAl->SetVisAttributes(alVis);
    new G4PVPlacement(0, G4ThreeVector(), lAl, "PhysAl", lBlack, false, 0, true);

    G4LogicalVolume* lTyvek = new G4LogicalVolume(sTyvek, tyvekMat, "LogicTyvek");
    lTyvek->SetVisAttributes(tyvekVis);
    new G4PVPlacement(0, G4ThreeVector(), lTyvek, "PhysTyvek", lAl, false, 0, true);
    new G4LogicalSkinSurface("TyvekSkin", lTyvek, fOpWrapping);

    // --- SUPERFICI OTTICHE PER ALLUMINIO E NASTRO NERO (Standard Panel) ---
    G4int nEntries = 8;
    G4double energy[8] = {2.00*eV, 2.30*eV, 2.50*eV, 2.70*eV, 2.90*eV, 3.10*eV, 3.30*eV, 3.50*eV};

    G4OpticalSurface* opAl = new G4OpticalSurface("AlSurface");
    opAl->SetType(dielectric_metal);
    opAl->SetFinish(polished);
    opAl->SetModel(unified);
    G4MaterialPropertiesTable* mptAl = new G4MaterialPropertiesTable();
    G4double refAl[8] = {0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90, 0.90}; // 90% Specular per Alluminio Foil
    mptAl->AddProperty("REFLECTIVITY", energy, refAl, nEntries);
    opAl->SetMaterialPropertiesTable(mptAl);
    new G4LogicalSkinSurface("AlSkin", lAl, opAl);

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

    G4double dxBundle         = 200.0*mm, straightFinal    = 30.0*mm;  
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
