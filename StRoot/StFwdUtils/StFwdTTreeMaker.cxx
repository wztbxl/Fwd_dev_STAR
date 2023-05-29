#include "StFwdUtils/StFwdTTreeMaker.h"
#include "StFwdTrackMaker/Common.h"

#include "TMath.h"

#include <limits>
#include <map>
#include <string>
#include <string>
#include <vector>

#include "StBFChain/StBFChain.h"

#include "StEvent/StEvent.h"
#include "StEvent/StGlobalTrack.h"
#include "StEvent/StHelixModel.h"
#include "StEvent/StPrimaryTrack.h"
#include "StEvent/StRnDHit.h"
#include "StEvent/StRnDHitCollection.h"
#include "StEvent/StTrack.h"
#include "StEvent/StTrackGeometry.h"
#include "StEvent/StTrackNode.h"
#include "StEvent/StPrimaryVertex.h"
#include "StEvent/StEnumerations.h"
#include "StEvent/StTrackDetectorInfo.h"
#include "StEvent/StFttPoint.h"
#include "StEvent/StFcsHit.h"
#include "StEvent/StFcsCluster.h"
#include "StEvent/StFttCollection.h"
#include "StEvent/StFcsCollection.h"
#include "StEvent/StTriggerData.h"
#include "StEvent/StFstHitCollection.h"
#include "StEvent/StFstHit.h"
#include "StEvent/StFwdTrackCollection.h"
#include "StChain/StChainOpt.h"

#include "StEventUtilities/StEventHelper.h"

#include "StMuDSTMaker/COMMON/StMuDstMaker.h"
#include "StMuDSTMaker/COMMON/StMuDst.h"
#include "StMuDSTMaker/COMMON/StMuFwdTrack.h"
#include "StMuDSTMaker/COMMON/StMuFcsCluster.h"


#include "tables/St_g2t_fts_hit_Table.h"
#include "tables/St_g2t_track_Table.h"
#include "tables/St_g2t_vertex_Table.h"
#include "tables/St_g2t_event_Table.h"

#include "StarMagField/StarMagField.h"

#include "St_base/StMessMgr.h"
#include "StarClassLibrary/StPhysicalHelix.hh"
#include "StarClassLibrary/SystemOfUnits.h"


#include "TROOT.h"
#include "TLorentzVector.h"
#include "StEvent/StFwdTrack.h"

#include "StFwdUtils/StFwdTTreeData.h"



























void copyFwdTrack ( StMuFwdTrack *from, StMuFwdTrack *to ){
    to->setPrimaryMomentum( from->momentum() );
    to->setDidFitConverge( from->didFitConverge() );
    to->setDidFitConvergeFully( from->didFitConvergeFully() );
    to->setNumberOfFailedPoints( from->numberOfFailedPoints() );
    to->setNumberOfSeedPoints( from->numberOfSeedPoints() );
    to->setNumberOfFitPoints( from->numberOfFitPoints() );
    to->setChi2( from->chi2() );
    to->setNDF( from->ndf() );
    to->setPval( from->pval() );
    to->setCharge( from->charge() );
}



void copyFcsClu( StMuFcsCluster *from, StMuFcsCluster *to){
    to->setId( from->id() );
    to->setDetectorId( from->detectorId() );
    to->setCategory( from->category() );
    to->setNTowers( from->nTowers() );
    to->setEnergy( from->energy() );
    to->setX( from->x() );
    to->setY( from->y() );
    to->setSigmaMin( from->sigmaMin() );
    to->setSigmaMax( from->sigmaMax() );
    to->setTheta( from->theta() );
    to->setChi2Ndf1Photon( from->chi2Ndf1Photon() );
    to->setChi2Ndf2Photon( from->chi2Ndf2Photon() );
    to->setFourMomentum( from->fourMomentum() );
}

//________________________________________________________________________
StFwdTTreeMaker::StFwdTTreeMaker() : StMaker("fwdTTree"){};

