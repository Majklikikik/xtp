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

// Overload of uBLAS prod function with MKL/GSL implementations
#include <votca/tools/linalg.h>

#include "egwbse.h"
#include <votca/xtp/esp2multipole.h>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/numeric/ublas/operation.hpp>
// #include <votca/xtp/logger.h>
#include <votca/xtp/qmpackagefactory.h>
#include <boost/math/constants/constants.hpp>
#include <boost/numeric/ublas/symmetric.hpp>

using boost::format;
using namespace boost::filesystem;

namespace votca {
    namespace xtp {
        namespace ub = boost::numeric::ublas;
        namespace CTP = votca::ctp;
        
        // +++++++++++++++++++++++++++++ //
        // GWBSE MEMBER FUNCTIONS         //
        // +++++++++++++++++++++++++++++ //

        void EGWBSE::CleanUp() {

        }

        void EGWBSE::Initialize(Property *options) {


            // tasks to be done by IBSE: dft_input, dft_run, dft_parse, mgbft, bse_coupling
            _do_dft_input = false;
            _do_dft_run = false;
            _do_dft_parse = false;
            _do_gwbse = false;
            _do_esp=false;

            // update options with the VOTCASHARE defaults   
            UpdateWithDefaults( options, "xtp" );
            ParseOptionsXML(options);

            // register all QM packages (Gaussian, turbomole, etc))
            XQMPackageFactory::RegisterAll();



        }

        void EGWBSE::ParseOptionsXML(Property* options) {



            _maverick = (_nThreads == 1) ? true : false;
            string key = "options." + Identify();
            // job tasks
            string _tasks_string = options->get(key + ".tasks").as<string> ();
            if (_tasks_string.find("input") != std::string::npos) _do_dft_input = true;
            if (_tasks_string.find("dft") != std::string::npos) _do_dft_run = true;
            if (_tasks_string.find("parse") != std::string::npos) _do_dft_parse = true;
            if (_tasks_string.find("gwbse") != std::string::npos) _do_gwbse = true;
            if (_tasks_string.find("esp") != std::string::npos) _do_esp = true;


            key = "options."+Identify();

        if ( options->exists(key+".job_file")) {
            _jobfile = options->get(key+".job_file").as<string>();
        }
        else {
            throw std::runtime_error("Job-file not set. Abort.");
        }

            // options for gwbse
            key = "options." + Identify();
            string _gwbse_xml = options->get(key + ".gwbse_options").as<string> ();
            load_property_from_xml(_gwbse_options, _gwbse_xml.c_str());
           


            // options for dft package
            string _package_xml = options->get(key + ".dftpackage").as<string> ();
            //cout << endl << "... ... Parsing " << _package_xml << endl ;
            load_property_from_xml(_package_options, _package_xml.c_str());
            key = "package";
            _package = _package_options.get(key + ".name").as<string> ();
            
            //options for esp/partialcharges
            if (_do_esp){
                key = "options." + Identify();
                string _esp_xml = options->get(key + ".esp_options").as<string> ();
                load_property_from_xml(_esp_options, _esp_xml.c_str());
            }
            


        }
        
        
        void EGWBSE::WriteJobFile(CTP::Topology *top) {

    cout << endl << "... ... Writing job file: " << flush;
    std::ofstream ofs;
    ofs.open(_jobfile.c_str(), std::ofstream::out);
    if (!ofs.is_open()) throw runtime_error("\nERROR: bad file handle: " + _jobfile);
 
    ofs << "<jobs>" << endl;   

    
    //QMNBList &nblist = top->NBList();    
    
      
    int jobCount = 0;
    
/*

   QMNBList::iterator pit;
    QMNBList &nblist = top->NBList();    
    
            
    int jobCount = 0;
    if (nblist.size() == 0) {
        cout << endl << "... ... No pairs in neighbor list, skip." << flush;
        return;
    } 

    // regenerate the list of bridging segments for every pair 
    // (Donor - Bridge1 - Bridge2 - ... - Acceptor) type
    nblist.GenerateSuperExchange();
    
    map< int,Segment* > segments;
    map< int,Segment* >::iterator sit;

    for (pit = nblist.begin(); pit != nblist.end(); ++pit) {
        
        int id1 = (*pit)->Seg1()->getId();
        int id2 = (*pit)->Seg2()->getId();
	segments[id1] = (*pit)->Seg1();
        segments[id2] = (*pit)->Seg2();
        
       // loop over bridging segments if any and add them to the map
       //    this in principle is not needed since all pairs between 
       //    donors, acceptors, and bridges are already in the list 
        
        vector<Segment*> bridges = (*pit)->getBridgingSegments();
        for ( vector<Segment*>::const_iterator bsit = bridges.begin(); bsit != bridges.end(); bsit++ ) {
            //cout << "Bridging segment " << (*bsit)->getId() << " : " <<  (*bsit)->getName() << endl;
            segments[ (*bsit)->getId() ] = (*bsit);
        }

    }
    

    
    for (sit = segments.begin(); sit != segments.end(); ++sit) {
    
        int id = ++jobCount;
        string tag = "";

        Property Input;
        Property *pInput = &Input.add("input","");
        Property *pSegment =  &pInput->add("segment" , (format("%1$s") % sit->first).str() );
        pSegment->setAttribute<string>("type", sit->second->getName() );
        pSegment->setAttribute<int>("id", sit->second->getId() );
        Job job(id, tag, Input, Job::AVAILABLE );
        job.ToStream(ofs,"xml");
    }
*/
    std::vector<CTP::Segment*> segments=top->Segments();    
    std::vector<CTP::Segment*>::iterator sit;
    for (sit = segments.begin(); sit != segments.end(); ++sit) {
    
        int id = ++jobCount;
        string tag = "";

        Property Input;
        Property *pInput = &Input.add("input","");
        Property *pSegment =  &pInput->add("segment" , (format("%1$s") % (*sit)->getId()).str() );
        pSegment->setAttribute<string>("type", (*sit)->getName() );
        pSegment->setAttribute<int>("id", (*sit)->getId() );
        CTP::Job job(id, tag, Input, CTP::Job::AVAILABLE );
        job.ToStream(ofs,"xml");
    }

    // CLOSE STREAM
    ofs << "</jobs>" << endl;    
    ofs.close();
    
    cout << jobCount << " jobs" << flush;
    
}

