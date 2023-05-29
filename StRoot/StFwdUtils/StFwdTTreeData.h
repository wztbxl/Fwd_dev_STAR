#ifndef STFWDTTREEDATA_H
#define STFWDTTREEDATA_H

#include "TROOT.h"
#include "TObject.h"

class StFwdTTreeTrackData : public TObject {
    public:

    const char* classname() const { return "StMuFwdTrack"; }
    void copy ( StFwdTTreeTrackData * that ){
        this->pt = that->pt;
        this->eta = that->eta;
        this->phi = that->phi;
        this->charge = that->charge;
    }

    float pt, eta, phi;
    short charge;

    ClassDef(StFwdTTreeTrackData,1)
};


class StFwdTTreeData : public TObject {
public:

    const char* classname() const { return "classname"; }
    vector<StFwdTTreeTrackData> tracks;

    ClassDef( StFwdTTreeData, 1)
};


#endif