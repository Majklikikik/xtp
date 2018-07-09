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

#include "votca/xtp/orbitals.h"
#include <votca/xtp/version.h>
#include <votca/tools/elements.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <numeric>



using namespace votca::tools;

namespace votca {
    namespace xtp {

        Orbitals::Orbitals() {

            _basis_set_size = 0;
            _occupied_levels = 0;
            _unoccupied_levels = 0;
            _number_of_electrons = 0;
            _self_energy = 0.0;
            _qm_energy = 0.0;
            _couplingsA = 0;
            _couplingsB = 0;
            _ECP = "";
            _bsetype = "";
            //_has_atoms = false;


            // GW-BSE
            _qpmin = 0;
            _qpmax = 0;
            _qptotal = 0;

            _rpamin = 0;
            _rpamax = 0;

            _ScaHFX = 0.0;

            _bse_cmin = 0;
            _bse_cmax = 0;
            _bse_vmin = 0;
            _bse_vmax = 0;
            _bse_vtotal = 0;
            _bse_ctotal = 0;
            _bse_size = 0;
            _bse_nmax = 0;

        };

        Orbitals::~Orbitals() {
            std::vector< QMAtom* >::iterator it;
            for (it = _atoms.begin(); it != _atoms.end(); ++it) delete *it;

        };

        void Orbitals::setNumberOfLevels(const int &occupied_levels, const int &unoccupied_levels) {
            _occupied_levels = occupied_levels;
            _unoccupied_levels = unoccupied_levels;
        }

        /**
         *
         * @param _energy_difference [ev] Two levels are degenerate if their energy is smaller than this value
         * @return A map with key as a level and a vector which is a list of close lying orbitals
         */
        bool Orbitals::CheckDegeneracy(double _energy_difference) {

            if (tools::globals::verbose) {
                cout << endl << "... ... Checking level degeneracy " << endl;
            }
            bool _degenerate = false;
            _level_degeneracy.clear();
            for (unsigned i = 0; i < _mo_energies.size(); ++i) {
                // add the level itself - it is easier to loo over all levels later
                // in all containers counters start with 0; real life - with 1
                _level_degeneracy[i + 1].push_back(i + 1);
                for (unsigned j = i + 1; j < _mo_energies.size(); ++j) {
                    if (std::abs(_mo_energies(i) - _mo_energies(j)) * tools::conv::hrt2ev < _energy_difference) {
                        _level_degeneracy[i + 1].push_back(j + 1);
                        _level_degeneracy[j + 1].push_back(i + 1);
                        _degenerate = true;
                    }
                }
            }
            if (tools::globals::verbose) {

                if (_degenerate) {
                    cout << "... ... Some levels are degenerate" << endl;
                    for (std::map<int, std::vector<int> >::iterator it = _level_degeneracy.begin();
                            it != _level_degeneracy.end();
                            ++it) {
                        // output only degenerate levels
                        if ((it->second).size() > 1) {
                            std::cout << "... ... level  " << it->first << " : ";
                            for (vector<int>::iterator lev = (it->second).begin(); lev != (it->second).end(); lev++)
                                cout << *lev << " ";
                            cout << endl;
                        }
                    }
                } else {
                    cout << "... ... No degeneracy found" << endl;
                }
                cout << "... ... Done checking level degeneracy" << endl;
            }
            return _degenerate;
        }

        std::vector<int>* Orbitals::getDegeneracy(int level, double _energy_difference) {
            if (!hasDegeneracy()) {
                CheckDegeneracy(_energy_difference);
            }
            return &_level_degeneracy.at(level);
        }

        std::vector<int> Orbitals::SortEnergies() {
            if (tools::globals::verbose) cout << "... ... Sorting energies" << endl;
            std::vector<int>index = std::vector<int>(_mo_energies.size());
            std::iota(index.begin(), index.end(), 0);
            std::stable_sort(index.begin(), index.end(), [this](int i1, int i2) {
                return this->MOEnergies()[i1] > this->MOEnergies()[i2];
            });
            return index;
        }

