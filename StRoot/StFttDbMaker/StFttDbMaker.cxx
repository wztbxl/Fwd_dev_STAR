/***************************************************************************
 * $id: StFttDbMaker.cxx,v 1.0 2020/12/17 21:01:04 jdb Exp $
 * \author: jdb
 ***************************************************************************
 *
 * Description: This maker is the interface between FCS and the STAR database
 *
 ***************************************************************************/

#include "StFttDbMaker.h"
#include "StFttDb.h"
#include "St_db_Maker/St_db_Maker.h"
#include "StMessMgr.h"

ClassImp(StFttDbMaker)

StFttDbMaker::StFttDbMaker(const char *name) : StMaker(name){
  LOG_INFO << "******** StFttDbMaker::StFttDbMaker = "<<name<<endm;
  mFttDb = new StFttDb("fttDb");
  // mFttDb->SetDebug();
  // mFttDb->setDebug();
  AddData(mFttDb,".const");
}; 

StFttDbMaker::~StFttDbMaker() {
  delete mFttDb;
}

int StFttDbMaker::Init(){
  mFttDb->Init();
  return StMaker::Init();
}

void StFttDbMaker::Clear(Option_t *option){
  StMaker::Clear(option);
}

int StFttDbMaker::Make(){
  return StMaker::Make();
}

int StFttDbMaker::InitRun(int runNumber) {
  LOG_INFO << "StFttDbMaker::InitRun - run = " << runNumber << endm;

  mFttDb->loadHardwareMapFromFile( "/star/u/wangzhen/sTGC/Commissioning/ClusterFinder/PointMaker_building_test_0616/star-sw-1/StRoot/StFwdTrackMaker/macro/vmm_map.dat" );
  mFttDb->loadStripCenterFromFile( "/star/u/wangzhen/sTGC/Commissioning/ClusterFinder/PointMaker_building_test_0616/star-sw-1/StRoot/StFwdTrackMaker/macro/Row1.txt" );
  mFttDb->loadStripEdgeFromFile( "/star/u/wangzhen/sTGC/Commissioning/ClusterFinder/PointMaker_building_test_0616/star-sw-1/StRoot/StFwdTrackMaker/macro/Row4_edge.txt" );
  mFttDb->loadStripCenterFromFile( "/star/u/wangzhen/sTGC/Commissioning/ClusterFinder/PointMaker_building_test_0616/star-sw-1/StRoot/StFwdTrackMaker/macro/Row4.txt" );

  return kStOK;
}
