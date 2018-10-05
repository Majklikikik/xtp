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

#ifndef VOTCA_XTP_QMMACHINE_H
#define	VOTCA_XTP_QMMACHINE_H

#include <votca/xtp/xjob.h>
#include <votca/xtp/xinductor.h>
#include <votca/xtp/gwbse.h>
#include <votca/xtp/qmpackagefactory.h>
#include <votca/xtp/orbitals.h>
#include <votca/xtp/espfit.h>
#include <votca/xtp/gdma.h>
#include <votca/xtp/qminterface.h>
#include <votca/xtp/qmiter.h>
#include <votca/xtp/statefilter.h>

namespace votca { namespace xtp {



class QMMachine{

public:

    QMMachine(xtp::XJob *job, xtp::XInductor *xind, QMPackage *qmpack,
              Property *opt, string sfx);
   ~QMMachine();

    int Evaluate(xtp::XJob *job);

    void setLog(xtp::Logger *log) { _log = log; }

private:
    bool Iterate(string jobFolder, int iterCnt);
    bool RunDFT(string& runFolder, std::vector<std::shared_ptr<xtp::PolarSeg> >& MultipolesBackground);
    void RunGWBSE(string& runFolder);
    void RunGDMA(QMMIter* thisIter, string& runFolder);
    void Density2Charges(const QMState& state);
    
    QMMIter *CreateNewIter();
    bool hasConverged();
    xtp::XJob *_job;
    xtp::XInductor *_xind;
    QMPackage *_qmpack;
    xtp::Logger *_log;

    std::vector<QMMIter*> _iters;
    bool _isConverged;
    int _maxIter;

    Property _gdma_options;
    bool _do_gdma;

    Property _gwbse_options;
    QMState  _initialstate;

    Statefilter _filter;
    
    double _crit_dR;
    double _crit_dQ;
    double _crit_dE_QM;
    double _crit_dE_MM;

    bool _do_gwbse=false; 
    bool _do_archive;
    bool _static_qmmm;
    Orbitals orb_iter_input;
    
    double _alpha;
    Eigen::MatrixXd _DMAT_old;

};


}}

#endif //  VOTCA_XTP_QMMACHINE_H

