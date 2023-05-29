#ifndef ST_FWD_TTREE_MAKER_H
#define ST_FWD_TTREE_MAKER_H

#include "StChain/StMaker.h"
#include "TVector3.h"
// ROOT includes
#include "TNtuple.h"
#include "TTree.h"
// STL includes
#include <vector>
#include <memory>

#include "StFwdUtils/StFwdTTreeData.h"
#include "StFwdUtils/BranchWriter.h"
#include "StFwdUtils/TClonesArrayWriter.h"

class StMuFwdTrack;
class StMuFcsCluster;


class StFwdTTreeMaker : public StMaker {

    ClassDef(StFwdTTreeMaker, 0);

  public:
    StFwdTTreeMaker();
    ~StFwdTTreeMaker(){/* nada */};

    int Init();
    int Finish();
    int Make();
    void Clear(const Option_t *opts = "");

  private:
  protected:

    TFile *mTreeFile = nullptr;
    TTree *mTree     = nullptr;
    TClonesArrayWriter<StMuFwdTrack> tracksWriter;
    TClonesArrayWriter<StMuFcsCluster> ecalWriter;
    // FwdTreeData mTreeData;
    StFwdTTreeData mTreeData;
};

#endif