        /// Writes a PDB file
        //TODO move to filewriter PDB

        void Orbitals::WritePDB(FILE *out, string tag) {
            string tag_extended = "HEADER ! GENERATED BY VOTCA::XTP::" + tag + "\n";
            fprintf(out, "%s", tag_extended.c_str());
            vector < QMAtom* > ::iterator atom;
            int id = 0;


            for (atom = _atoms.begin(); atom < _atoms.end(); ++atom) {
                id++;
                string resname = "QM";
                int resnr = 1;
                const tools::vec pos = (*atom)->getPos() * tools::conv::bohr2ang;
                fprintf(out, "ATOM  %5d %4s%1s%3s %1s%4d%1s   %8.3f%8.3f%8.3f%6.2f%6.2f      %4s%2s  %8.3f\n",
                        id, // Atom serial number           %5d
                        (*atom)->getType().c_str(), // Atom name                    %4s
                        " ", // alternate location indicator.%1s
                        resname.c_str(), // Residue name.                %3s
                        "A", // Chain identifier             %1s
                        resnr, // Residue sequence number      %4d
                        " ", // Insertion of residues.       %1s
                        pos.getX(), // X in Angstroms               %8.3f
                        pos.getY(), // Y in Angstroms               %8.3f
                        pos.getZ(), // Z in Angstroms               %8.3f
                        1.0, // Occupancy                    %6.2f
                        0.0, // Temperature factor           %6.2f
                        " ", // Segment identifier           %4s
                        (*atom)->getType().c_str(), // Element symbol               %2s
                        (*atom)->getPartialcharge() // Charge on the atom.          %2s
                        );
            }
            return;
        }

        // reduces the number of virtual orbitals to (factor-1)*number_of_occupied_orbitals

        void Orbitals::Trim(int factor) {

            if (hasMOCoefficients()) {
                _mo_coefficients.conservativeResize(_basis_set_size,factor * _occupied_levels);
                _unoccupied_levels = (factor - 1) * _occupied_levels;
            }

            if (hasMOEnergies()) {
                _mo_energies.conservativeResize(factor * _occupied_levels);
                _unoccupied_levels = (factor - 1) * _occupied_levels;
            }
            return;
        }


        // reduces the number of orbitals to [HOMO-degH:LUMO+degL]

        void Orbitals::Trim(int degH, int degL) {

            if (hasMOCoefficients()) {
                _mo_coefficients = _mo_coefficients.block(0,_occupied_levels - degH,_basis_set_size, degL + degH).eval();
            }
            if (hasMOEnergies()) {
                _mo_energies = _mo_energies.segment(_occupied_levels - degH, degL + degH).eval();
            }
            _occupied_levels = degH;
            _unoccupied_levels = degL;

            return;
        }


        // Determine ground state density matrix

        Eigen::MatrixXd Orbitals::DensityMatrixGroundState() {

            Eigen::MatrixXd occstates = _mo_coefficients.block(0, 0, _mo_coefficients.rows(), _occupied_levels);
            Eigen::MatrixXd dmatGS = 2.0 * occstates * occstates.transpose();
            return dmatGS;
        }

        Eigen::MatrixXd Orbitals::LambdaMatrixQuasiParticle() {
            return _QPdiag_coefficients * _mo_coefficients.block(0, _qpmin, _mo_coefficients.rows(), _qptotal);
        }

        // Determine QuasiParticle Density Matrix

        Eigen::MatrixXd Orbitals::DensityMatrixQuasiParticle(int state) {
            Eigen::MatrixXd lambda = LambdaMatrixQuasiParticle();
            Eigen::MatrixXd dmatQP = lambda.col(state) * lambda.col(state).transpose();
            return dmatQP;
        }

