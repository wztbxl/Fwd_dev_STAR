/***************************************************************************
 *
 * $Id: StFttClusterMaker.cxx,v 1.4 2019/03/08 18:45:40 fseck Exp $
 *
 * Author: Florian Seck, April 2018
 ***************************************************************************
 *
 * Description: StFttClusterMaker - class to fill the StEvent from DAQ reader:
 * unpack raw data & save StETofHeader & StETofDigis in StETofCollection 
 *
 ***************************************************************************/
#include <vector>
#include <set>
#include <map>
#include <array>
#include <algorithm>    // std::is_sorted


#include "StEvent.h"
#include "StEnumerations.h"

#include "StFttClusterMaker.h"


#include "StEvent/StFttRawHit.h"
#include "StEvent/StFttCluster.h"
#include "StEvent/StEvent.h"
#include "StEvent/StFttCollection.h"

#include "StFttDbMaker/StFttDb.h"


//_____________________________________________________________
StFttClusterMaker::StFttClusterMaker( const char* name )
: StMaker( name ),
  mEvent( 0 ),          /// pointer to StEvent
  mRunYear( 0 ),        /// year in which the data was taken (switch at 1st Oct)
  mDebug( false ),       /// print out of all full messages for debugging
  mFttDb( nullptr )
{
    LOG_DEBUG << "StFttClusterMaker::ctor"  << endm;
}

//_____________________________________________________________
StFttClusterMaker::~StFttClusterMaker()
{  /* no op */

}

//_____________________________________________________________
Int_t
StFttClusterMaker::Init()
{
    LOG_INFO << "StFttClusterMaker::Init" << endm;

    return kStOk;
}

//_____________________________________________________________
Int_t
StFttClusterMaker::InitRun( Int_t runnumber )
{ 
    mRunYear = ( runnumber + 727000 ) / 1000000 + 1999;

    LOG_INFO << "runnumber: " << runnumber << "  --> year: " << mRunYear << endm;

    return kStOk;
}

//_____________________________________________________________
Int_t
StFttClusterMaker::FinishRun( Int_t runnumber )
{ 
    return kStOk;
}

//_____________________________________________________________
Int_t
StFttClusterMaker::Finish()
{ 
    return kStOk;
}