int StFwdTTreeMaker::Finish() { 
    LOG_INFO << "StFwdTTreeMaker::Finish" << endm;
    LOG_INFO << "mTreeFile: " << mTreeFile << ", mTree: " << mTree << endm;
    if ( mTreeFile ){
        LOG_INFO << "Writing TTREE" << endm;
        auto td = gDirectory;
        mTreeFile->cd();
        mTree->Write();
        mTreeFile->Write();
        LOG_INFO << "Done Writing TTREE" << endm;

        td->cd();
    }
    return kStOk;
}
//________________________________________________________________________
int StFwdTTreeMaker::Init() { 
    LOG_INFO << "StFwdTTreeMaker::Init" << endm; 

    mTreeFile = new TFile("StFwdTTree.root", "RECREATE");
    mTree = new TTree("fwd", "fwd tracking tree");
    tracksWriter.createBranch( mTree, "StMuFwdTrack", 256000, 99);
    ecalWriter.createBranch( mTree, "StMuFcsCluster", 256000, 99 );
    // mTree->Branch("data",  &mTreeData);  

    // mTree->SetAutoFlush(0);
    LOG_INFO << "mTreeFile: " << mTreeFile << ", mTree: " << mTree << endm;

    return kStOK;
}
//________________________________________________________________________
int StFwdTTreeMaker::Make() {
    LOG_DEBUG << "StFwdTTreeMaker::Make" << endm;
    StEvent *event = (StEvent *)GetDataSet("StEvent");
    
    long long itStart = FwdTrackerUtils::nowNanoSecond();

    tracksWriter.reset();
    ecalWriter.reset();


    LOG_INFO << "StFwdAnalysisMaker::ProcessFwdMuTracks" << endm;
    StMuDstMaker *mMuDstMaker = (StMuDstMaker *)GetMaker("MuDst");
    if(!mMuDstMaker) {
        LOG_WARN << " No MuDstMaker ... bye-bye" << endm;
        return kStOK;
    }
    StMuDst *mMuDst = mMuDstMaker->muDst();
    if(!mMuDst) {
        LOG_WARN << " No MuDst ... bye-bye" << endm;
        return kStOK;
    }
    StMuFwdTrackCollection * ftc = mMuDst->muFwdTrackCollection();
    if (!ftc) return kStOK;
    cout << "Number of StMuFwdTracks: " << ftc->numberOfFwdTracks() << endl;
    
    for ( size_t iTrack = 0; iTrack < ftc->numberOfFwdTracks(); iTrack++ ){
        StMuFwdTrack * muFwdTrack = ftc->getFwdTrack( iTrack );
        // LOG_INFO << TString::Format("StMuFwdTrack[ nProjections=%lu, nFTTSeeds=%lu, nFSTSeeds=%lu, mPt=%f ]", muFwdTrack->mProjections.size(), muFwdTrack->mFTTPoints.size(), muFwdTrack->mFSTPoints.size(), muFwdTrack->momentum().Pt()) << endm;
        LOG_INFO << "muFwdTrack.mEcalClusters entries = " << muFwdTrack->mEcalClusters.GetEntries() << endm;
        auto t = tracksWriter.add( muFwdTrack );
        copyFwdTrack( muFwdTrack, t );
        // t.copy( muFwdTrack );

        for ( int i = 0; i < muFwdTrack->mEcalClusters.GetEntries(); i++ ){
            StMuFcsCluster * ecalClu = static_cast<StMuFcsCluster*>(muFwdTrack->mEcalClusters.At(i));
            auto c = ecalWriter.add( ecalClu );
            LOG_INFO << "Ecal Clu with Energy = " << ecalClu->energy() << endm;;
            copyFcsClu( ecalClu, c );
            t->addEcalCluster( c );
        }
        
    }

    // StFwdTTreeTrackData track;
    // for (int i = 0; i < 10; i++){
    //     track.pt = 10;
    //     track.eta = 0.1;

    //     tracksWriter.add( &track );
    // }
    
    mTree->Fill();
    
    LOG_DEBUG << "Processing Fwd Tracks took: " << (FwdTrackerUtils::nowNanoSecond() - itStart) * 1e6 << " ms" << endm;
    return kStOK;
} // Make
//________________________________________________________________________
void StFwdTTreeMaker::Clear(const Option_t *opts) { LOG_DEBUG << "StFwdTTreeMaker::CLEAR" << endm; }
//________________________________________________________________________
