#ifndef _QMPAIR2_H
#define _QMPAIR2_H

#include "segment.h"
#include <utility>


namespace votca { namespace ctp {

class Topology;



class QMPair2 : public std::pair< Segment*, Segment* >
{
public:
    QMPair2() : _R(0,0,0), _ghost(NULL), _top(NULL),
                _id(-1),    _hasGhost(0),
                _rate12_e(0), _rate21_e(0),
                _rate12_h(0), _rate21_h(0),
                _has_e(false), _has_h(false),
                _lambdaO_e(0), _lambdaO_h(0),
                _Jeff2_e(0),   _Jeff2_h(0) { };
    QMPair2(int id, Segment *seg1, Segment *seg2);
   ~QMPair2();


   int       getId() { return _id; }
   Topology *getTopology() { return _top; }
   void      setTopology(Topology *top) { _top = top; }
   vec      &R() { return _R; }
   double    Dist() { return abs(_R); }

   void     setIsPathCarrier(bool yesno, int carrier);
   bool     isPathCarrier(int carrier);

   void     setLambdaO(double lO, int carrier);
   double   getLambdaO(int carrier);

   void     setRate12(double rate, int state);
   void     setRate21(double rate, int state);
   double   getRate12(int state);
   double   getRate21(int state);

   void     setJs(const vector <double> Js, int state);
   double   calcJeff2(int state);
   double   getJeff2(int state) { return (state == -1) ? _Jeff2_e : _Jeff2_h; }
   void     setJeff2(double Jeff2, int state);
   vector<double> &Js(int state) { return (state==-1) ? _Js_e : _Js_h; }

   Segment* Seg1PbCopy() { return first; }
   Segment* Seg2PbCopy();
   Segment* Seg1() { return first; }
   Segment* Seg2() { return second; }

   bool     HasGhost() { return _hasGhost; }
   void     WritePDB(string fileName);



protected:

    int         _id;
    vec         _R;
    Topology   *_top;

    Segment    *_ghost;
    bool        _hasGhost;
    
    double _lambdaO_e;   // from ::EOutersphere output    DEFAULT 0
    double _lambdaO_h;
    double _rate12_e;    // from ::Rates        output    DEFAULT 0
    double _rate12_h;
    double _rate21_e;    // from ::Rates        output    DEFAULT 0
    double _rate21_h;
    double _has_e;       // from ::Rates        input     DEFAULT 0
    double _has_h;
    
    vector <double> _Js_e;
    vector <double> _Js_h;
    double          _Jeff2_e;
    double          _Jeff2_h;




};

}}


#endif