//_____________________________________________________________
Int_t
StFttClusterMaker::Make()
{ 
    LOG_INFO << "StFttClusterMaker::Make()" << endm;


    mEvent = (StEvent*)GetInputDS("StEvent");
    if(mEvent) {
        LOG_DEBUG<<"Found StEvent"<<endm;
    } else {
        return kStOk;
    }
    mFttCollection=mEvent->fttCollection();
    if(!mFttCollection) {
        LOG_WARN << "No StFttCollection" << endm;
        return kStOk;
    }

    mFttDb = static_cast<StFttDb*>(GetDataSet("fttDb"));

    assert( mFttDb );
    LOG_INFO << "StFttClusterMaker found " << mFttCollection->rawHits().size() << " raw Ftt hits to work with" << endm;
    ApplyHardwareMap();

    // InjectTestData();

    // next we need to sort the hits into 1D projections
    // process 1 quadrant (ROB) at a time,
    // process horizontal, vertical or diagonal strips one at a time

    // key == ROB
    std::map< UChar_t, std::vector<StFttRawHit *> > hStripsPerRob; // Horizontal
    std::map< UChar_t, std::vector<StFttRawHit *> > vStripsPerRob; // Vertical
    std::map< UChar_t, std::vector<StFttRawHit *> > hdStripsPerRob; // Diagonal on Horizontal
    std::map< UChar_t, std::vector<StFttRawHit *> > vdStripsPerRob; // Diagonal on Vertical

    size_t nStripsHit = 0; //number of good hits
    for ( StFttRawHit* hit : mFttCollection->rawHits() ) {
        UChar_t rob = mFttDb->rob( hit );
        UChar_t so = mFttDb->orientation( hit );

        // LOG_INFO << "StFttRawHit with tb= " << hit->tb() << endm;
        // LOG_INFO << "StFttRawHit with map info " << "quadrant = " << (int)hit->quadrant() << " row = " << (int)hit->row() << " strip = " << (int)hit->strip()  << endm;
        // LOG_INFO << "StFttRawHit with map info " << "strip center = " << hit->strip_center() << " strip left edge = " << hit->strip_LEdge() << " strip right edge = " << hit->strip_REdge()  << endm;
        
        // Apply the time cut
        if ( !PassTimeCut( hit ) ) continue;

        if ( kFttHorizontal == so ){
            hStripsPerRob[ rob ].push_back(hit);
            // LOG_INFO << "HORIZONTAL @ ROB = " << (int) rob << endm;
            nStripsHit++;
        }
        if ( kFttVertical   == so ){
            vStripsPerRob[ rob ].push_back(hit);
            // LOG_INFO << "VERTICAL @ ROB = " << (int) rob << endm;
            nStripsHit++;
        }
        if ( kFttDiagonalH   == so ){
            hdStripsPerRob[ rob ].push_back(hit);
            // LOG_INFO << "DIAGONAL @ ROB = " << (int) rob << endm;
            nStripsHit++;
        }
        if ( kFttDiagonalV   == so ){
            vdStripsPerRob[ rob ].push_back(hit);
            // LOG_INFO << "DIAGONAL @ ROB = " << (int) rob << endm;
            nStripsHit++;
        }
    } // loop on hit

size_t nClusters = 0;
LOG_INFO << "StFttClusterMaker::Make{ nStripsHit = " << nStripsHit << " }" << endm;
    if ( nStripsHit > 0 ){ // could make more strict?
        for ( UChar_t iRob = 1; iRob < StFttDb::nRob+1; iRob++ ){
            // LOG_INFO << "ROB=" << (int)iRob << " has " << hStripsPerRob[iRob].size() << " horizontal, "
                // << vStripsPerRob[iRob].size() << " vertical, "
                // << hdStripsPerRob[iRob].size() << " diagonalH, "
                // << vdStripsPerRob[iRob].size() << " diagonalV, "
                // << " strips hit" << endm;

            auto hClusters = FindClusters( hStripsPerRob[iRob] );
            // Add them to StEvent  
            for ( StFttCluster * clu : hClusters ){
                mFttCollection->addCluster( clu );
                nClusters++;
            }
            auto vClusters = FindClusters( vStripsPerRob[iRob] );
            // Add them to StEvent  
            for ( StFttCluster * clu : vClusters ){
                mFttCollection->addCluster( clu );
                nClusters++;
            }
            auto hdClusters = FindClusters( hdStripsPerRob[iRob] );
            // Add them to StEvent  
            for ( StFttCluster * clu : hdClusters ){
                mFttCollection->addCluster( clu );
                nClusters++;
            }
            auto vdClusters = FindClusters( vdStripsPerRob[iRob]);
            // Add them to StEvent  
            for ( StFttCluster * clu : vdClusters ){
                mFttCollection->addCluster( clu );
                nClusters++;
            }
        } // loop on iRob
    } // nStripsHit
    LOG_INFO << "StFttClusterMaker Made " << nClusters << " clusters this event" << endm;

    return kStOk;
} // Make

//dose this function inject test data randomly? 
void StFttClusterMaker::InjectTestData(){
    mFttCollection->rawHits().clear();

    // TODO: inject clean strip hits to test cluster finder
    // TODO: need to using new setMapping fucntion 

    StFttRawHit *hit = new StFttRawHit(  1, 1, 1, 1, 1, 55, 1, 1, 0 );
    hit->setMapping( 1, 1, 1, 23, 86.60, kFttHorizontal ); // LEFT 2
    mFttCollection->addRawHit( hit );

    hit = new StFttRawHit( 1, 1, 1, 1, 1, 90, 1, 1, 0  );
    hit->setMapping( 1, 1, 1, 24, 89.80, kFttHorizontal ); // LEFT 1
    mFttCollection->addRawHit( hit );

    hit = new StFttRawHit( 1, 1, 1, 1, 1, 60, 1, 1, 0  );
    hit->setMapping( 1, 1, 1, 27, 99.40, kFttHorizontal );
    mFttCollection->addRawHit( hit );

    hit = new StFttRawHit( 1, 1, 1, 1, 1, 95, 1, 1, 0 );
    hit->setMapping( 1, 1, 1, 25,93.00, kFttHorizontal ); // CENTER
    mFttCollection->addRawHit( hit );

    hit = new StFttRawHit( 1, 1, 1, 1, 1, 93, 1, 1, 0 );
    hit->setMapping( 1, 1, 1, 26, 96.20, kFttHorizontal );
    mFttCollection->addRawHit( hit );

    

    hit = new StFttRawHit( 1, 1, 1, 1, 1, 19, 1, 1, 0 );
    hit->setMapping( 1, 1, 1, 28, 102.90, kFttHorizontal );
    mFttCollection->addRawHit( hit );


}

