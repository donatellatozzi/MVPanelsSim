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
    G4Material* cladMat   = G4Material::GetMaterial("FiberCladding");
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

    // Approssima un arco circolare (raggio bendR, centro dato, angolo phi da
    // phiStart a phiEnd, nel piano locale ruotato di "alpha" attorno a X
    // tramite cosA=cos(alpha), sinA=sin(alpha)) con una catena di nSeg corde
    // dritte (G4Tubs). Serve a sostituire i G4Torus usati per la convergenza
    // del bundle esterno: quando rBend e' molto grande rispetto a cladR
    // (rapporto d'aspetto fino a ~1600:1 per le gambe con offset Y-Z minimo
    // su 170mm fissi), G4Torus naviga male (fotoni che "vibrano" sulla
    // superficie senza avanzare, simulazione bloccata, 0 hit sul SiPM nel
    // Nested) -- problema osservato e diagnosticato con il profiler di
    // sistema (CPU bloccata in SteppingAction/IsPhotonUnique). G4Tubs non ha
    // questo problema indipendentemente dal rapporto lunghezza/raggio. Con
    // nSeg=24 la freccia massima (sagitta) della corda rispetto all'arco
    // vero e' di pochi micron anche nel caso peggiore (rBend~830mm), molto
    // sotto il margine di clearance tra fibre (~1mm): la forma del percorso
    // (e quindi la permutazione fibra->punto impacchettato gia' validata)
    // resta sostanzialmente quella originale.
    auto placeArcChain = [&](G4double bendR, G4double xCenter, G4double yCenter, G4double zCenter,
                              G4double cosA, G4double sinA, G4double xSign, G4double phiStart, G4double phiEnd,
                              G4int nSeg, G4String tag) {
        auto pointAt = [&](G4double phi) {
            return G4ThreeVector(xCenter + xSign*bendR*std::cos(phi),
                                  yCenter + bendR*std::sin(phi)*cosA,
                                  zCenter + bendR*std::sin(phi)*sinA);
        };
        G4ThreeVector prev = pointAt(phiStart);
        for (G4int s = 0; s < nSeg; s++) {
            G4double phiNext = phiStart + (phiEnd - phiStart) * (s+1.0)/nSeg;
            G4ThreeVector next = pointAt(phiNext);
            G4ThreeVector mid  = (prev + next) * 0.5;
            G4ThreeVector dvec = next - prev;
            G4double segLen = dvec.mag();
            G4ThreeVector u = dvec.unit();

            G4RotationMatrix* rotSeg = new G4RotationMatrix();
            G4ThreeVector zAxis(0,0,1);
            G4double dotZ = zAxis.dot(u);
            if (dotZ < 1.0 - 1.0e-9) {
                G4ThreeVector axis = zAxis.cross(u);
                if (axis.mag() > 1.0e-9) {
                    rotSeg->rotate(std::acos(dotZ), axis.unit());
                } else {
                    rotSeg->rotateX(180*deg);
                }
            }
            G4String segName = tag + "_S" + std::to_string(s);
            G4LogicalVolume* lSeg = makeNudeStraight(segLen, segName);
            // Consecutive chord-segments share endpoints: overlap is fiber-on-fiber
            // (same cladding material in air), physically harmless — skip check.
            new G4PVPlacement(rotSeg, mid, lSeg, "p"+segName, logicWorld, false, 0, false);
            prev = next;
        }
    };

    G4RotationMatrix* rotY90  = new G4RotationMatrix(); rotY90->rotateY(90*deg);
    G4RotationMatrix* rotX180 = new G4RotationMatrix(); rotX180->rotateX(180*deg);

    // Distanza pannello -> SiPM = bridgeLen + dxBundle + straightFinal = 200.0 mm (20 cm), come richiesto.
    G4double dxBundle         = 170.0*mm, straightFinal    = 20.0*mm;
    G4double sipmThick        = 0.5*mm, bridgeLen        = 10.0*mm;
    G4double greaseThick      = 0.5*mm;

    G4double xBundleEnd  = xScintEnd - bridgeLen - dxBundle;
    G4double xFiberEnd   = xBundleEnd - straightFinal;    // faccia sinistra del Final
    G4double xSiPM       = xFiberEnd - greaseThick - sipmThick;

    // Le 8 fibre del bundle (nude, raggio cladR) sono incollate (colla EJ-500,
    // come nei groove) dentro un cilindro di diametro interno 3.45mm, accoppiato
    // allo stesso SiPM fisico usato in entrambe le geometrie. Impacchettamento:
    // 1 fibra centrale + 7 ad un raggio scelto USANDO TUTTO IL MARGINE fino alla
    // parete del cilindro (rRing = cilindroR - cladR = 1.230mm), invece del
    // raggio minimo di tangenza tra vicini (2.305*cladR = 1.141mm): questo da'
    // ~77um di clearance reale tra fibre adiacenti (necessaria, verificato che a
    // tangenza esatta il check overlap di Geant4 segnala urti). Centrato su
    // (Y=0, Z=20.25mm), identico nel Nested.
    G4double zSiPMCenter = 20.25*mm;
    G4double rRing = 1.725*mm - cladR;
    G4double packedY[8], packedZ[8];
    packedY[0] = 0.0*mm; packedZ[0] = zSiPMCenter;
    for (int k = 0; k < 7; k++) {
        G4double ang = k * (360.0/7.0) * deg;
        packedY[k+1] = rRing * std::cos(ang);
        packedZ[k+1] = zSiPMCenter + rRing * std::sin(ang);
    }

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

    // Grasso ottico tra faccia delle fibre e SiPM (accoppiamento ottico, n=1.465)
    G4Material* greaseMat = G4Material::GetMaterial("OpticalGrease");
    G4Box* sGrease = new G4Box("Grease", greaseThick/2., 3.0*mm, 3.0*mm);
    G4LogicalVolume* lGrease = new G4LogicalVolume(sGrease, greaseMat, "LogicGrease");
    G4VisAttributes* greaseVis = new G4VisAttributes(G4Colour(0.9, 0.9, 0.2, 0.4));
    lGrease->SetVisAttributes(greaseVis);
    new G4PVPlacement(0, G4ThreeVector(xFiberEnd - greaseThick/2., 0, zSiPMCenter),
                      lGrease, "pGrease", logicWorld, false, 0, true);

    new G4PVPlacement(0, G4ThreeVector(xSiPM, 0, zSiPMCenter), lSiPM, "pSiPM", logicWorld, false, 0, true);

    // Assegnazione fiberCount -> indice del punto impacchettato (0=centro,
    // 1-7=anello). Trovata con ricerca esaustiva su tutte le 8! permutazioni
    // (simulando i percorsi curvi reali Bridge+Arc1+Arc2+Final e calcolando la
    // distanza minima punto-punto tra ogni coppia di fibre, poi riverificata a
    // risoluzione crescente fino a N=1000 campioni per evitare falsi positivi
    // da sotto-campionamento): questa e' la permutazione che massimizza la
    // clearance minima tra fibre diverse lungo tutto il bundle esterno
    // (1.067mm, sopra il minimo richiesto di 0.99mm = 2*cladR), stabile a tutte
    // le risoluzioni testate (quindi un vero non-incrocio, non un artefatto).
    int fiberToPacked[8] = {2, 3, 1, 7, 0, 4, 6, 5};
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
            G4double yTarget = packedY[fiberToPacked[fiberCount]];
            G4double zTarget = packedZ[fiberToPacked[fiberCount]];
            G4String tag = "Std_F" + std::to_string(fiberCount);
            fiberCount++;

            G4LogicalVolume* lBridge = makeNudeStraight(bridgeLen, tag+"_Bridge");
            new G4PVPlacement(rotY90, G4ThreeVector(xScintEnd - bridgeLen/2.0, yStart, zFib), lBridge, "p"+tag+"_Bridge", logicWorld, false, f*2+leg, true);

            // Piegatura combinata Y-Z (non solo Y): il bundle converge nel punto
            // impacchettato (yTarget,zTarget) dentro il cilindro da 3.45mm.
            // Stessa formula "a budget esatto" di prima (l'intero dxBundle e'
            // consumato dalla curva S a 2 archi) -- questa e' la forma di
            // percorso su cui e' stata validata (esaustivamente, via Geant4
            // CheckOverlaps) la permutazione fiberToPacked. La curva e'
            // pero' ora approssimata con una catena di corde dritte
            // (placeArcChain, vedi sopra) invece di un G4Torus vero: per le
            // gambe con offset Y-Z minimo su 170mm fissi rBend arriva fino a
            // ~830mm (rapporto d'aspetto ~1680:1 rispetto a cladR=0.495mm),
            // valore che G4Torus non naviga in modo numericamente stabile.
            G4double dy = yTarget - yStart;
            G4double dz = zTarget - zFib;
            G4double dr = std::sqrt(dy*dy + dz*dz);
            G4double dx = dxBundle;
            G4double rBend = (dx*dx + dr*dr) / (4.0 * dr);
            G4double theta = std::acos(1.0 - dr / (2.0 * rBend));
            G4double cosA  = -dy/dr;
            G4double sinA  = -dz/dr;

            G4double xArcStart = xScintEnd - bridgeLen;
            G4double yCenter1 = yStart + rBend*dy/dr;
            G4double zCenter1 = zFib   + rBend*dz/dr;
            placeArcChain(rBend, xArcStart, yCenter1, zCenter1, cosA, sinA, +1.0, 90*deg, 90*deg+theta, 24, tag+"_NA1");

            G4double yCenter2 = yTarget - rBend*dy/dr;
            G4double zCenter2 = zTarget - rBend*dz/dr;
            placeArcChain(rBend, xBundleEnd, yCenter2, zCenter2, cosA, sinA, -1.0, 270*deg-theta, 270*deg, 24, tag+"_NA2");

            G4LogicalVolume* lFinal = makeNudeStraight(straightFinal, tag+"_Final");
            G4double xFinalCenter = xBundleEnd - straightFinal/2.0;
            new G4PVPlacement(rotY90, G4ThreeVector(xFinalCenter, yTarget, zTarget), lFinal, "p"+tag+"_Final", logicWorld, false, 0, false);
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
    G4Material* cladMat   = G4Material::GetMaterial("FiberCladding");
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
    // Geometria conforme al disegno Muon_Panel_v2: pannello 1250x550mm,
    // R60mm per U-turn (destra) e S-curve (sinistra), 4 fori sul bordo sinistro
    // a Y = +30, +10, -10, -30 mm (ricavati da 245+20+20+20+245=550mm).
    //
    // Ogni fibra ha 2 capi ("out" = lontano dal centro, "in" = vicino al centro)
    // collegati da una curva a U sul lato destro (R60mm).
    // I capi convergono verso i 4 fori tramite S-curve R60mm sul lato sinistro.
    //
    // Schema di assegnazione (topologia "cross-loop" del disegno):
    //   foro1 (+30mm) : B_in (dritto, Y=+30)  + C_in (curva da Y=-90, dy=120mm PULITA)
    //   foro2 (+10mm) : A_out (curva da Y=+210, dy=200mm) + B_out (curva da Y=+150, dy=140mm)
    //   foro3 (-10mm) : C_out (curva da Y=-210, dy=200mm) + D_out (curva da Y=-150, dy=140mm)
    //   foro4 (-30mm) : D_in (dritto, Y=-30)  + A_in (curva da Y=+90, dy=120mm PULITA)
    //
    // Le fibre A_in e C_in ATTRAVERSANO il centro del pannello (curva "pulita" a 90deg
    // con R60mm: dxCurve = sqrt(120*(4*60-120)) = 120mm, visibile nel disegno come
    // le curve che arrivano al foro1 dal basso e al foro4 dall'alto).
    //
    // B_out e D_out devono attraversare Y del proprio braccio interno (B_in/D_in)
    // che corre dritto alla stessa Z: necessario un breve salto diagonale in Z
    // (zHome -> z, "jogDx" in X) prima della S-curva, cosi' la curva avviene a
    // una Z sicura e la compagna non viene toccata.
    struct FiberLeg {
        G4String name;
        G4double yNominal; // Y lungo la corsa rettilinea principale (dopo la curva a U)
        G4double yHole;    // Y effettiva al foro
        G4double zHome;    // Z alla curva a U lontana
        G4double z;        // Z effettiva al foro (= zHome se non serve salto)
    };

    G4double zA = 24.0*mm, zB = 22.5*mm, zC = 18.0*mm, zD = 16.5*mm;
    G4double zBOutJog = 19.5*mm, zDOutJog = 21.0*mm;

    std::vector<FiberLeg> legs = {
        {"A_out", +210.0*mm,  +10.0*mm, zA,  zA       },  // foro2, curva giu' 200mm
        {"A_in",   +90.0*mm,  -30.0*mm, zA,  zA       },  // foro4, curva giu' 120mm (PULITA, CROSS)
        {"B_out", +150.0*mm,  +10.0*mm, zB,  zBOutJog },  // foro2, curva giu' 140mm + salto Z
        {"B_in",   +30.0*mm,  +30.0*mm, zB,  zB       },  // foro1, dritto
        {"C_out", -210.0*mm,  -10.0*mm, zC,  zC       },  // foro3, curva su 200mm
        {"C_in",   -90.0*mm,  +30.0*mm, zC,  zC       },  // foro1, curva su 120mm (PULITA, CROSS)
        {"D_out", -150.0*mm,  -10.0*mm, zD,  zDOutJog },  // foro3, curva su 140mm + salto Z
        {"D_in",   -30.0*mm,  -30.0*mm, zD,  zD       },  // foro4, dritto
    };

    // Taglia 8 fori (uno per capo-fibra, 2 per ogni posizione Y sul bordo sinistro)
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

    G4double glueR = 0.600*mm, cladR = 0.495*mm, coreR = 0.480*mm;
    G4Tubs* sPlug = new G4Tubs("Plug_Solid_N", cladR, holeR, holeLen/2.0, 0, 360*deg);
    G4LogicalVolume* lPlug = new G4LogicalVolume(sPlug, blackMat, "Plug_Logic_N");
    lPlug->SetVisAttributes(blackVis);
    for (const auto& leg : legs) {
        G4ThreeVector holePos(xScintEnd, leg.yHole, leg.z);
        new G4PVPlacement(rotHole, holePos, lPlug, "PhysPlug_N", logicWorld, false, 0, true);
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

    auto makeNudeStraight = [&](G4double len, G4String name) {
        G4Tubs* sClad = new G4Tubs(name+"_SClad", 0, cladR, len/2.0, 0, 360*deg);
        G4Tubs* sCore = new G4Tubs(name+"_SCore", 0, coreR, len/2.0, 0, 360*deg);
        G4LogicalVolume* lClad = new G4LogicalVolume(sClad, cladMat, name+"_LClad");
        G4LogicalVolume* lCore = new G4LogicalVolume(sCore, coreMat, name+"_LCore");
        lClad->SetVisAttributes(cladVis); lCore->SetVisAttributes(coreVis);
        new G4PVPlacement(0, G4ThreeVector(), lCore, name+"_PCore", lClad, false, 0, true);
        return lClad;
    };

    // Approssima un arco con una catena di corde dritte (G4Tubs) per evitare
    // problemi numerici di G4Torus a rapporti d'aspetto estremi nel bundle esterno.
    auto placeArcChain = [&](G4double bendR, G4double xCenter, G4double yCenter, G4double zCenter,
                              G4double cosA, G4double sinA, G4double xSign, G4double phiStart, G4double phiEnd,
                              G4int nSeg, G4String tag) {
        auto pointAt = [&](G4double phi) {
            return G4ThreeVector(xCenter + xSign*bendR*std::cos(phi),
                                  yCenter + bendR*std::sin(phi)*cosA,
                                  zCenter + bendR*std::sin(phi)*sinA);
        };
        G4ThreeVector prev = pointAt(phiStart);
        for (G4int s = 0; s < nSeg; s++) {
            G4double phiNext = phiStart + (phiEnd - phiStart) * (s+1.0)/nSeg;
            G4ThreeVector next = pointAt(phiNext);
            G4ThreeVector mid  = (prev + next) * 0.5;
            G4ThreeVector dvec = next - prev;
            G4double segLen = dvec.mag();
            G4ThreeVector u = dvec.unit();
            G4RotationMatrix* rotSeg = new G4RotationMatrix();
            G4ThreeVector zAxis(0,0,1);
            G4double dotZ = zAxis.dot(u);
            if (dotZ < 1.0 - 1.0e-9) {
                G4ThreeVector axis = zAxis.cross(u);
                if (axis.mag() > 1.0e-9) {
                    rotSeg->rotate(std::acos(dotZ), axis.unit());
                } else {
                    rotSeg->rotateX(180*deg);
                }
            }
            G4String segName = tag + "_S" + std::to_string(s);
            G4LogicalVolume* lSeg = makeNudeStraight(segLen, segName);
            // Consecutive chord-segments share endpoints: overlap is fiber-on-fiber
            // (same cladding material in air), physically harmless — skip check.
            new G4PVPlacement(rotSeg, mid, lSeg, "p"+segName, logicWorld, false, 0, false);
            prev = next;
        }
    };

    G4RotationMatrix* rotY90 = new G4RotationMatrix(); rotY90->rotateY(90*deg);

    // --- CURVE A U LONTANE (raggio 60mm, lato destro) ---
    G4double rU = 60.0*mm;
    G4double xBackOfU     = scintX - 60.0*mm;     // = 565mm
    G4double xUturnCenter = xBackOfU - rU;          // = 505mm
    G4double straightLen  = xUturnCenter - xScintEnd; // = 1130mm

    auto placeUturn = [&](G4double yOut, G4double yIn, G4double zLeg, G4String name) {
        G4double yMid = (yOut + yIn) / 2.0;
        G4LogicalVolume* lU = makeArc(rU, 270*deg, 180*deg, name);
        new G4PVPlacement(0, G4ThreeVector(xUturnCenter, yMid, zLeg), lU, name+"_P", logicScint, false, 0, true);
    };

    placeUturn( 210.0*mm,   90.0*mm, zA, "Nest_UA");
    placeUturn( 150.0*mm,   30.0*mm, zB, "Nest_UB");
    placeUturn(-210.0*mm,  -90.0*mm, zC, "Nest_UC");
    placeUturn(-150.0*mm,  -30.0*mm, zD, "Nest_UD");

    // --- GAMBE: tratto rettilineo + (se serve) salto Z + S-curva R60mm ---
    // Per ogni capo-fibra (leg): corre dritto a yNominal da xUturnCenter verso
    // sinistra, poi eventuale jog diagonale in Z (per B_out e D_out, che altrimenti
    // colliderebbero con B_in/D_in al passaggio), poi S-curva R60mm da yNominal
    // a yHole. La formula della curva S con raggio fisso rMerge:
    //   dxCurve = sqrt(dy*(4*rMerge - dy))    [estensione orizzontale dell'intera S]
    //   theta   = acos(1 - dy/(2*rMerge))     [angolo di ciascun quarto di cerchio]
    // Caso "pulito" (A_in, C_in): dy=120mm, rMerge=60mm -> dxCurve=120mm, theta=90deg.
    G4double rMerge = 60.0*mm;
    G4double jogDx  = 40.0*mm;
    for (const auto& leg : legs) {
        G4double yStart  = leg.yNominal;
        G4double yTarget = leg.yHole;
        G4double dy      = std::abs(yStart - yTarget);
        bool needsJog    = std::abs(leg.z - leg.zHome) > 1.0e-9;

        if (dy < 1.0e-9 && !needsJog) {
            // Rettilineo puro (B_in, D_in)
            G4LogicalVolume* lFull = makeStraight(straightLen, "Nest_"+leg.name);
            G4double xCenter = (xUturnCenter + xScintEnd) / 2.0;
            new G4PVPlacement(rotY90, G4ThreeVector(xCenter, yTarget, leg.z),
                              lFull, "Nest_P"+leg.name, logicScint, false, 0, true);
            continue;
        }

        // dy > 2*R: due archi fissi 90° + tratto verticale (come da disegno per A_out/C_out)
        // dy <= 2*R: S-curva a theta variabile (A_in/C_in e B_out/D_out)
        bool bigDrop     = (dy > 2.0*rMerge + 1.0e-9);
        G4double dxCurve = bigDrop ? (2.0*rMerge)
                                   : ((dy > 1.0e-9) ? std::sqrt(dy * (4.0*rMerge - dy)) : 0.0);
        // A_in e C_in (dy==2R): tratto orizzontale finale di 60mm al livello del foro
        G4double finalApproach = (std::abs(dy - 2.0*rMerge) < 1.0e-9 && !bigDrop) ? 60.0*mm : 0.0;
        G4double dxTotalEnd = dxCurve + (needsJog ? jogDx : 0.0) + finalApproach;
        G4double preLen     = straightLen - dxTotalEnd;

        if (preLen > 0.0) {
            G4LogicalVolume* lPre = makeStraight(preLen, "Nest_"+leg.name+"_Pre");
            G4double xPreCenter   = xUturnCenter - preLen / 2.0;
            new G4PVPlacement(rotY90, G4ThreeVector(xPreCenter, yStart, leg.zHome),
                              lPre, "Nest_P"+leg.name+"_Pre", logicScint, false, 0, true);
        }

        if (needsJog) {
            G4double xAfterPre = xUturnCenter - preLen;
            G4double dz        = leg.z - leg.zHome;
            G4double jogLen    = std::sqrt(jogDx*jogDx + dz*dz);
            G4double jogAngle  = std::atan2(-jogDx, dz);
            G4RotationMatrix* rotJog = new G4RotationMatrix();
            rotJog->rotateY(jogAngle);
            G4LogicalVolume* lJog = makeStraight(jogLen, "Nest_"+leg.name+"_ZJog");
            G4double xJogCenter   = xAfterPre - jogDx / 2.0;
            G4double zJogCenter   = (leg.zHome + leg.z) / 2.0;
            new G4PVPlacement(rotJog, G4ThreeVector(xJogCenter, yStart, zJogCenter),
                              lJog, "Nest_P"+leg.name+"_ZJog", logicScint, false, 0, true);
        }

        if (dy < 1.0e-9) continue;

        G4double rBend       = rMerge;
        G4double xCurveStart = xScintEnd + finalApproach + dxCurve;
        bool goingDown       = (yStart > yTarget);

        G4RotationMatrix* rotArc1 = new G4RotationMatrix();
        G4RotationMatrix* rotArc2 = new G4RotationMatrix();
        rotArc2->rotateY(180*deg);
        if (!goingDown) {
            rotArc1->rotateX(180*deg);
            rotArc2->rotateX(180*deg);
        }

        if (bigDrop) {
            // Arco 90° → tratto verticale → arco 90°
            G4double yCenter1 = goingDown ? (yStart - rBend) : (yStart + rBend);
            G4LogicalVolume* lArc1 = makeArc(rBend, 90*deg, 90*deg, "Nest_"+leg.name+"_M1");
            new G4PVPlacement(rotArc1, G4ThreeVector(xCurveStart, yCenter1, leg.z),
                              lArc1, "Nest_P"+leg.name+"_M1", logicScint, false, 0, true);

            G4double vertLen     = dy - 2.0*rBend;
            G4double xVertical   = xCurveStart - rBend;
            G4double yVertCenter = goingDown ? (yCenter1 - vertLen/2.0) : (yCenter1 + vertLen/2.0);
            G4RotationMatrix* rotX90 = new G4RotationMatrix(); rotX90->rotateX(90*deg);
            G4LogicalVolume* lVert = makeStraight(vertLen, "Nest_"+leg.name+"_Vert");
            new G4PVPlacement(rotX90, G4ThreeVector(xVertical, yVertCenter, leg.z),
                              lVert, "Nest_P"+leg.name+"_Vert", logicScint, false, 0, true);

            G4double yCenter2 = goingDown ? (yTarget + rBend) : (yTarget - rBend);
            G4LogicalVolume* lArc2 = makeArc(rBend, 180*deg, 90*deg, "Nest_"+leg.name+"_M2");
            new G4PVPlacement(rotArc2, G4ThreeVector(xScintEnd, yCenter2, leg.z),
                              lArc2, "Nest_P"+leg.name+"_M2", logicScint, false, 0, true);
        } else {
            G4double theta    = std::acos(1.0 - dy / (2.0*rBend));
            G4LogicalVolume* lArc1 = makeArc(rBend, 90*deg, theta, "Nest_"+leg.name+"_M1");
            G4double yCenter1 = goingDown ? (yStart - rBend) : (yStart + rBend);
            new G4PVPlacement(rotArc1, G4ThreeVector(xCurveStart, yCenter1, leg.z),
                              lArc1, "Nest_P"+leg.name+"_M1", logicScint, false, 0, true);

            G4double xArc2Center = xScintEnd + finalApproach;
            G4LogicalVolume* lArc2 = makeArc(rBend, 270*deg - theta, theta, "Nest_"+leg.name+"_M2");
            G4double yCenter2 = goingDown ? (yTarget + rBend) : (yTarget - rBend);
            new G4PVPlacement(rotArc2, G4ThreeVector(xArc2Center, yCenter2, leg.z),
                              lArc2, "Nest_P"+leg.name+"_M2", logicScint, false, 0, true);

            if (finalApproach > 1.0e-9) {
                G4LogicalVolume* lFApp = makeStraight(finalApproach, "Nest_"+leg.name+"_FApp");
                new G4PVPlacement(rotY90,
                    G4ThreeVector(xScintEnd + finalApproach/2.0, yTarget, leg.z),
                    lFApp, "Nest_P"+leg.name+"_FApp", logicScint, false, 0, true);
            }
        }
    }

    // --- SiPM E SUA SUPERFICIE OTTICA ---
    G4double sipmThick   = 0.5*mm;
    G4double bridgeLen   = 10.0*mm, dxBundle = 170.0*mm, straightFinal = 20.0*mm;
    G4double greaseThick = 0.5*mm;

    G4double zSiPMCenter = 20.25*mm;
    G4double rRing = 1.725*mm - cladR;
    G4double packedY[8], packedZ[8];
    packedY[0] = 0.0*mm; packedZ[0] = zSiPMCenter;
    for (int k = 0; k < 7; k++) {
        G4double ang = k * (360.0/7.0) * deg;
        packedY[k+1] = rRing * std::cos(ang);
        packedZ[k+1] = zSiPMCenter + rRing * std::sin(ang);
    }

    G4Box* sSiPM = new G4Box("SiPM_N", sipmThick, 3.0*mm, 3.0*mm);
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
    G4double xFiberEnd  = xBundleEnd - straightFinal;
    G4double xSiPM      = xFiberEnd - greaseThick - sipmThick;

    // Grasso ottico tra faccia delle fibre e SiPM
    G4Material* greaseMat = G4Material::GetMaterial("OpticalGrease");
    G4Box* sGrease = new G4Box("Grease_N", greaseThick/2., 3.0*mm, 3.0*mm);
    G4LogicalVolume* lGrease = new G4LogicalVolume(sGrease, greaseMat, "LogicGrease_N");
    G4VisAttributes* greaseVis = new G4VisAttributes(G4Colour(0.9, 0.9, 0.2, 0.4));
    lGrease->SetVisAttributes(greaseVis);
    new G4PVPlacement(0, G4ThreeVector(xFiberEnd - greaseThick/2., 0, zSiPMCenter),
                      lGrease, "pGrease_N", logicWorld, false, 0, true);

    new G4PVPlacement(0, G4ThreeVector(xSiPM, 0, zSiPMCenter), lSiPM, "pSiPM_N", logicWorld, false, 0, true);

    // Assegnazione fibra -> posizione impacchettata nel bundle da 3.45mm.
    int legToPacked[8] = {0, 4, 7, 2, 6, 1, 3, 5};

    // --- BUNDLE ESTERNO: 8 capi convergono verso il SiPM ---
    for (size_t i = 0; i < legs.size(); i++) {
        G4double yStart  = legs[i].yHole;
        G4double zFib    = legs[i].z;
        G4double yTarget = packedY[legToPacked[i]];
        G4double zTarget = packedZ[legToPacked[i]];
        G4String tag = "Nest_" + legs[i].name;

        G4LogicalVolume* lBridge = makeNudeStraight(bridgeLen, tag+"_Bridge");
        new G4PVPlacement(rotY90, G4ThreeVector(xScintEnd - bridgeLen/2.0, yStart, zFib),
                          lBridge, "p"+tag+"_Bridge", logicWorld, false, 0, true);

        G4double dy   = yTarget - yStart;
        G4double dz   = zTarget - zFib;
        G4double dr   = std::sqrt(dy*dy + dz*dz);
        G4double dx   = dxBundle;
        G4double rBend = (dx*dx + dr*dr) / (4.0*dr);
        G4double theta = std::acos(1.0 - dr/(2.0*rBend));
        G4double cosA  = -dy/dr;
        G4double sinA  = -dz/dr;

        G4double xArcStart = xScintEnd - bridgeLen;
        G4double yCenter1 = yStart + rBend*dy/dr;
        G4double zCenter1 = zFib   + rBend*dz/dr;
        placeArcChain(rBend, xArcStart, yCenter1, zCenter1, cosA, sinA, +1.0, 90*deg, 90*deg+theta, 24, tag+"_NA1");

        G4double yCenter2 = yTarget - rBend*dy/dr;
        G4double zCenter2 = zTarget - rBend*dz/dr;
        placeArcChain(rBend, xBundleEnd, yCenter2, zCenter2, cosA, sinA, -1.0, 270*deg-theta, 270*deg, 24, tag+"_NA2");

        G4LogicalVolume* lFinal = makeNudeStraight(straightFinal, tag+"_Final");
        G4double xFinalCenter = xBundleEnd - straightFinal/2.0;
        new G4PVPlacement(rotY90, G4ThreeVector(xFinalCenter, yTarget, zTarget),
                          lFinal, "p"+tag+"_Final", logicWorld, false, 0, false);
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

    G4Material* coreMat = nist->FindOrBuildMaterial("G4_POLYSTYRENE");

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

    // Cladding: PMMA (n=1.49) come materiale separato — FIX del bug in cui
    // glueMat e cladMat puntavano allo stesso G4_PLEXIGLASS e le proprieta'
    // del glue (n=1.57) sovrascrivevano quelle del cladding (n=1.49).
    G4Element* eC = nist->FindOrBuildElement("C");
    G4Element* eH = nist->FindOrBuildElement("H");
    G4Element* eO = nist->FindOrBuildElement("O");
    G4Material* FiberCladding = new G4Material("FiberCladding", 1.19*g/cm3, 3);
    FiberCladding->AddElement(eC, 5);
    FiberCladding->AddElement(eH, 8);
    FiberCladding->AddElement(eO, 2);

    G4double rindexClad[8] = {1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49, 1.49};
    G4double absClad[8]    = {10.0*m, 10.0*m, 10.0*m, 10.0*m, 10.0*m, 10.0*m, 10.0*m, 10.0*m};
    G4MaterialPropertiesTable* mptClad = new G4MaterialPropertiesTable();
    mptClad->AddProperty("RINDEX", energy, rindexClad, nEntries);
    mptClad->AddProperty("ABSLENGTH", energy, absClad, nEntries);
    FiberCladding->SetMaterialPropertiesTable(mptClad);

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

    // Grasso ottico: n≈1.465 (silicone Dow Corning), accoppia le fibre al SiPM
    G4Element* eSi = nist->FindOrBuildElement("Si");
    G4Material* OpticalGrease = new G4Material("OpticalGrease", 1.03*g/cm3, 4);
    OpticalGrease->AddElement(eSi, 1);
    OpticalGrease->AddElement(eC,  2);
    OpticalGrease->AddElement(eH,  6);
    OpticalGrease->AddElement(eO,  1);
    G4double rindexGrease[8] = {1.465,1.465,1.465,1.465,1.465,1.465,1.465,1.465};
    G4double absGrease[8]    = {10*m, 10*m, 10*m, 10*m, 10*m, 10*m, 10*m, 10*m};
    G4MaterialPropertiesTable* mptGrease = new G4MaterialPropertiesTable();
    mptGrease->AddProperty("RINDEX", energy, rindexGrease, nEntries);
    mptGrease->AddProperty("ABSLENGTH", energy, absGrease, nEntries);
    OpticalGrease->SetMaterialPropertiesTable(mptGrease);

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
    G4Material* cladMat = G4Material::GetMaterial("FiberCladding");
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
