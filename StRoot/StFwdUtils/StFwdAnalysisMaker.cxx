#include "StFwdUtils/StFwdAnalysisMaker.h"
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
#include "StFcsDbMaker/StFcsDb.h"

//________________________________________________________________________
StFwdAnalysisMaker::StFwdAnalysisMaker() : StMaker("fwdAna"){};
int StFwdAnalysisMaker::Finish() { 
    
    auto prevDir = gDirectory;
        
    // output file name
    string name = "FwdAna.root";
    TFile *fOutput = new TFile(name.c_str(), "RECREATE");
    fOutput->cd();
    for (auto nh : mHists) {
        nh.second->SetDirectory(gDirectory);
        nh.second->Write();
    }

    // restore previous directory
    gDirectory = prevDir;

    return kStOk; 
}
//________________________________________________________________________
int StFwdAnalysisMaker::Init() { 
    LOG_DEBUG << "StFwdAnalysisMaker::Init" << endm; 

    mHists["fwdMult"] = new TH1F("fwdMult", ";N_{ch}^{FWD}; counts", 100, 0, 100);
    mHists["fwdMultFST"] = new TH1F("fwdMultFST", ";N_{ch}^{FWD}; counts", 100, 0, 100);
    mHists["fwdMultEcalMatch"] = new TH1F("fwdMultEcalMatch", ";N_{ch}^{FWD}; counts", 100, 0, 100);
    mHists["fwdMultHcalMatch"] = new TH1F("fwdMultHcalMatch", ";N_{ch}^{FWD}; counts", 100, 0, 100);
    mHists["eta"] = new TH1F("eta", ";#eta; counts", 100, 0, 5);
    mHists["phi"] = new TH1F("phi", ";#phi; counts", 100, -3.1415926, 3.1415926);

    mHists["ecalMatchPerTrack"] = new TH1F("ecalMatchPerTrack", ";N_{match} / track; counts", 5, 0, 5);
    mHists["hcalMatchPerTrack"] = new TH1F("hcalMatchPerTrack", ";N_{match} / track; counts", 5, 0, 5);

    mHists["ecalEnergy"] = new TH1F("ecalEnergy", ";Energy; counts", 100, 0, 15);
    mHists["hcalEnergy"] = new TH1F("hcalEnergy", ";Energy; counts", 100, 0, 15);

    mHists["ecalXY"] = new TH2F( "ecalXY", ";ecalX;ecalY", 200, -200, 200, 200, -200, 200 );
    mHists["hcalXY"] = new TH2F( "hcalXY", ";hcalX;hcalY", 200, 0, 50, 200, 0, 50 );

    mHists[ "ecaldX" ] = new TH1F( "ecaldX", ";dx (trk - ecal); counts", 400, -200, 200 );
    mHists[ "ecaldY" ] = new TH1F( "ecaldY", ";dy (trk - ecal); counts", 400, -200, 200 );
    mHists[ "ecaldR" ] = new TH1F( "ecaldR", ";dr (trk - ecal); counts", 400, 0, 400 );

    mHists[ "trkEcalX" ] = new TH2F( "trkEcalX", ";trkX;ecalX", 200, -50, 50, 200, -50, 50 );

    return kStOK;
}
//________________________________________________________________________
int StFwdAnalysisMaker::Make() {
    LOG_DEBUG << "StFwdAnalysisMaker::Make" << endm;
    StEvent *event = (StEvent *)GetDataSet("StEvent");
    if (event){
        StFttCollection *fttCol = event->fttCollection();
        if (fttCol){
            LOG_INFO << "The Ftt Collection has " << fttCol->numberOfPoints() << " points" << endm;
        }
    }
    long long itStart = FwdTrackerUtils::nowNanoSecond();
    ProcessFwdTracks();
    ProcessFwdMuTracks();
    LOG_DEBUG << "Processing Fwd Tracks took: " << (FwdTrackerUtils::nowNanoSecond() - itStart) * 1e6 << " ms" << endm;
    return kStOK;
} // Make
//________________________________________________________________________
void StFwdAnalysisMaker::Clear(const Option_t *opts) { LOG_DEBUG << "StFwdAnalysisMaker::CLEAR" << endm; }
//________________________________________________________________________
void StFwdAnalysisMaker::ProcessFwdTracks(  ){
    // This is an example of how to process fwd track collection
    LOG_INFO << "StFwdAnalysisMaker::ProcessFwdTracks" << endm;
    StEvent *stEvent = static_cast<StEvent *>(GetInputDS("StEvent"));
    if (!stEvent)
        return;
    StFwdTrackCollection * ftc = stEvent->fwdTrackCollection();
    if (!ftc)
        return;
    for ( auto fwdTrack : ftc->tracks() ){
        LOG_INFO << TString::Format("StFwdTrack[ nProjections=%lu, nFTTSeeds=%lu, nFSTSeeds=%lu, mPt=%f ]", fwdTrack->mProjections.size(), fwdTrack->mFTTPoints.size(), fwdTrack->mFSTPoints.size(), fwdTrack->momentum().perp()) << endm;
        for ( auto proj : fwdTrack->mProjections ) {
            LOG_DEBUG << TString::Format("Proj[ %d, %f, %f, %f ]", proj.mDetId, proj.mXYZ.x(), proj.mXYZ.y(), proj.mXYZ.z() ) << endm;
        }
    }
}