bool StFttClusterMaker::PassTimeCut( StFttRawHit * hit ){
    int time_cut0 = -999;
    int time_cut1 =  999;
    int time_cutm = 0;
    //  in principle it could vary VMM to VMM;
    mFttDb->getTimeCut(hit, time_cutm, time_cut0, time_cut1);
    if ( time_cutm == 0 ) // default, cut on bunch crossing
        return (hit->time() <= time_cut1 && hit->time() >= time_cut0); 

    // cut on timebin
    return (hit->tb() <= time_cut1 && hit->tb() >= time_cut0);
}


StFttRawHit * StFttClusterMaker::FindMaxAdc( std::vector<StFttRawHit *> hits, size_t &pos ){
    // Lambda expressions to get the max StFttRawHit with MaxADC
    auto itMax = std::max_element(hits.begin(),
                             hits.end(),
                             [](const StFttRawHit* a,const StFttRawHit* b) { return a->adc() < b->adc(); });

    // LOG_INFO << "FindMaxAdc, pos = " << (pos) << endm; 
    pos = (itMax - hits.begin());
    if ( pos >= hits.size() )
        return nullptr;
    return *itMax;
}

void StFttClusterMaker::SearchClusterEdges( std::vector< StFttRawHit * > hits, 
                                            size_t start, // start index at MaxADC
                                            size_t &left, size_t &right ){
    // set initial values
    left     = start;
    right    = start;

    auto lastHitLeft  = hits[start];
    auto lastHitRight = hits[start];

    bool searchRight = true;
    bool searchLeft = true;

    StFttRawHit *hitLeft = nullptr, *hitRight = nullptr;

    while ( searchRight || searchLeft ){
            LOG_DEBUG << "LEFT: " << left << ", RIGHT: " << right <<  ", start = " << start << ", size=" << hits.size() << endm;
        if ( searchRight ){
            if ( right == hits.size() || right == hits.size() - 1 ){ 
                searchRight = false;
            }
            else {
                
                hitRight = hits[right+1];
                if ( hitRight->adc() > lastHitRight->adc() || hitRight->adc() < GetThresholdFor( hitRight ) ){
                    searchRight = false;
                }
                if ( hitRight->row() != lastHitRight->row() || abs( hitRight->strip() - lastHitRight->strip() ) > 1 ){
                    searchRight = false;
                }

                if ( searchRight ){
                    right ++;
                    lastHitRight = hitRight;
                }
            } // right < size - 1
        } // searchRight

        if ( searchLeft ){
            if ( left == 0 ){
                searchLeft = false;
            } else {
                hitLeft = hits[left-1];
                if ( hitLeft->adc() > lastHitLeft->adc() || hitLeft->adc() < GetThresholdFor( hitLeft ) ){
                    searchLeft = false;
                }
                if ( hitLeft->row() != lastHitLeft->row() || abs( hitLeft->strip() - lastHitLeft->strip() ) > 1 ){
                    searchLeft = false;
                }

                if (searchLeft){
                    left--;
                    lastHitLeft = hitLeft;
                }
            } // left != 0
        } // searchLeft
    } // while searching
} // SearchClusterEdges


