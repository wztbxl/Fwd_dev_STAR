/***************************************************************************
 *
 * $Id: StFttClusterMaker.h,v 1.3 2019/02/19 20:32:05 fseck Exp $
 *
 * Author: Florian Seck, April 2018
 ***************************************************************************
 *
 * Description: StFttClusterMaker - class to fill the StEvent from DAQ reader:
 * unpack raw data & save StETofHeader & StETofDigis in StETofCollection 
 *
 ***************************************************************************/
#ifndef STFTTCLUSTERMAKER_H
#define STFTTCLUSTERMAKER_H
#include "StMaker.h"
#include <vector>
#include <map>



class StFttDb;
class StEvent;
class StFttCollection;
class StFttRawHit;
class StFttCluster;

class StFttClusterMaker: public StMaker {

public:
    StFttClusterMaker( const char* name = "stgcCluster" );

    ~StFttClusterMaker();


    Int_t  Init();
    Int_t  InitRun( Int_t );
    Int_t  FinishRun( Int_t );
    Int_t  Finish();
    Int_t  Make();

private:
    void ApplyHardwareMap();
    std::vector<StFttCluster*> FindClusters( std::vector<StFttRawHit * >, UChar_t );
    std::vector<StFttCluster*> FindClusters( std::vector<StFttRawHit * > );
    StFttRawHit * FindMaxAdc( std::vector<StFttRawHit *>, size_t &pos );

    void InjectTestData();
    void SearchClusterEdges( std::vector< StFttRawHit * > hits, 
                                            size_t start, // start index at MaxADC
                                            size_t &left, size_t &right);

    void CalculateClusterInfo( StFttCluster * clu );
    float GetThresholdFor( StFttRawHit * hit ) { return 0.0;}
    bool PassTimeCut( StFttRawHit * hit );

    StEvent*             mEvent;
    StFttCollection*     mFttCollection;
    Int_t                mRunYear;
    Bool_t               mDebug;
    StFttDb*             mFttDb;


    ClassDef( StFttClusterMaker, 1 )
};

#endif // STFTTCLUSTERMAKER_H
