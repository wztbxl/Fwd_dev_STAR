/**************************************************************************
 *
 * StFttPoint.h
 *
 * Author: jdb 2021
 **************************************************************************
 *
 * Description: Declaration of StFttPoint, the StEvent FTT point structure
 * Represents a "point" (cluster centroid etc) via cluster finding.
 *
 **************************************************************************/
#ifndef StFttPoint_h
#define StFttPoint_h

#include "StThreeVectorD.hh"
#include "StObject.h"
#include "StEnumerations.h"
#include "StContainers.h"

class StFttCluster;

class StFttPoint : public StObject {
public:
    StFttPoint();
    ~StFttPoint();

    UChar_t plane()   const; // Detector plane.
    UChar_t quadrant()   const; // detector quadrant.
    float x() const;  // x position in cell unit at which point intersects the sub-detector in local coordinate
    float y() const;  // y position in cell unit at which point intersects the sub-detector in local coordinate
    float diag1() const;
    float diag2() const;
    int nClusters() const; // Number of points in the parent cluster.
    StFttCluster* cluster( size_t i); //  Parent cluster of the photon.
    const StThreeVectorD& xyz() const; // XYZ position in global STAR coordinate

    void setPlane(UChar_t plane);
    void setQuadrant(UChar_t quad);
    void setX(float x);
    void setY(float y);
    void setDiag1(float diag1);
    void setDiag2(float diag2);
    void addCluster(StFttCluster* cluster, UChar_t dir);
    void setXYZ(const StThreeVectorD& p3);

    
    void print(int option=0);

private:
    UChar_t mPlane = 99;
    UChar_t mQuadrant = 99;
    Float_t  mX=-999.;         // x-position in local coordinate
    Float_t  mY=-999.;         // y-position in local coordinate
    Float_t  mDiag1 = -999.;
    Float_t  mDiag2 = -999.;
    StFttCluster *mClusters[4];
    StThreeVectorD  mXYZ;    // Photon position in STAR coordinate

    ClassDef(StFttPoint, 1)
};

inline UChar_t StFttPoint::plane() const { return mPlane; }
inline UChar_t StFttPoint::quadrant() const { return mQuadrant; }
inline float StFttPoint::x() const { return mX; } // x position (cm) in local coords.
inline float StFttPoint::y() const { return mY; } // y position (cm) in local coords.
inline float StFttPoint::diag1() const { return mDiag1; } // x position (cm) in local coords.
inline float StFttPoint::diag2() const { return mDiag2; } // y position (cm) in local coords.
inline StFttCluster* StFttPoint::cluster( size_t i ) { if ( i < 4 ) return mClusters[i]; return nullptr; } //  Parent cluster of the photon.
inline const StThreeVectorD& StFttPoint::xyz() const { return mXYZ; }
inline void StFttPoint::setPlane(UChar_t plane) { mPlane = plane; }
inline void StFttPoint::setQuadrant(UChar_t quadrant) { mQuadrant = quadrant; }
inline void StFttPoint::setX(float xpos) { mX = xpos; }
inline void StFttPoint::setY(float ypos) { mY = ypos; }
inline void StFttPoint::setDiag1(float diag1) { mDiag1 = diag1; }
inline void StFttPoint::setDiag2(float diag2) { mDiag2 = diag2; }
inline void StFttPoint::addCluster(StFttCluster* cluster, UChar_t dir) { mClusters[dir] = (cluster); }
inline void StFttPoint::setXYZ(const StThreeVectorD& p3) { mXYZ = p3; }

#endif  // StFttPoint_h