void StFttClusterMaker::CalculateClusterInfo( StFttCluster * clu ){

    clu->setNStrips( clu->rawHits().size() );

    // Compute the sumAdc, strip gravity center, and variance
    float m0Sum = 0;
    float m1Sum = 0;
    float m2Sum = 0;

    std::for_each (clu->rawHits().begin(), clu->rawHits().end(), [&](const StFttRawHit *h) {
            // float x = ( h->strip() * 3.2 - 1.6 ); // replace with CONST, old version

        //strip center was added into StFttRawHit
        // cout << "using hit at strip " << (int)h->strip() << " and strip center is " << h->strip_center() << " ADC is " <<  h->adc() << endl;
        float x = ( h->strip_center() ); 
        m0Sum += h->adc();
        m1Sum += (h->adc() * x); 
        m2Sum += ( h->adc() * x * x );
    });

    // m0Sum = sumAdc
    // m1Sum / m0Sum = gravity center (1st moment)
    // m2Sum = accumulated variance (2nd moment)

    clu->setSumAdc( m0Sum );
    clu->setX( m1Sum / m0Sum );
    float var = (m2Sum - m1Sum*m1Sum / m0Sum) / m0Sum;
    clu->setSigma( sqrt( var ) );
    // cout << "cluster plane = " << clu->plane() << " quadrant = " << clu->quadrant()<< " row = " << clu->row() << endl;
    // cout << "cluster dir = " << clu->orientation() << endl;
    // cout << "cluster Sum ADC = " << m0Sum << " X = " << m1Sum / m0Sum << endl;


}



std::vector<StFttCluster*> StFttClusterMaker::FindClusters( std::vector< StFttRawHit * > hits, UChar_t stripOrientattion ){
    std::vector<StFttCluster*> clusters;

    sort(hits.begin(), hits.end(), [](const StFttRawHit * a, const StFttRawHit * b) -> bool { 
        return a->strip() < b->strip(); 
    });

    if(mDebug){
        for ( auto *h : hits ){
        LOG_INFO << *h;
        }
    }
    

    size_t anchor  = hits.size()+1;
    auto maxAdcHit = FindMaxAdc( hits, anchor );

    // loop all the hits until can not find a cluster
    while ( maxAdcHit ){
        StFttCluster * clu = new StFttCluster();

        // Set "location" from max ADC hit
        clu->setPlane( maxAdcHit->plane() );
        clu->setQuadrant( maxAdcHit->quadrant() );
        clu->setOrientation( maxAdcHit->orientation() );
        // Set strip information of max ADC Hit
        // cout << "Max ADC info : strip = " <<  maxAdcHit->strip() << " sc : " << maxAdcHit->strip_center() << " LEdge = " << maxAdcHit->strip_LEdge() << " REdge = " << maxAdcHit->strip_REdge() << endl;
        clu->setIdx_MaxStrip( maxAdcHit->strip() );
        clu->setSC_MaxStrip( maxAdcHit->strip_center() );
        clu->setMaxStripLEdge( maxAdcHit->strip_LEdge() );
        clu->setMaxStripREdge( maxAdcHit->strip_REdge() );


        // Now find the cluster edges
        size_t left = anchor, right = anchor;
        SearchClusterEdges( hits, anchor, left, right);
        LOG_INFO << "Cluster points ( " << left << ", " << anchor << ", " << right << " )" << endm;
        
        // OK now add these hits to the cluster
        for ( size_t i = left; i < right; i++ )
            clu->addRawHit( hits[i] );

        // Compute cluster information from the added hits
        CalculateClusterInfo( clu );

        // LOG_INFO << *clu << endm;
        // only the clusters with at least 1 strips will added
        // May changed follow Yingying's update
        if ( clu->nStrips() > 1)
            clusters.push_back( clu );

        // Now erase all hits from this cluster so that we can move on to find the next one
        hits.erase( hits.begin() + left, hits.begin() + right + 1 );
        maxAdcHit = FindMaxAdc( hits, anchor );
    }


    return clusters;
}

