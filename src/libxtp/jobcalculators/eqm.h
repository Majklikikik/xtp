/*
 *            Copyright 2009-2018 The VOTCA Development Team
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

#ifndef _CALC_XTP_EQM_H
#define _CALC_XTP_EQM_H

#include <votca/xtp/gwbse.h> // including GWBSE functionality
#include <votca/xtp/qmpackagefactory.h>
#include <votca/xtp/parallelxjobcalc.h>
#include <votca/xtp/segment.h>

namespace votca {
    namespace xtp {

        /**
         * \brief GWBSE implementation
         *
         * Evaluates DFT and GWBSE for all molecules
         * Requires a first-principles package, i.e. GAUSSIAN, ORCA, NWChem
         *
         * Callname: eqm
         */

        class EQM : public xtp::ParallelXJobCalc< vector< xtp::Job*>, xtp::Job*, xtp::Job::JobResult > {
        public:
            void WriteLoggerToFile(const std::string& logfile, xtp::Logger& logger);
            std::string Identify() {
                return "eqm";
            }
            void Initialize(Property *options);
            xtp::Job::JobResult EvalJob(xtp::Topology *top, xtp::Job *job, xtp::QMThread *thread);
            
            void CleanUp() {;}
            void WriteJobFile(xtp::Topology *top);
        private:
            
           void SetJobToFailed(xtp::Job::JobResult& jres, xtp::Logger* pLog, const string& errormessage);
            void ParseOptionsXML(Property *options);

            std::string _package;
            Property _package_options;
            Property _gwbse_options;
            Property _esp_options;

            // what to do
            bool _do_dft_input;
            bool _do_dft_run;
            bool _do_dft_parse;
            bool _do_gwbse;
            bool _do_esp;

        };


    }
}

#endif /* _CALC_GWBSE_TOOL_H */
