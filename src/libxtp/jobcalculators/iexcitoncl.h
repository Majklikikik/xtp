/*
 *            Copyright 2009-2016 The VOTCA Development Team
 *                       (http://www.votca.org)
 *
 *      Licensed under the Apache License, Version 2.0 (the "License")
 *
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _CALC_COUPLING_EXCL_H
#define	_CALC_COUPLING_EXCL_H

#include <votca/tools/property.h>

#include <votca/ctp/parallelxjobcalc.h>
#include <boost/numeric/ublas/io.hpp>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include <votca/ctp/xmapper.h>
#include <votca/ctp/xjob.h>

namespace votca { namespace xtp {
    
/**
* \brief Evaluates Transition Charge distributions classically
*
* Evaluates the electrostatic classical coupling between molecules in 
* their excited states.
* 

* 
* Callname: iexcitoncl
*/

class IEXCITON : public ctp::ParallelXJobCalc< vector<ctp::Job*>, ctp::Job*, ctp::Job::JobResult >
{
public:

    IEXCITON() {};
   ~IEXCITON() {};
   
    void    Initialize(tools::Property *options );
    
    string  Identify() { return "iexcitoncl"; }
    
    ctp::Job::JobResult EvalJob(ctp::Topology *top, ctp::Job *job, ctp::QMThread *Thread);

    void WriteJobFile(ctp::Topology *top);
    void ReadJobFile(ctp::Topology *top);

    

private:

    
    
   
    ctp::XMpsMap                        _mps_mapper;
    bool                           _induce;
    int                           _statenumber;
    string                         _emp_file;
    string                         _xml_file;

        

        
    
    void PreProcess(ctp::Topology *top);
    void CustomizeLogger(ctp::QMThread *thread);
    double EvaluatePair(ctp::Topology *top,ctp::PolarSeg* Seg1,ctp::PolarSeg* Seg2, ctp::Logger* pLog);
 
    
 
        
};

}}
#endif	/* _CALC_INTEGRALS_DFT_H */