std::vector<StFttCluster*> StFttClusterMaker::FindClusters( std::vector< StFttRawHit * > hits ){
    std::vector<StFttCluster*> clusters;

    /* early data (i.e. cosmic data pre dec 10 2021)
     * had duplicates where the hits are identical except 
     * a different tb. Tonko fixed at some point
     * So this could be wrapped in a run range block, but
     * does no harm.
     */
    const bool dedup = false;
    if ( dedup ){
        auto cmp = [](StFttRawHit* a, StFttRawHit* b) { 
            
            return  a->plane() < b->plane() ||
                    a->quadrant() < b->quadrant() ||
                    a->row() < b->row() ||
                    a->strip() < b->strip() ||
                    a->orientation() < b->orientation(); 
        };
    
        // NOTE according to SO this is faster than using ctor
        set<StFttRawHit*, decltype(cmp)> s(cmp);
        unsigned size = hits.size();
        for( auto h : hits ) s.insert( h );
        hits.assign( s.begin(), s.end() );
    }

    // Sort the hits by row and strip
    // sort(hits.begin(), hits.end(), [](const StFttRawHit * a, const StFttRawHit * b) -> bool { 
    //         size_t indexA = a->orientation() + StFttDb::nStripOrientations * ( a->strip() + a->row() * StFttDb::maxStripPerRow);
    //         size_t indexB = b->orientation() + StFttDb::nStripOrientations * ( b->strip() + b->row() * StFttDb::maxStripPerRow);
    //         return indexA < indexB; 
    //     });

    sort(hits.begin(), hits.end(), [](const StFttRawHit * a, const StFttRawHit * b) -> bool { 
            size_t indexA = ((int)a->orientation()+1)*10000 +  (a->strip()+1) + (a->row()+1)*1000;
            size_t indexB = ((int)b->orientation()+1)*10000 + (b->strip()+1) + (b->row()+1)*1000;
            return indexA < indexB; 
        });

    if ( mDebug ) {
        LOG_INFO << "We have " << hits.size() << " hits after removing duplicates" << endm;
    }


    // Get the max ADC hit in this projection
    size_t anchor  = hits.size()+1;
    auto maxAdcHit = FindMaxAdc( hits, anchor );

    // Loop as long as there is at least 1 hit left
    while ( maxAdcHit ){
        StFttCluster * clu = new StFttCluster();

        if ( Debug() ){
            LOG_DEBUG << "CLUSTER FIND START WITH HITS:" << endm;
            size_t i = 0;
            for ( auto *h : hits ){
                LOG_DEBUG << "[" << i << "]" << *h;
                i++;
            }
        }

        // Set cluster "location" from max ADC hit
        clu->setPlane       ( maxAdcHit->plane       ( ) );
        clu->setQuadrant    ( maxAdcHit->quadrant    ( ) );
        clu->setRow         ( maxAdcHit->row         ( ) );
        clu->setOrientation ( maxAdcHit->orientation ( ) );
        // Set strip information of max ADC Hit
        // cout << "Max ADC info : strip = " <<  maxAdcHit->strip() << " sc : " << maxAdcHit->strip_center() << " LEdge = " << maxAdcHit->strip_LEdge() << " REdge = " << maxAdcHit->strip_REdge() << endl;
        clu->setIdx_MaxStrip( maxAdcHit->strip() );
        clu->setSC_MaxStrip( maxAdcHit->strip_center() );
        clu->setMaxStripLEdge( maxAdcHit->strip_LEdge() );
        clu->setMaxStripREdge( maxAdcHit->strip_REdge() );

        // Now find the cluster edges
        size_t left = anchor, right = anchor;
        SearchClusterEdges( hits, anchor, left, right);
        
        LOG_DEBUG << "Cluster points ( " << left << ", " << anchor << ", " << right << " )" << endm;
        
        
        // OK now add these hits to the cluster
        for ( size_t i = left; i < right + 1; i++ ){
            clu->addRawHit( hits[i] );
        }

        // Compute cluster information from the added hits
        CalculateClusterInfo( clu );

        // if (mDebug){
        //     LOG_INFO << *clu << endm;;
        // }
        clusters.push_back( clu );

        // Now erase all hits from this cluster so that we can move on to find the next one
        hits.erase( hits.begin() + left, hits.begin() + right + 1 );
        maxAdcHit = FindMaxAdc( hits, anchor );
    } // while maxAdcHit
    return clusters;
}



void StFttClusterMaker::ApplyHardwareMap(){
    for ( StFttRawHit* rawHit : mFttCollection->rawHits() ) {
        // cout << "applying the hardware map " << endl;
        mFttDb->hardwareMap( rawHit );
    }
}
