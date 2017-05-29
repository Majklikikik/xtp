/* 
 *            Copyright 2009-2017 The VOTCA Development Team
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

#ifndef VOTCA_XTP_QMAPECALC_H
#define	VOTCA_XTP_QMAPECALC_H

#include <votca/ctp/pewald3d.h>
#include <votca/ctp/parallelxjobcalc.h>
#include <votca/ctp/xmapper.h>
#include <votca/ctp/xjob.h>
#include <votca/ctp/xinductor.h>
#include <votca/ctp/xinteractor.h>
#include <votca/xtp/gwbse.h>
#include <votca/xtp/qmapemachine.h>
#include <boost/format.hpp>

using boost::format;

namespace votca { namespace xtp {

   
class QMAPE : public ctp::ParallelXJobCalc< vector<ctp::Job*>, ctp::Job*, ctp::Job::JobResult >
{

public:

    QMAPE() {};
   ~QMAPE() {};
   
    string          Identify() { return "qmape"; }
    void            Initialize(Property *);

    void            CustomizeLogger(ctp::QMThread *thread);
    void            PreProcess(ctp::Topology *top);
    ctp::Job::JobResult  EvalJob(ctp::Topology *top, ctp::Job *job, ctp::QMThread *thread);
    ctp::XJob            ProcessInputString(ctp::Job *job, ctp::Topology *top, ctp::QMThread *thread);

private:
    
    // ======================================== //
    // MULTIPOLE ALLOCATION, XJOBS, ADD. OUTPUT //
    // ======================================== //

	string                         _xml_file;
	string                         _mps_table;
	string                         _polar_bg_arch;
	ctp::XMpsMap                   _mps_mapper;
	bool                           _pdb_check;
	bool                           _ptop_check;
    
    // ======================================== //
    // INDUCTION + ENERGY EVALUATION            //
    // ======================================== //

    // Induction, subthreading (-> base class)


    // Multipole Interaction parameters
    string                          _method;
   
    
  
    Property                        _dft_opt;
    
    // GWBSE options
    string                          _gwbse;
    Property                        _gwbse_opt;
    int                             _state;

    // XJob logbook (file output)
    string                          _outFile;
    //bool                            _energies2File;
    
    Property                       *_options;
};

// ========================================================================== //
//                      PARALLELCALC MEMBER FUNCTIONS                         //
// ========================================================================== //


void QMAPE::Initialize(Property *options) {
    
	_options = options;

    cout << endl
         << "... ... Initialized with " << _nThreads << " threads. "
         << flush;

    _maverick = (_nThreads == 1) ? true : false;

    string key = "options."+Identify();

        if ( options->exists(key+".job_file")) {
            _jobfile = options->get(key+".job_file").as<string>();
        }
        else {
            throw std::runtime_error("Job-file not set. Abort.");
        }

	key = "options"+Identify()+".ewald";
		if (options->exists(key+".mapping")) {
			_xml_file = options->get(key+".mapping").as< string >();
		}
		else {
			cout << endl;
			throw std::runtime_error("Multipole mapping file not set. Abort.");
		}
		if ( options->exists(key+".mps_table")) {
			_mps_table = options->get(key+".mps_table").as<string>();
		}
		else {
			cout << endl;
			throw std::runtime_error("Background mps table not set. Abort.");
		}
		if (options->exists(key+".polar_bg")) {
			_polar_bg_arch = options->get(key+".polar_bg").as<string>();
		}
		else { _polar_bg_arch = ""; }
		if (options->exists(key+".pdb_check")) {
			_pdb_check = options->get(key+".pdb_check").as<bool>();
		}
		else { _pdb_check = false; }
		if (options->exists(key+".ptop_check")) {
			_ptop_check = options->get(key+".ptop_check").as<bool>();
		}
		else { _ptop_check = false; }

    
    key = "options."+Identify();
        if ( options->exists(key+".dftoptions")) {
            string package_xml = options->get(key+".dftoptions").as< string >();
            load_property_from_xml(_dft_opt, package_xml.c_str());
        }
        else {
            throw runtime_error("No DFT package specified.");
        }
    

    key = "options."+Identify()+".gwbse";
    if ( options->exists(key)) { 
    	cout << endl << "... ... Configure for excited states (DFT+GWBSE)" << flush;
        if ( options->exists(key+".gwbse_options")) {
            string gwbse_xml = options->get(key+".gwbse_options").as< string >();
            load_property_from_xml(_gwbse_opt, gwbse_xml.c_str());
            // _gwbse = _gwbse_opt.get("package.name").as< string >();
        }
        else {
            throw runtime_error("GWBSE options not specified.");
        }
        _state = options->get(key+".state").as< int >();
    }
    else {
        cout << endl << "... ... Configure for ground states (DFT)" << flush;
    }

}


void QMAPE::PreProcess(ctp::Topology *top) {
    // INITIALIZE MPS-MAPPER (=> POLAR TOP PREP)
    cout << endl << "... ... Initialize MPS-mapper: " << flush;
    _mps_mapper.GenerateMap(_xml_file, _mps_table, top);
    return;
}


void QMAPE::CustomizeLogger(ctp::QMThread *thread) {
    
    // CONFIGURE LOGGER
    ctp::Logger* log = thread->getLogger();
    log->setReportLevel(ctp::logDEBUG);
    log->setMultithreading(_maverick);

    log->setPreface(ctp::logINFO,    (format("\nT%1$02d INF ...") % thread->getId()).str());
    log->setPreface(ctp::logERROR,   (format("\nT%1$02d ERR ...") % thread->getId()).str());
    log->setPreface(ctp::logWARNING, (format("\nT%1$02d WAR ...") % thread->getId()).str());
    log->setPreface(ctp::logDEBUG,   (format("\nT%1$02d DBG ...") % thread->getId()).str());        
}


// ========================================================================== //
//                            QMAPE MEMBER FUNCTIONS                          //
// ========================================================================== //


ctp::XJob QMAPE::ProcessInputString(ctp::Job *job,ctp::Topology *top, ctp::QMThread *thread) {

    // Input string looks like this:
    // <id1>:<name1>:<mpsfile1> <id2>:<name2>: ... ... ...

    string input = job->getInput().as<string>();
    vector<ctp::Segment*> qmSegs;
    vector<string>   qmSegMps;
    vector<string> split;
    Tokenizer toker(input, " \t\n");
    toker.ToVector(split);

    for (unsigned i = 0; i < split.size(); ++i) {

        string id_seg_mps = split[i];
        vector<string> split_id_seg_mps;
        Tokenizer toker(id_seg_mps, ":");
        toker.ToVector(split_id_seg_mps);

        int segId = boost::lexical_cast<int>(split_id_seg_mps[0]);
        string segName = split_id_seg_mps[1];
        string mpsFile = split_id_seg_mps[2];

        ctp::Segment *seg = top->getSegment(segId);
        if (seg->getName() != segName) {
            LOG(ctp::logERROR,*(thread->getLogger()))
                << "ERROR: Seg " << segId << ":" << seg->getName() << " "
                << " maltagged as " << segName << ". Skip job ..." << flush;
            throw std::runtime_error("Input does not match topology.");
        }

        qmSegs.push_back(seg);
        qmSegMps.push_back(mpsFile);
    }

    return ctp::XJob(job->getId(), job->getTag(), qmSegs, qmSegMps, top);
}


ctp::Job::JobResult QMAPE::EvalJob(ctp::Topology *top, ctp::Job *job, ctp::QMThread *thread) {
    
    // SILENT LOGGER FOR QMPACKAGE
    ctp::Logger* log = thread->getLogger();    
    ctp::Logger* qlog = new ctp::Logger();
    qlog->setReportLevel(ctp::logDEBUG);
    qlog->setMultithreading(_maverick);
    qlog->setPreface(ctp::logINFO,    (format("\nQ%1$02d ... ...") % thread->getId()).str());
    qlog->setPreface(ctp::logERROR,   (format("\nQ%1$02d ERR ...") % thread->getId()).str());
    qlog->setPreface(ctp::logWARNING, (format("\nQ%1$02d WAR ...") % thread->getId()).str());
    qlog->setPreface(ctp::logDEBUG,   (format("\nQ%1$02d DBG ...") % thread->getId()).str());

    // CREATE XJOB FROM JOB INPUT STRING
    LOG(ctp::logINFO,*log)
        << "Job input = " << job->getInput().as<string>() << flush;
    ctp::XJob xjob = this->ProcessInputString(job, top, thread);  

	// SETUP POLAR TOPOLOGY (GENERATE VS LOAD IF PREPOLARIZED)
	if (_polar_bg_arch == "") {
		LOG(ctp::logINFO,*log) << "Mps-Mapper: Generate FGC FGN BGN" << flush;
		_mps_mapper.Gen_FGC_FGN_BGN(top, &xjob, thread);
	}
	else {
		LOG(ctp::logINFO,*log) << "Mps-Mapper: Generate FGC, load FGN BGN from '"
				<< _polar_bg_arch << "'" << flush;
		_mps_mapper.Gen_FGC_Load_FGN_BGN(top, &xjob, _polar_bg_arch, thread);
	}
    LOG(ctp::logINFO,*log) << xjob.getPolarTop()->ShellInfoStr() << flush;

    // SETUP MM METHOD
    ctp::PEwald3D3D cape = ctp::PEwald3D3D(top, xjob.getPolarTop(), _options,
		thread->getLogger());
	if (_pdb_check)
		cape.WriteDensitiesPDB(xjob.getTag()+".densities.pdb");

   
    
    // SETUP QMAPE
    QMAPEMachine machine = QMAPEMachine(&xjob, &cape, _options, "options.qmape", _subthreads);
    machine.setLog(thread->getLogger());
    
    // EVALUATE: ITERATE UNTIL CONVERGED
    machine.Evaluate(&xjob);

    // GENERATE OUTPUT AND FORWARD TO PROGRESS OBSERVER (RETURN)
    ctp::Job::JobResult jres = ctp::Job::JobResult();
    jres.setOutput(xjob.getInfoLine());
    jres.setStatus(ctp::Job::COMPLETE);
    
 
    delete qlog;

    return jres;
}



    
}}

#endif /* __QMAPE__H */
