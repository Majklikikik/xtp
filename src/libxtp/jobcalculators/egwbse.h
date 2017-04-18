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

#ifndef _CALC_GWBSE_TOOL_H
#define	_CALC_GWBSE_TOOL_H


#include <votca/xtp/gwbse.h> // including GWBSE functionality
#include <votca/xtp/orbitals.h>
#include <votca/xtp/aomatrix.h>
#include <votca/xtp/threecenters.h>
#include <votca/xtp/qmpackagefactory.h>

#include <votca/ctp/parallelxjobcalc.h>
#include <votca/ctp/segment.h>

#include <votca/tools/linalg.h>



namespace votca { namespace xtp {
    namespace ub = boost::numeric::ublas;
/**
* \brief GWBSE implementation
*
* Requires a first-principles package, i.e. GAUSSIAN installation
*
* Callname: egwbse
*/

class EGWBSE : public ctp::ParallelXJobCalc< vector< ctp::Job*>,  ctp::Job*,  ctp::Job::JobResult >
{
public:

    EGWBSE() {};
   ~EGWBSE() {};

    string  Identify() { return "egwbse"; }
    void    Initialize( Property *options);
     ctp::Job::JobResult EvalJob( ctp::Topology *top,  ctp::Job *job,  ctp::QMThread *thread);
    
    string              _package;
    Property            _package_options; 
    Property            _gwbse_options; 
    Property            _esp_options;
    
    void    CleanUp();
    void WriteJobFile( ctp::Topology *top);

    
    // what to do
    bool                _do_dft_input;
    bool                _do_dft_run;
    bool                _do_dft_parse;
    bool                _do_gwbse;
    bool                _do_esp;

    
    // all GWBSE functionality is in GWBSE object
    //GWBSE _gwbse; 
    
    
    void ParseOptionsXML( Property *options);    

};


}}

#endif	/* _CALC_GWBSE_TOOL_H */