        Eigen::MatrixXd Orbitals::TransitionDensityMatrix(const string& spin, int state) {
            if (!(spin == "singlet" || spin == "triplet")) {
                throw runtime_error("Spin type not known for density matrix. Available are singlet and triplet");
            }
            MatrixXfd& _BSECoefs = (spin == "singlet") ? _BSE_singlet_coefficients : _BSE_triplet_coefficients;
            if(_BSECoefs.cols()<state || _BSECoefs.rows()<2){
                throw runtime_error("Orbitals object has no information about that state");
            }
            Eigen::MatrixXd dmatTS = Eigen::MatrixXd::Zero(_basis_set_size, _basis_set_size);
            // The Transition dipole is sqrt2 bigger because of the spin, the excited state is a linear combination of 2 slater determinants, where either alpha or beta spin electron is excited
            double sqrt2 = sqrt(2.0);
            /*Trying to implement D_{alpha,beta}= sqrt2*sum_{i}^{occ}sum_{j}^{virt}{BSEcoef(i,j)*MOcoef(alpha,i)*MOcoef(beta,j)} */
            // c stands for conduction band and thus virtual orbitals
            // v stand for valence band and thus occupied orbitals
            std::vector<int> _index2v;
            std::vector<int> _index2c;

            // indexing info BSE vector index to occupied/virtual orbital
            for (unsigned _v = 0; _v < _bse_vtotal; _v++) {
                for (unsigned _c = 0; _c < _bse_ctotal; _c++) {
                    _index2v.push_back(_bse_vmin + _v);
                    _index2c.push_back(_bse_cmin + _c);
                }
            }

            if (_bsetype == "full" && spin == "singlet") {
                const MatrixXfd& _BSECoefs_AR = _BSE_singlet_coefficients_AR;
#pragma omp parallel for
                for (unsigned a = 0; a < dmatTS.rows(); a++) {
                    for (unsigned b = 0; b < dmatTS.cols(); b++) {
                        for (unsigned i = 0; i < _bse_size; i++) {
                            int occ = _index2v[i];
                            int virt = _index2c[i];
                            dmatTS(a, b) += sqrt2 * (_BSECoefs(i, state) + _BSECoefs_AR(i, state)) * _mo_coefficients(a, occ) * _mo_coefficients(b, virt); //check factor 2??
                        }
                    }
                }
            } else {

#pragma omp parallel for
                for (unsigned a = 0; a < dmatTS.rows(); a++) {
                    for (unsigned b = 0; b < dmatTS.cols(); b++) {
                        for (unsigned i = 0; i < _bse_size; i++) {
                            int occ = _index2v[i];
                            int virt = _index2c[i];
                            dmatTS(a, b) += sqrt2 * _BSECoefs(i, state) * _mo_coefficients(a, occ) * _mo_coefficients(b, virt); //check factor 2??
                        }
                    }
                }

            }
            return dmatTS;
        }

        std::vector<Eigen::MatrixXd > Orbitals::DensityMatrixExcitedState(const string& spin, int state) {
            std::vector<Eigen::MatrixXd > dmat = DensityMatrixExcitedState_R(spin, state);
            if (_bsetype == "full" && spin == "singlet") {
                std::vector<Eigen::MatrixXd > dmat_AR = DensityMatrixExcitedState_AR(spin, state);
                dmat[0] -= dmat_AR[0];
                dmat[1] -= dmat_AR[1];
            }
            return dmat;
        }

        // Excited state density matrix