        CTP::Job::JobResult EGWBSE::EvalJob(CTP::Topology *top, CTP::Job *job, CTP::QMThread *opThread) {
            

            Orbitals _orbitals;
            CTP::Job::JobResult jres = CTP::Job::JobResult();
            Property _job_input = job->getInput();
            list<Property*> lSegments = _job_input.Select("segment");

            vector < CTP::Segment* > segments;
            int segId = lSegments.front()->getAttribute<int>("id");
            string segType = lSegments.front()->getAttribute<string>("type");

            CTP::Segment *seg = top->getSegment(segId);
            assert(seg->getName() == segType);
            segments.push_back(seg);

            CTP::Logger* pLog = opThread->getLogger();
            
            LOG(CTP::logINFO, *pLog) << CTP::TimeStamp() << " Evaluating site " << seg->getId() << flush;

            
            string output;
            
            // directories and files
            path arg_path;
            string egwbse_work_dir = "OR_FILES";
            
            string frame_dir =  "frame_" + boost::lexical_cast<string>(top->getDatabaseId());     
            string orb_file = (format("%1%_%2%%3%") % "molecule" % segId % ".orb").str();
            
            string _mol_dir = ( format("%1%%2%%3%") % "molecule" % "_" % segId ).str();
            string _package_append = _package + "_gwbse";
            string _qmpackage_work_dir = (arg_path / egwbse_work_dir / _package_append / frame_dir / _mol_dir).c_str();

    
            // get the corresponding object from the QMPackageFactory
            XQMPackage *_qmpackage = XQMPackages().Create(_package);
            // set a log file for the package
            _qmpackage->setLog(pLog);
            // set the run dir 
            _qmpackage->setRunDir(_qmpackage_work_dir);
            // get the package options
            _qmpackage->Initialize(&_package_options);
            
            
            Property _job_summary;
            Property *_output_summary = &_job_summary.add("output", "");
            Property *_segment_summary = &_output_summary->add("segment", "");
            string segName = seg->getName();
            segId = seg->getId();
            _segment_summary->setAttribute("id", segId);
            _segment_summary->setAttribute("type", segName);
            
            
            // different tasks
            
            // create input for DFT
            if ( _do_dft_input ){
                boost::filesystem::create_directories( _qmpackage_work_dir );
                _qmpackage->WriteInputFile( segments );
            }
            
            bool _run_dft_status = false;
            if (_do_dft_run) {
                _run_dft_status = _qmpackage->Run();
                if (!_run_dft_status) {
                    output += "run failed; ";
                    LOG(CTP::logERROR, *pLog) << _qmpackage->getPackageName() << " run failed" << flush;
                    cout << *pLog;
                    jres.setOutput(output);
                    jres.setStatus(CTP::Job::FAILED);
                    delete _qmpackage;
                    return jres;
                }
            }

            // parse the log/orbitals files
            bool _parse_log_status = false;
            bool _parse_orbitals_status = false;
            if (_do_dft_parse) {
                _parse_log_status = _qmpackage->ParseLogFile(&_orbitals);

                if (!_parse_log_status) {
                    output += "log incomplete; ";
                    LOG(CTP::logERROR, *pLog) << "LOG parsing failed" << flush;
                    cout << *pLog;
                    jres.setOutput(output);
                    jres.setStatus(CTP::Job::FAILED);
                    delete _qmpackage;
                    return jres;
                }

                _parse_orbitals_status = _qmpackage->ParseOrbitalsFile(&_orbitals);

                if (!_parse_orbitals_status) {
                    output += "orbfile failed; ";
                    LOG(CTP::logERROR, *pLog) << "Orbitals parsing failed" << flush;
                    cout << *pLog;
                    jres.setOutput(output);
                    jres.setStatus(CTP::Job::FAILED);
                    delete _qmpackage;
                    return jres;
                }  
            } // end of the parse orbitals/log
            else {

                    // load the DFT data from serialized orbitals object
                    string DIR = egwbse_work_dir + "/molecules_gwbse/" + frame_dir;
                    std::ifstream ifs((DIR + "/" + orb_file).c_str());
                    if (_do_gwbse){
                    LOG(CTP::logDEBUG, *pLog) << CTP::TimeStamp() << " Loading DFT data from " << DIR << "/" << orb_file << flush;
                    }
                    else {
                    LOG(CTP::logDEBUG, *pLog) << CTP::TimeStamp() << " Loading data from " << DIR << "/" << orb_file << flush;    
                    }
                    boost::archive::binary_iarchive ia(ifs);
                    ia >> _orbitals;
                    ifs.close();
                }
            
 
            if (_do_gwbse) {
                
              
                
                
                
                //cout << "hallo" <<endl;
                   
               
                GWBSE _gwbse=GWBSE(&_orbitals); 
               // cout << "hallo1" <<endl;
                _gwbse.setLogger(pLog);    
                _gwbse.Initialize(&_gwbse_options);
                // _gwbse.setLogger(pLog);
                //cout << "hallo2" <<endl;
                
                // define own logger for GW-BSE that is written into a runFolder logfile
                CTP::Logger gwbse_logger(CTP::logDEBUG);
                gwbse_logger.setMultithreading(false);
                _gwbse.setLogger(&gwbse_logger);
                gwbse_logger.setPreface(CTP::logINFO,    (format("\nGWBSE INF ...") ).str());
                gwbse_logger.setPreface(CTP::logERROR,   (format("\nGWBSE ERR ...") ).str());
                gwbse_logger.setPreface(CTP::logWARNING, (format("\nGWBSE WAR ...") ).str());
                gwbse_logger.setPreface(CTP::logDEBUG,   (format("\nGWBSE DBG ...") ).str());
                
                
                //bool _evaluate = _gwbse.Evaluate(&_orbitals);
                _gwbse.Evaluate();
                _gwbse.addoutput(_segment_summary);
                // write logger to log file
                std::ofstream ofs;
                string gwbse_logfile = _qmpackage_work_dir + "/gwbse.log";
                ofs.open(gwbse_logfile.c_str(), std::ofstream::out);
                if (!ofs.is_open()) {
                    throw runtime_error("Bad file handle: " + gwbse_logfile);
                }    
                ofs << gwbse_logger << endl;
                ofs.close();
                
                

            }

          
            
            if (_do_esp){
                string mps_file="";
                Esp2multipole esp2multipole=Esp2multipole(pLog);;
                esp2multipole.Initialize(&_esp_options);
                string ESPDIR="MP_FILES/"+frame_dir+"/"+esp2multipole.GetIdentifier();
                esp2multipole.Extractingcharges(_orbitals);
                

                mps_file = (format("%1%_%2%_%3%.mps") % segType % segId % esp2multipole.GetIdentifier() ).str();
                boost::filesystem::create_directories( ESPDIR );
                esp2multipole.WritetoFile((ESPDIR + "/" + mps_file).c_str(),Identify());
    
    
                LOG(CTP::logDEBUG, *pLog) << "Written charges to " << (ESPDIR + "/" + mps_file).c_str() << flush;
                
                _segment_summary->add("partialcharges", (ESPDIR + "/" + mps_file).c_str());
            }
            LOG(CTP::logINFO, *pLog) << CTP::TimeStamp() << " Finished evaluating site " << seg->getId() << flush;

            if(_do_dft_parse ||_do_gwbse ){
            LOG(CTP::logDEBUG, *pLog) << "Saving data to " << orb_file << flush;
            string DIR = egwbse_work_dir + "/molecules_gwbse/" + frame_dir;
            boost::filesystem::create_directories(DIR);  
            std::ofstream ofs((DIR + "/" + orb_file).c_str());
            boost::archive::binary_oarchive oa(ofs);
           

            oa << _orbitals;
            ofs.close();
            }

            // output of the JOB 
            jres.setOutput(_job_summary);
            jres.setStatus(CTP::Job::COMPLETE);

            // dump the LOG

            return jres;
        }

    }


};
