#ifndef ST_FWD_VISUALIZATION_MAKER_H
#define ST_FWD_VISUALIZATION_MAKER_H

#include "StChain/StMaker.h"
#include "TVector3.h"
// ROOT includes
#include "TNtuple.h"
#include "TTree.h"
// STL includes
#include <vector>
#include <memory>
#include <map>

#include "StFcsDbMaker/StFcsDb.h"
#include "StMuDSTMaker/COMMON/StMuDstMaker.h"
#include "StMuDSTMaker/COMMON/StMuDst.h"
#include "StMuDSTMaker/COMMON/StMuFwdTrack.h"

class StFwdTrack;


class StFwdVisualizationMaker : public StMaker {

    ClassDef(StFwdVisualizationMaker, 0);

  public:
    StFwdVisualizationMaker() {}
    ~StFwdVisualizationMaker(){/* nada */};

    int Init() { return kStOk;}
    int Finish() { return kStOk; }
    int Make() {

        // StMuDstMaker *mMuDstMaker = (StMuDstMaker *)GetMaker("MuDst");
        // if(!mMuDstMaker) {
        //     LOG_WARN << " No MuDstMaker ... bye-bye" << endm;
        //     return;
        // }
        // StMuDst *mMuDst = mMuDstMaker->muDst();
        // if(!mMuDst) {
        //     LOG_WARN << " No MuDst ... bye-bye" << endm;
        //     return kStOk;
        // }
        // StMuFcsCollection *fcs = mMuDst->muFcsCollection();
        // if (!fcs) return kStOk;

        // cout << "Number of StMuFwdTracks: " << ftc->numberOfFwdTracks() << endl;

        // StFcsDb *mFcsDb = static_cast<StFcsDb *>(GetDataSet("fcsDb"));

        // for( int i = 0; i < fcs->numberOfClusters(); i++){
        //     StMuFcsCluster * clu = fcs->getCluster(i);

        //     if ( clu->energy() < 1 ) continue;
        //     StThreeVectorD xyz = mFcsDb->getStarXYZfromColumnRow(clu->detectorId(), clu->x(), clu->y());

        //     float dx = ecalProj.mXYZ.X() - xyz.y();
        //     float dy = ecalProj.mXYZ.Y() - xyz.x();
        //     float dr = sqrt(dx*dx + dy*dy);

        //     mHists[ "ecaldX" ]->Fill( dx );
        //     mHists[ "ecaldY" ]->Fill( dy );
        //     mHists[ "ecaldR" ]->Fill( dr );

        //     mHists[ "trkEcalX" ] -> Fill( ecalProj.mXYZ.X(), xyz.x() );

        // }

        return kStOk;
    }
    void Clear(const Option_t *opts = "") {}

  private:
  protected:
};

#endif