        std::vector<Eigen::MatrixXd> Orbitals::DensityMatrixExcitedState_R(const string& spin, int state) {
            if (!(spin == "singlet" || spin == "triplet")) {
                throw runtime_error("Spin type not known for density matrix. Available are singlet and triplet");
            }

            MatrixXfd & _BSECoefs = (spin == "singlet") ? _BSE_singlet_coefficients : _BSE_triplet_coefficients;
            if(_BSECoefs.cols()<state || _BSECoefs.rows()<2){
                throw runtime_error("Orbitals object has no information about that state");
            }
            /******
             *
             *    Density matrix for GW-BSE based excitations
             *
             *    - electron contribution
             *      D_ab = \sum{vc} \sum{c'} A_{vc}A_{vc'} mo_a(c)mo_b(c')
             *
             *    - hole contribution
             *      D_ab = \sum{vc} \sum{v'} A_{vc}A_{v'c} mo_a(v)mo_b(v')
             *
             *
             *   more efficient:
             *
             *   - electron contribution
             *      D_ab = \sum{c} \sum{c'} mo_a(c)mo_b(c') [ \sum{v} A_{vc}A_{vc'} ]
             *           = \sum{c} \sum{c'} mo_a(c)mo_b(c') A_{cc'}
             *
             *   - hole contribution
             *      D_ab = \sum{v} \sum{v'} mo_a(v)mo_b(v') [ \sum{c} A_{vc}A_{v'c} ]
             *           = \sum{v} \sum{v'} mo_a(v)mo_b(v') A_{vv'}
             *
             */
            std::vector<Eigen::MatrixXd > dmatEX;
            dmatEX.resize(2);
            dmatEX[0] = Eigen::MatrixXd::Zero(_basis_set_size, _basis_set_size);
            dmatEX[1] = Eigen::MatrixXd::Zero(_basis_set_size, _basis_set_size);

            std::vector<int> _index2v;
            std::vector<int> _index2c;
            // indexing info BSE vector index to occupied/virtual orbital
            for (unsigned _v = 0; _v < _bse_vtotal; _v++) {
                for (unsigned _c = 0; _c < _bse_ctotal; _c++) {
                    _index2v.push_back(_bse_vmin + _v);
                    _index2c.push_back(_bse_cmin + _c);
                }
            }

            // electron assist matrix A_{cc'}
            Eigen::MatrixXd _Acc = Eigen::MatrixXd::Zero(_bse_ctotal, _bse_ctotal);
            Eigen::MatrixXd _Avv = Eigen::MatrixXd::Zero(_bse_vtotal, _bse_vtotal);

            for (unsigned _idx1 = 0; _idx1 < _bse_size; _idx1++) {
                int _v = _index2v[_idx1];
                int _c = _index2c[_idx1];
                // electron assist matrix A_{cc'}
#pragma omp parallel for
                for (unsigned _c2 = _bse_cmin; _c2 <= _bse_cmax; _c2++) {
                    unsigned _idx2 = (_bse_cmax - _bse_cmin + 1)*(_v - _bse_vmin)+(_c2 - _bse_cmin);
                    _Acc(_c - _bse_cmin, _c2 - _bse_cmin) += _BSECoefs(_idx1, state) * _BSECoefs(_idx2, state);
                }

                // hole assist matrix A_{vv'}
#pragma omp parallel for
                for (unsigned _v2 = _bse_vmin; _v2 <= _bse_vmax; _v2++) {
                    unsigned _idx2 = (_bse_cmax - _bse_cmin + 1)*(_v2 - _bse_vmin)+(_c - _bse_cmin);
                    _Avv(_v - _bse_vmin, _v2 - _bse_vmin) += _BSECoefs(_idx1, state) * _BSECoefs(_idx2, state);
                }
            }

            // hole part as matrix products
            Eigen::MatrixXd _occlevels = _mo_coefficients.block(0, _bse_vmin, _mo_coefficients.rows(), _bse_vtotal);
            dmatEX[0] = _occlevels * _Avv * _occlevels.transpose();

            // electron part as matrix products
            Eigen::MatrixXd _virtlevels = _mo_coefficients.block(0, _bse_cmin, _mo_coefficients.rows(), _bse_ctotal);
            dmatEX[1] = _virtlevels * _Acc * _virtlevels.transpose();
            return dmatEX;
        }

        // Excited state density matrix