//________________________________________________________________________
void StFwdAnalysisMaker::ProcessFwdMuTracks(  ){
    // This is an example of how to process fwd track collection
    LOG_INFO << "StFwdAnalysisMaker::ProcessFwdMuTracks" << endm;
    StMuDstMaker *mMuDstMaker = (StMuDstMaker *)GetMaker("MuDst");
    if(!mMuDstMaker) {
        LOG_WARN << " No MuDstMaker ... bye-bye" << endm;
        return;
    }
    StMuDst *mMuDst = mMuDstMaker->muDst();
    if(!mMuDst) {
        LOG_WARN << " No MuDst ... bye-bye" << endm;
        return;
    }
    StMuFwdTrackCollection * ftc = mMuDst->muFwdTrackCollection();
    if (!ftc) return;

    StMuFcsCollection *fcs = mMuDst->muFcsCollection();
    if (!fcs) return;

    cout << "Number of StMuFwdTracks: " << ftc->numberOfFwdTracks() << endl;

    StFcsDb *mFcsDb = static_cast<StFcsDb *>(GetDataSet("fcsDb"));
    

    size_t fwdMultFST = 0;
    size_t fwdMultEcalMatch = 0;
    size_t fwdMultHcalMatch = 0;

    for ( size_t iTrack = 0; iTrack < ftc->numberOfFwdTracks(); iTrack++ ){
        StMuFwdTrack * muFwdTrack = ftc->getFwdTrack( iTrack );
        LOG_DEBUG << TString::Format("StMuFwdTrack[ nProjections=%lu, nFTTSeeds=%lu, nFSTSeeds=%lu, mPt=%f ]", muFwdTrack->mProjections.size(), muFwdTrack->mFTTPoints.size(), muFwdTrack->mFSTPoints.size(), muFwdTrack->momentum().Pt()) << endm;

        mHists["eta"]->Fill( muFwdTrack->momentum().Eta() );
        mHists["phi"]->Fill( muFwdTrack->momentum().Phi() );

        if (muFwdTrack->mFSTPoints.size() > 0){
            fwdMultFST ++;
        }

        if (muFwdTrack->mEcalClusters.GetEntries() > 0)
            fwdMultEcalMatch++;
        if (muFwdTrack->mHcalClusters.GetEntries() > 0)
            fwdMultHcalMatch++;

        
        // ecal proj
        float c[9];
        int detId = kFcsWcalId;
        TVector3 ecalXYZ;
        TVector3 ecapP;

        StMuFwdTrackProjection ecalProj;
        bool foundEcalProj = muFwdTrack->getProjectionFor( detId, ecalProj, 0 );

        if (foundEcalProj){

            // LOG_INFO << "EcalProj z= " << ecalProj.mXYZ.Z() << endm;
            for( int i = 0; i < fcs->numberOfClusters(); i++){
                StMuFcsCluster * clu = fcs->getCluster(i);

                if ( clu->energy() < 1 ) continue;
                StThreeVectorD xyz = mFcsDb->getStarXYZfromColumnRow(clu->detectorId(), clu->x(), clu->y());

                float dx = ecalProj.mXYZ.X() - xyz.x();
                float dy = ecalProj.mXYZ.Y() - xyz.y();
                float dr = sqrt(dx*dx + dy*dy);

                mHists[ "ecaldX" ]->Fill( dx );
                mHists[ "ecaldY" ]->Fill( dy );
                mHists[ "ecaldR" ]->Fill( dr );

                mHists[ "trkEcalX" ] -> Fill( ecalProj.mXYZ.X(), xyz.x() );

            }

            
        }

        
        for ( size_t i = 0; i < muFwdTrack->mEcalClusters.GetEntries(); i++ ){
            auto c = (StMuFcsCluster*) muFwdTrack->mEcalClusters.At(i);
            if (!c) continue;
            mHists["ecalEnergy"]->Fill( c->energy() );
            
            StThreeVectorD xyz = mFcsDb->getStarXYZfromColumnRow(c->detectorId(), c->x(), c->y());
            mHists["ecalXY"]->Fill( xyz.x(), xyz.y() );

            
        }

        mHists["ecalMatchPerTrack"]->Fill( muFwdTrack->mEcalClusters.GetEntries() );
        mHists["hcalMatchPerTrack"]->Fill( muFwdTrack->mHcalClusters.GetEntries() );

        for ( size_t i = 0; i < muFwdTrack->mHcalClusters.GetEntries(); i++ ){
            auto c = (StMuFcsCluster*) muFwdTrack->mHcalClusters.At(i);
            if (!c) continue;
            mHists["hcalEnergy"]->Fill( c->energy() );

            mHists["hcalXY"]->Fill( c->x(), c->y() );
        }

    }
    mHists["fwdMult"]->Fill( ftc->numberOfFwdTracks() );
    mHists["fwdMultFST"]->Fill( fwdMultFST );
    mHists["fwdMultHcalMatch"]->Fill( fwdMultHcalMatch );
    mHists["fwdMultEcalMatch"]->Fill( fwdMultEcalMatch );

}
