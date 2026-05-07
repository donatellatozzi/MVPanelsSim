#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "G4String.hh"
#include "G4ThreeVector.hh"
#include "G4OpticalSurface.hh"
#include <vector>

class G4VPhysicalVolume;
class G4LogicalVolume;
class DetectorMessenger; // <--- Dichiarazione della nuova classe

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction();
    virtual ~DetectorConstruction();

    virtual G4VPhysicalVolume* Construct();
    virtual void ConstructSDandField();

    // Metodo per cambiare il materiale da macro!
    void SetWrappingType(G4String val);
    void SetGeometryType(G4String val);

  private:
    G4String fWrappingType;
    G4String fGeometryType;
    G4OpticalSurface* fOpWrapping;
    DetectorMessenger* fMessenger; // <--- Puntatore al Messenger

    void DefineMaterials();

    // Dividiamo la costruzione in due sottometodi privati:
    G4VPhysicalVolume* ConstructStandardPanel();
    G4VPhysicalVolume* ConstructSpiralPanel();
  
    void BuildContinuousSpiralSector(G4LogicalVolume* motherVolume, G4ThreeVector center, G4int sectorID, G4ThreeVector sipmPos);
   
};

#endif