        std::vector<Eigen::MatrixXd > Orbitals::DensityMatrixExcitedState_AR(const string& spin, int state) {
            if (!(spin == "singlet")) {
                throw runtime_error("Spin type not known for density matrix. Available is singlet");
            }
            
            MatrixXfd& _BSECoefs_AR = _BSE_singlet_coefficients_AR;
            if(_BSECoefs_AR.cols()<state || _BSECoefs_AR.rows()<2){
                throw runtime_error("Orbitals object has no information about that state");
            }
            /******
             *
             *    Density matrix for GW-BSE based excitations
             *
             *    - electron contribution
             *      D_ab = \sum{vc} \sum{v'} B_{vc}B_{v'c} mo_a(v)mo_b(v')
             *
             *    - hole contribution
             *      D_ab = \sum{vc} \sum{c'} B_{vc}B_{vc'} mo_a(c)mo_b(c')
             *
             *
             *   more efficient:
             *
             *   - electron contribution
             *      D_ab = \sum{v} \sum{v'} mo_a(v)mo_b(v') [ \sum{c} B_{vc}B_{v'c} ]
             *           = \sum{v} \sum{v'} mo_a(v)mo_b(v') B_{vv'}
             *
             *   - hole contribution
             *      D_ab = \sum{c} \sum{c'} mo_a(c)mo_b(c') [ \sum{v} B_{vc}B_{vc'} ]
             *           = \sum{c} \sum{c'} mo_a(c)mo_b(c') B_{cc'}
             *
             */
            std::vector<Eigen::MatrixXd > dmatAR;
            dmatAR.resize(2);
            dmatAR[0] = Eigen::MatrixXd::Zero(_basis_set_size, _basis_set_size);
            dmatAR[1] = Eigen::MatrixXd::Zero(_basis_set_size, _basis_set_size);
            std::vector<int> _index2v;
            std::vector<int> _index2c;

            // indexing info BSE vector index to occupied/virtual orbital
            for (unsigned _v = 0; _v < _bse_vtotal; _v++) {
                for (unsigned _c = 0; _c < _bse_ctotal; _c++) {
                    _index2v.push_back(_bse_vmin + _v);
                    _index2c.push_back(_bse_cmin + _c);
                }
            }

            // hole assist matrix B_{cc'}
            Eigen::MatrixXd _Bcc = Eigen::MatrixXd::Zero(_bse_ctotal, _bse_ctotal);
            Eigen::MatrixXd _Bvv = Eigen::MatrixXd::Zero(_bse_vtotal, _bse_vtotal);

            for (unsigned _idx1 = 0; _idx1 < _bse_size; _idx1++) {
                int _v = _index2v[_idx1];
                int _c = _index2c[_idx1];
                // hole assist matrix B_{cc'}
#pragma omp parallel for
                for (unsigned _c2 = _bse_cmin; _c2 <= _bse_cmax; _c2++) {
                    unsigned _idx2 = (_bse_cmax - _bse_cmin + 1)*(_v - _bse_vmin)+(_c2 - _bse_cmin);
                    _Bcc(_c - _bse_cmin, _c2 - _bse_cmin) += _BSECoefs_AR(_idx1, state) * _BSECoefs_AR(_idx2, state);
                }

                // electron assist matrix B_{vv'}
#pragma omp parallel for
                for (unsigned _v2 = _bse_vmin; _v2 <= _bse_vmax; _v2++) {
                    unsigned _idx2 = (_bse_cmax - _bse_cmin + 1)*(_v2 - _bse_vmin)+(_c - _bse_cmin);
                    _Bvv(_v - _bse_vmin, _v2 - _bse_vmin) += _BSECoefs_AR(_idx1, state) * _BSECoefs_AR(_idx2, state);
                }
            }

            // hole part as matrix products
            Eigen::MatrixXd _occlevels = _mo_coefficients.block(0, _bse_vmin, _mo_coefficients.rows(), _bse_vtotal);
            dmatAR[0] = _occlevels * _Bvv * _occlevels.transpose();
            // electron part as matrix products
            Eigen::MatrixXd _virtlevels = _mo_coefficients.block(0, _bse_cmin, _mo_coefficients.rows(), _bse_ctotal);
            dmatAR[1] = _virtlevels * _Bcc * _virtlevels.transpose();
            return dmatAR;
        }

