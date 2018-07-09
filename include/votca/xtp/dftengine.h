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

#ifndef _VOTCA_XTP_DFTENGINE_H
#define _VOTCA_XTP_DFTENGINE_H



#include <votca/ctp/segment.h>
#include <votca/xtp/orbitals.h>
#include <votca/ctp/polarseg.h>

#include <votca/ctp/topology.h>
#include <votca/xtp/numerical_integrations.h>
#include <votca/ctp/apolarsite.h>
#include <boost/filesystem.hpp>
#include <votca/xtp/ERIs.h>
#include <votca/xtp/convergenceacc.h>
#include <votca/ctp/logger.h>

namespace votca {
    namespace xtp {


/**
         * \brief Electronic ground-state via Density-Functional Theory
         *
         * Evaluates electronic ground state in molecular systems based on
         * density functional theory with Gaussian Orbitals.
         * 
         */

        class DFTENGINE {
        public:

            DFTENGINE() {
                _addexternalsites = false;
                _do_externalfield = false;
                guess_set = false;
            };

            ~DFTENGINE() {

            };



            void Initialize(tools::Property *options);

            std::string Identify() {
                return "dftengine";
            }

            void CleanUp();

            void setLogger(ctp::Logger* pLog) {
                _pLog = pLog;
            }

            void ConfigureExternalGrid(std::string grid_name_ext) {
                _grid_name_ext = grid_name_ext;
                _do_externalfield = true;
            }

            void setExternalcharges(std::vector<ctp::PolarSeg*> externalsites) {
                

                _externalsites = externalsites;
                //for ( int i = 0; i < externalsites.size(); i++){
                //    ctp::PolarSeg* pseg = &externalsites[i];
                //    _externalsites.push_back(ctp::PolarSeg(pseg, false));
                //}
                _addexternalsites = true;
            }
            
           /* void SetExternalMultipoles( ctp::PolarTop *ptop  ){
                
                
                return;
                
            }*/
            
            

            void setExternalGrid(std::vector<double> electrongrid, std::vector<double> nucleigrid) {
                _externalgrid = electrongrid;
                _externalgrid_nuc = nucleigrid;
            }

            std::vector< const tools::vec *> getExternalGridpoints() {
                return _gridIntegration_ext.getGridpoints();
            }


            bool Evaluate(Orbitals* _orbitals);
            

            void Prepare(Orbitals* _orbitals);

            std::string GetDFTBasisName() {
                return _dftbasis_name;
            };
            
            void CalculateERIs(const AOBasis& dftbasis, const Eigen::MatrixXd &DMAT);

        private:
            void PrintMOs(const Eigen::VectorXd& MOEnergies);
            ctp::Logger *_pLog;

            void ConfigOrbfile(Orbitals* _orbitals);
            void SetupInvariantMatrices();
            Eigen::MatrixXd AtomicGuess(Orbitals* _orbitals);
            Eigen::MatrixXd DensityMatrix_unres(const Eigen::MatrixXd& MOs, int numofelec);
            Eigen::MatrixXd DensityMatrix_frac(const Eigen::MatrixXd& MOs, const Eigen::VectorXd& MOEnergies, int numofelec);
            std::string Choosesmallgrid(std::string largegrid);
            void NuclearRepulsion();
            double ExternalRepulsion(ctp::Topology* top = NULL);
            double ExternalGridRepulsion(std::vector<double> externalpotential_nuc);
            Eigen::MatrixXd AverageShells(const Eigen::MatrixXd& dmat, AOBasis& dftbasis);



            int _openmp_threads;

            // options
            std::string _dft_options;
            tools::Property _dftengine_options;

            // atoms
            std::vector<QMAtom*> _atoms;

            // basis sets
            std::string _auxbasis_name;
            std::string _dftbasis_name;
            std::string _ecp_name;
            BasisSet _dftbasisset;
            BasisSet _auxbasisset;
            BasisSet _ecpbasisset;
            AOBasis _dftbasis;
            AOBasis _auxbasis;
            AOBasis _ecp;

            bool _with_ecp;
            bool _with_RI;
            
            std::string _four_center_method; // direct | cache
            
            // Pre-screening
            bool _with_screening;
            double _screening_eps;
            
            // numerical integration Vxc
            std::string _grid_name;
            std::string _grid_name_small;
            bool _use_small_grid;
            NumericalIntegration _gridIntegration;
            NumericalIntegration _gridIntegration_small;
            //used to store Vxc after final iteration

            //numerical integration externalfield;
            //this will not remain here but be moved to qmape
            bool _do_externalfield;
            std::string _grid_name_ext;
            NumericalIntegration _gridIntegration_ext;
            std::vector<double> _externalgrid;
            std::vector<double> _externalgrid_nuc;

            Eigen::MatrixXd _dftAOdmat;

            // AO Matrices
            AOOverlap _dftAOoverlap;
            AOKinetic _dftAOkinetic;
            AOESP _dftAOESP;
            AOECP _dftAOECP;
            AODipole_Potential _dftAODipole_Potential;
            AOQuadrupole_Potential _dftAOQuadrupole_Potential;
            AOPlanewave _dftAOplanewave;
            bool _with_guess;
            std::string _initial_guess;
            double E_nucnuc;

            // COnvergence 
            double _mixingparameter;
            double _Econverged;
            double _error_converged;
            int _numofelectrons;
            int _max_iter;
            

            //levelshift

            double _levelshiftend;
            double _levelshift;


            //DIIS variables
            ConvergenceAcc conv_accelerator;
            bool _usediis;
            unsigned _histlength;
            bool _maxout;
            Eigen::MatrixXd _Sminusonehalf;
            double _diis_start;
            double _adiis_start;
            bool _useautomaticmixing;
            //Electron repulsion integrals
            ERIs _ERIs;

            // external charges
            std::vector<ctp::PolarSeg*> _externalsites;
            bool _addexternalsites;

            // exchange and correlation
            double _ScaHFX;
            std::string _xc_functional_name;


            Eigen::MatrixXd last_dmat;
            bool guess_set;
        };


    }
}

#endif /* _VOTCA_XTP_DFTENGINE_H */
