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

#ifndef _VOTCA_XTP_GWBSEENGINE_H
#define _VOTCA_XTP_GWBSEENGINE_H



#include <votca/ctp/segment.h>
#include <votca/xtp/orbitals.h>
#include <votca/ctp/polarseg.h>
#include <votca/xtp/qmpackage.h>
#include <votca/ctp/topology.h>
#include <votca/ctp/apolarsite.h>
#include <boost/filesystem.hpp>
#include <votca/ctp/logger.h>

namespace votca {
    namespace xtp {
        namespace ub = boost::numeric::ublas;

/**
         * \brief Electronic Excitations via Density-Functional Theory
         *
         * Evaluates electronic ground state in molecular systems based on
         * density functional theory with Gaussian Orbitals.
         * 
         */

        class GWBSEENGINE {
        public:

            GWBSEENGINE() {
            };

            ~GWBSEENGINE() {
            };

            std::string Identify() {
                return "gwbse_engine";
            }

            void Initialize(Property *options, string _archive_filename);
            void ExcitationEnergies(QMPackage* _qmpackage, vector<ctp::Segment*> _segments, Orbitals* _orbitals);

            void setLog(ctp::Logger* pLog) {
                _pLog = pLog;
            }

            string GetDFTLog() {
                return _dftlog_file;
            };

            void setLoggerFile(string logger_file) {
                _logger_file = logger_file;
            };

            void setRedirectLogger(bool redirect_logger) {
                _redirect_logger = redirect_logger;
            };

        private:

            ctp::Logger *_pLog;

            // task options
            bool _do_guess;
            bool _do_dft_input;
            bool _do_dft_run;
            bool _do_dft_parse;
            bool _do_gwbse;
            bool _redirect_logger;

            // DFT log and MO file names
            string _MO_file; // file containing the MOs from qmpackage...
            string _dftlog_file; // file containing the Energies etc... from qmpackage...
            string _logger_file;
            string _archive_file;
            string _guess_archiveA;
            string _guess_archiveB;

            // Options for GWBSE module
            Property _gwbse_options;

            void SaveRedirectedLogger(ctp::Logger* pLog);



        };


    }
}

#endif /* _VOTCA_XTP_GWBSEENGINE_H */