        Eigen::VectorXd Orbitals::LoewdinPopulation(const Eigen::MatrixXd & _densitymatrix, const Eigen::MatrixXd & _overlapmatrix, int _frag) {

            Eigen::VectorXd fragmentCharges = Eigen::VectorXd::Zero(2);
            Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> es;
            es.compute(_overlapmatrix);
            Eigen::MatrixXd sqrtm1 = es.operatorInverseSqrt();
            Eigen::MatrixXd _prodmat = sqrtm1 * _densitymatrix*sqrtm1;

            for (int _i = 0; _i < _frag; _i++) {
                fragmentCharges(0) += _prodmat(_i, _i);
            }
            for (unsigned _i = _frag; _i < _overlapmatrix.rows(); _i++) {
                fragmentCharges(1) += _prodmat(_i, _i);
            }

            return fragmentCharges;
        }

        std::vector<double> Orbitals::Oscillatorstrengths() {
            std::vector<double> oscs;
            unsigned size = _transition_dipoles.size();
            if (size > _BSE_singlet_energies.size()) {
                size = _BSE_singlet_energies.size();
            }
            for (unsigned i = 0; i < size; ++i) {
                double osc = (_transition_dipoles[i] * _transition_dipoles[i]) * 2.0 / 3.0 * (_BSE_singlet_energies(i));
                oscs.push_back(osc);
            }
            return oscs;
        }

        double Orbitals::GetTotalEnergy(string _spintype, int _opt_state) {

            // total energy of the excited state
            double _total_energy;
            double _omega = 0.0;

            double _dft_energy = getQMEnergy();

            if (_spintype == "singlet") {
                _omega = BSESingletEnergies()[_opt_state - 1];
            } else if (_spintype == "triplet") {
                _omega = BSETripletEnergies()[_opt_state - 1];
            } else {
                throw std::runtime_error("GetTotalEnergy only knows spintypes:singlet,triplet");
            }

            // DFT total energy is stored in eV
            // singlet energies are stored in Hrt...
            return _total_energy = _dft_energy * tools::conv::ev2hrt + _omega; //  e.g. hartree
        }

        Eigen::VectorXd Orbitals::FragmentNuclearCharges(int _frag) {
            Elements _elements;

            // go through atoms and count
            vector < QMAtom* > ::iterator atom;
            int id = 0;

            if (_frag < 0) {
                throw runtime_error("Orbitals::FragmentNuclearCharges Fragment index is smaller than zero");
            }

            Eigen::VectorXd fragmentNuclearCharges = Eigen::VectorXd::Zero(2);

            for (atom = _atoms.begin(); atom < _atoms.end(); ++atom) {
                id++;
                // get element type and determine its nuclear charge
                double crg = (*atom)->getNuccharge();
                // add to either fragment
                if (id <= _frag) {
                    fragmentNuclearCharges(0) += crg;
                } else {
                    fragmentNuclearCharges(1) += crg;
                }
            }
            return fragmentNuclearCharges;
        }

                /**
         * \brief Guess for a dimer based on monomer orbitals
         *
         * Given two monomer orbitals (A and B) constructs a guess for dimer
         * orbitals: | A 0 | and energies: [EA, EB]
         *           | 0 B |
         */
        void Orbitals::PrepareGuess(Orbitals* _orbitalsA, Orbitals* _orbitalsB, Orbitals* _orbitalsAB) {

            // constructing the direct product orbA x orbB
            int _basisA = _orbitalsA->getBasisSetSize();
            int _basisB = _orbitalsB->getBasisSetSize();

            int _levelsA = _orbitalsA->getNumberOfLevels();
            int _levelsB = _orbitalsB->getNumberOfLevels();

            int _electronsA = _orbitalsA->getNumberOfElectrons();
            int _electronsB = _orbitalsB->getNumberOfElectrons();


            Eigen::MatrixXd& _mo_coefficients = _orbitalsAB->MOCoefficients();
            _mo_coefficients = Eigen::MatrixXd::Zero(_basisA + _basisB, _levelsA + _levelsB);

            // AxB = | A 0 |  //   A = [EA, EB]  //
            //       | 0 B |  //                 //
            _orbitalsAB->setBasisSetSize(_basisA + _basisB);
            _orbitalsAB->setNumberOfLevels(_electronsA - _electronsB,
                    _levelsA + _levelsB - _electronsA - _electronsB);
            _orbitalsAB->setNumberOfElectrons(_electronsA + _electronsB);

            _mo_coefficients.block(0, 0, _basisA, _levelsA) = _orbitalsA->MOCoefficients();
            _mo_coefficients.block(_basisA, _levelsA, _basisB, _levelsB) = _orbitalsB->MOCoefficients();

            Eigen::VectorXd& _energies = _orbitalsAB->MOEnergies();
            _energies.resize(_levelsA + _levelsB);

            _energies.segment(0, _levelsA) = _orbitalsA->MOEnergies();
            _energies.segment(_levelsA, _levelsB) = _orbitalsB->MOEnergies();

            return;
        }
        //TODO move to Filereader

