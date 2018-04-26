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


#include <exception>
#include <votca/xtp/nbo.h>
#include <votca/xtp/aomatrix.h>

namespace votca { namespace xtp {

void NBO::EvaluateNBO(std::vector< QMAtom* >& _atomlist,const  Eigen::MatrixXd &_dmat,const AOBasis &basis, BasisSet &bs){
    AOOverlap _overlap;
    // Fill overlap
    _overlap.Fill(basis);
  
    Eigen::MatrixXd P=_overlap.Matrix()*_dmat*_overlap.Matrix();
   
    Eigen::MatrixXd PNAOs_trans=IntercenterOrthogonalisation(P,_overlap.Matrix(), _atomlist ,bs);
    
    cout<<P<<endl;
    cout<<_overlap.Matrix()<<endl;
    
      throw invalid_argument("Evaluate NBO function is incomplete");    
   
    return;
}


Eigen::MatrixXd NBO::IntercenterOrthogonalisation(Eigen::MatrixXd &P, Eigen::MatrixXd &overlap,vector< QMAtom* >& _atomlist , BasisSet &bs){
    
    
    vector< QMAtom* >::iterator atom;

// make an array which stores for each atom the the starting point for all s functions, p functions, d functions etc...   
    
    
    //atom/shell/starting 
    std::vector< std::map< int,std::vector< int > > >sorting;
    
    int functionindex=0;
    for (atom = _atomlist.begin(); atom < _atomlist.end(); ++atom){
        std::map< int,std::vector< int > > shellsort;
        Element* element = bs.getElement((*atom)->getType());
        for (Element::ShellIterator its = element->firstShell(); its != element->lastShell(); its++) {
            
           // for loop because shells can also consist of SP shells or alike
            for(unsigned i = 0; i <(*its)->getType().length(); ++i) {
            string local_shell = string( (*its)->getType(), i, 1 );
            int l=FindLmax(local_shell);
            std::vector< int >& index =shellsort[l];
           
            index.push_back(functionindex);
            
            functionindex+=NumFuncShell(local_shell);
            }
        }
        sorting.push_back(shellsort);
         }
    typedef std::map< int,std::vector< int > >::iterator it_type;
    
    
    for (unsigned i=0;i<sorting.size();i++){
        cout << "Atom "<<i<<endl;
        
        for(it_type iterator = sorting[i].begin(); iterator != sorting[i].end(); iterator++) {
    cout<< "Shell "<<iterator->first<<":";
    std::vector< int >& index=iterator->second;
    for (unsigned j=0;j<index.size();j++){
        cout <<" "<<index[j];
    }
    cout<<endl;
}
    }
    
    
    Eigen::MatrixXd transformation=Eigen::MatrixXd::Zero(P.rows(),P.cols());
    Eigen::VectorXd occupancies=Eigen::VectorXd::Zero(P.rows());
    // Stting up atomic transformations
    
    //atoms
    for (unsigned i=0;i<sorting.size();i++){
        //shell
        for(it_type iterator = sorting[i].begin(); iterator != sorting[i].end(); iterator++) {
            int ll=2*(iterator->first)+1;
            std::vector< int >& index=iterator->second;
            unsigned size=index.size();
            Eigen::MatrixXd P_L=Eigen::MatrixXd::Zero(size,size);
            Eigen::MatrixXd S_L=Eigen::MatrixXd::Zero(size,size);
            
            //filling P_L and S_L
            for (unsigned i=0;i<size;i++){
                for (unsigned j=0;j<size;j++){
                    //summing over m
                    for (int m=0;m<ll;m++){
                        P_L(i,j)+=P(index[i]+m,index[j]+m);
                        S_L(i,j)+=overlap(index[i]+m,index[j]+m);
                    } 
                }
            }
            cout << P_L<< endl;
            cout << S_L<<endl;
            
            //Setup generalized eigenproblem for each P_L/S_L
            
            Eigen::GeneralizedSelfAdjointEigenSolver<  Eigen::MatrixXd > es(P_L,S_L);
        
            
            for (unsigned i=0;i<size;i++){
                for (int m=0;m<ll;m++){
                occupancies(index[i]+m)=es.eigenvalues()(i);
                }
                for (unsigned j=0;j<size;j++){
                    for (int m=0;m<ll;m++){
                    transformation(index[i]+m,index[j]+m)=es.eigenvectors()(i,j);
                }
            }
        }
        }
    }
        cout<<transformation<<endl;
        cout<<occupancies<<endl;
    
       P=transformation*P*transformation.transpose();
       overlap=transformation*overlap*transformation.transpose();
    
    return transformation;
}




}}