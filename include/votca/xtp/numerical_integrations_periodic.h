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

#ifndef __XTP_NUMERICAL_INTEGRATION_PERIODIC__H
#define	__XTP_NUMERICAL_INTEGRATION_PERIODIC__H


#include <votca/xtp/numerical_integrations.h>
#include <votca/xtp/grid.h>

namespace votca { namespace xtp {

    namespace ub = boost::numeric::ublas;
    
    class NumericalIntegrationPeriodic: public NumericalIntegration {
        public: 
            
            //void GridSetup(std::string type, BasisSet* bs , std::vector<ctp::QMAtom* > _atoms,AOBasis* basis  );
            
            double IntegrateDensity_Molecule(ub::matrix<double>& _density_matrix, AOBasis* basis, std::vector<int> AtomIndeces);

            double IntegratePotential_w_PBC(vec rvector);
            void IntegratePotential_w_PBC_gromacs_like(Grid &eval_grid, ub::vector<double>& _ESPatGrid);
            double IntegrateEnergy_w_PBC(vec rvector);
            double CalcDipole_w_PBC(vec rvector);
            void findAlpha(double Rc, double dtol);
            void PrepKspaceDensity(double ext_alpha, std::vector< ctp::QMAtom* > & _local_atomlist, bool ECP, int nK);
            void PrepKspaceDensity_gromacs_like(double ext_alpha, std::vector< ctp::QMAtom* > & _local_atomlist, bool ECP, Grid &eval_grid, int nK);
            void FreeKspace(void);
            inline void setBox(vec box){
                boxLen=box;
                return;
            }
            
            //used for debuging and testing
            std::vector< std::vector< GridContainers::integration_grid > > _Madelung_grid;
            void FillMadelungGrid(vec boxLen, int natomsonside);

                    
        private:
            void FindCenterCenterDist(vector<ctp::QMAtom*> _atoms);
            std::vector< std::vector<double> > FindGridpointCenterDist(vector<ctp::QMAtom*> _atoms, std::vector< GridContainers::integration_grid > _atomgrid);
            
            std::complex<double>* Rho_k; //density in k-space, used for Ewald summation of potential in periodic systems
            std::vector<std::vector<std::vector< std::complex<double> > > > eikR;  //gromacs-like storage for exp(k*R) -> where to evaluate
            std::vector<std::vector<std::vector< std::vector<std::complex<double> > > > > eikr;  //gromacs-like storage for exp(k*r) -> charge distribution
            vec lll;
            vec boxLen; //in Bohr
            int numK[3];   //number of k-vectors along each axis
            double alpha;  //inverse length in Ewald summation
            double *Kcoord;//k-values
            double *prefactor; //prefactors for k-space components of potential before summation
            double E_rspace;
            double E_kspace;
            double E_erfc;
    };
        
    tools::vec WrapPoint(const tools::vec r, const tools::vec box);
    tools::vec WrapDisplacement(const tools::vec a, const tools::vec b, const tools::vec box);

}}
#endif	/* NUMERICAL_INTEGRATION_PERIODIC__H */