        void Orbitals::LoadFromXYZ(std::string filename) {

            string line;
            std::ifstream in;
            string type;
            in.open(filename.c_str(), std::ios::in);
            if (!in) throw runtime_error(string("Error reading coordinates from: ")
                    + filename);
            int atomCount = 0;

            if (in.is_open()) {
                while (in.good()) {
                    std::getline(in, line);

                    vector< string > split;
                    Tokenizer toker(line, " \t");
                    toker.ToVector(split);
                    if (!split.size() ||
                            split.size() != 4 ||
                            split[0] == "#" ||
                            split[0].substr(0, 1) == "#") {
                        continue;
                    }
                    // Interesting information written here: e.g. 'C 0.000 0.000 0.000'

                    string element = split[0];
                    double x = boost::lexical_cast<double>(split[1]);
                    double y = boost::lexical_cast<double>(split[2]);
                    double z = boost::lexical_cast<double>(split[3]);
                    tools::vec pos = tools::vec(x, y, z);
                    AddAtom(atomCount, element, pos * tools::conv::ang2bohr);
                    atomCount++;

                }
            } else {
                throw std::runtime_error("No such file: '" + filename + "'.");
            }
            return;
        }

        void Orbitals::WriteToCpt(const std::string& filename) {
            CheckpointFile cpf(filename, true);
            WriteToCpt(cpf);
        }

        void Orbitals::WriteToCpt(CheckpointFile f) {
            std::string name = "QMdata";
            CptLoc orbGr = f.getHandle().createGroup("/" + name);
            WriteToCpt(orbGr);
        }

        void Orbitals::WriteToCpt(CptLoc parent) {
            try {
                CheckpointWriter w(parent);
                w(XtpVersionStr(), "Version");
                w(_basis_set_size, "basis_set_size");
                w(_occupied_levels, "occupied_levels");
                w(_unoccupied_levels, "unoccupied_levels");
                w(_number_of_electrons, "number_of_electrons");

                w(_mo_energies, "mo_energies");
                w(_mo_coefficients, "mo_coefficients");

                // write qmatoms

                {
                    CptLoc qmAtomsGr = parent.createGroup("qmatoms");
                    size_t count = 0;
                    for (const auto& qma : _atoms) {
                        CptLoc tempLoc = qmAtomsGr.createGroup("atom" + std::to_string(count));
                        qma->WriteToCpt(tempLoc);
                        ++count;
                    }

                }

                w(_qm_energy, "qm_energy");
                w(_qm_package, "qm_package");
                w(_self_energy, "self_energy");

                w(_dftbasis, "dftbasis");
                w(_auxbasis, "auxbasis");

                w(_rpamin, "rpamin");
                w(_rpamax, "rpamax");
                w(_qpmin, "qpmin");
                w(_qpmax, "qpmax");
                w(_bse_vmin, "bse_vmin");
                w(_bse_vmax, "bse_vmax");
                w(_bse_cmin, "bse_cmin");
                w(_bse_cmax, "bse_cmax");

                w(_ScaHFX, "ScaHFX");

                w(_bsetype, "bsetype");
                w(_ECP, "ECP");

                w(_QPpert_energies, "QPpert_energies");
                w(_QPdiag_energies, "QPdiag_energies");

                w(_QPdiag_coefficients, "QPdiag_coefficients");
                w(_eh_t, "eh_t");

                w(_eh_s, "eh_s");

                w(_BSE_singlet_energies, "BSE_singlet_energies");

                w(_BSE_singlet_coefficients, "BSE_singlet_coefficients");

                w(_BSE_singlet_coefficients_AR, "BSE_singlet_coefficients_AR");

                w(_transition_dipoles, "transition_dipoles");

                w(_BSE_triplet_energies, "BSE_triplet_energies");
                w(_BSE_triplet_coefficients, "BSE_triplet_coefficients");

            } catch (H5::Exception& error) {
                throw std::runtime_error(error.getDetailMsg());
            }

        }

