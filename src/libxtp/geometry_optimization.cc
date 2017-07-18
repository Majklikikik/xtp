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

#include <votca/tools/linalg.h>
#include <votca/xtp/geometry_optimization.h>
#include <votca/xtp/forces.h>
#include <votca/xtp/bfgs-trm.h>

namespace votca {
    namespace xtp {

        void GeometryOptimization::Initialize(Property *options) {

            // checking if there is only one segment
            _nsegments = _segments.size();
            if (_nsegments > 1) throw runtime_error(string("\n Geometry optimization of more than 1 conjugated segment not supported. Stopping!"));

            _opt_state = options->ifExistsReturnElseReturnDefault<int>(".state", 1);
            _spintype = options->ifExistsReturnElseReturnDefault<string>(".spintype", "singlet");


            // pre-check optimizer method
            std::vector<string> choices = {"BFGS-TRM"};
            _optimizer = options->ifExistsAndinListReturnElseThrowRuntimeError<string>(".optimizer.method", choices);
            _optimizer_options = options->get(".optimizer");

            // pre-check forces method
            choices = {"forward", "central"};
            _force_method = options->ifExistsAndinListReturnElseThrowRuntimeError<string>(".forces.method", choices);
            _force_options = options->get(".forces");

            _natoms = _segments[0]->Atoms().size();

            return;

        }

        void GeometryOptimization::Evaluate() {


            CTP_LOG(ctp::logINFO, *_pLog) << "Requested geometry optimization of excited state " << _spintype << " " << _opt_state << flush;

            // get a force object
            Forces _force_engine(_gwbse_engine, _qmpackage, _segments, _orbitals);
            _force_engine.Initialize(&_force_options);
            _force_engine.setLog(_pLog);
            _force_engine.SetOptState(_opt_state);
            _force_engine.SetSpinType(_spintype);


            // get the optimizer
            if (_optimizer == "BFGS-TRM") {

                BFGSTRM _bfgstrm(_gwbse_engine, _qmpackage, _segments, _orbitals, _force_engine);
                _bfgstrm.Initialize(&_optimizer_options);
                _bfgstrm.setLog(_pLog);
                _bfgstrm.Optimize();

            }

            return;
        }

    }
}