        void Orbitals::ReadFromCpt(const std::string& filename) {
            CheckpointFile cpf(filename, false);
            ReadFromCpt(cpf);
        }

        void Orbitals::ReadFromCpt(CheckpointFile f) {
            std::string name = "QMdata";
            CptLoc orbGr = f.getHandle().openGroup("/" + name);
            ReadFromCpt(orbGr);
        }

        void Orbitals::ReadFromCpt(CptLoc parent) {
            try {
                CheckpointReader r(parent);
                r(_basis_set_size, "basis_set_size");
                r(_occupied_levels, "occupied_levels");
                r(_unoccupied_levels, "unoccupied_levels");
                r(_number_of_electrons, "number_of_electrons");

                r(_mo_energies, "mo_energies");
                r(_mo_coefficients, "mo_coefficients");
                // Read qmatoms
                {
                    CptLoc qmAtomsGr = parent.openGroup("qmatoms");
                    size_t count = qmAtomsGr.getNumObjs();

                    for (size_t i = 0; i < count; ++i) {
                        CptLoc tempLoc = qmAtomsGr.openGroup("atom" + std::to_string(i));
                        QMAtom temp;
                        temp.ReadFromCpt(tempLoc);
                        AddAtom(temp);
                    }
                }

                r(_qm_energy, "qm_energy");
                r(_qm_package, "qm_package");
                r(_self_energy, "self_energy");

                r(_dftbasis, "dftbasis");
                r(_auxbasis, "auxbasis");

                r(_rpamin, "rpamin");
                r(_rpamax, "rpamax");
                r(_qpmin, "qpmin");
                r(_qpmax, "qpmax");
                r(_bse_vmin, "bse_vmin");
                r(_bse_vmax, "bse_vmax");
                r(_bse_cmin, "bse_cmin");
                r(_bse_cmax, "bse_cmax");
                _bse_vtotal = _bse_vmax - _bse_vmin + 1;
                _bse_ctotal = _bse_cmax - _bse_cmin + 1;
                _bse_size = _bse_vtotal * _bse_ctotal;

                r(_ScaHFX, "ScaHFX");

                r(_bsetype, "bsetype");
                r(_ECP, "ECP");

                r(_QPpert_energies, "QPpert_energies");
                r(_QPdiag_energies, "QPdiag_energies");

                r(_QPdiag_coefficients, "QPdiag_coefficients");
                r(_eh_t, "eh_t");

                r(_eh_s, "eh_s");

                r(_BSE_singlet_energies, "BSE_singlet_energies");

                r(_BSE_singlet_coefficients, "BSE_singlet_coefficients");

                r(_BSE_singlet_coefficients_AR, "BSE_singlet_coefficients_AR");

                r(_transition_dipoles, "transition_dipoles");

                r(_BSE_triplet_energies, "BSE_triplet_energies");
                r(_BSE_triplet_coefficients, "BSE_triplet_coefficients");

            } catch (H5::Exception& error) {
                throw std::runtime_error(error.getDetailMsg());
            }
        }
    }
}
