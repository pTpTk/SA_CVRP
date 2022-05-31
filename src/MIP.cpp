// #include "MIP.h"
// #include "ortools/linear_solver/linear_solver.h"

// #include <algorithm>

// MIP::MIP(std::vector<Rute>& routes, Params* params)
// : routes(routes),
//   params(params)
// {}

// using namespace operations_research;

// void MIP::work() {
//     // Create the mip solver with the SCIP backend.
//     std::unique_ptr<MPSolver> solver(MPSolver::CreateSolver("SCIP"));
//     if (!solver) {
//         std::cout << "SCIP solver unavailable.";
//         return;
//     }

//     const double infinity = solver->infinity();
//     // create integer non-negative variables.
//     std::vector<const MPVariable*> Xs(routes.size());
//     for(auto& x : Xs){
//         x = solver->MakeIntVar(-0.5, 1.5, "");
//     }
//     std::cout << "Number of variables = " << solver->NumVariables();

//     // set constraints
//     int Cs_size = params->cli.size() - 1;
//     std::vector<MPConstraint*> Cs(Cs_size);
//     for(auto& c : Cs){
//         c = solver->MakeRowConstraint(0.5, 1.5, "");
//     }
//     //for(int i = 0; i < routes.size(); i++){
//     //    auto& rc = routes[i].clients;
//     //    for(int j = 0; j < Cs_size; j++){
//     //        if(std::find(rc.begin(), rc.end(), j) == rc.end())
//     //            Cs[j]->SetCoefficient(Xs[i], 0.);
//     //    }
//     //}
            
//     for(int i = 0; i < routes.size(); i++){
//         for(int c : routes[i].clients){
//             Cs[c-1]->SetCoefficient(Xs[i], 1);
//         }
//     }

//     std::cout << "Number of constraints = " << solver->NumConstraints();
   
//     // set objective
//     MPObjective* const objective = solver->MutableObjective();
//     for(int i = 0; i < routes.size(); i++){
//         objective->SetCoefficient(Xs[i], routes[i].totalCost);
//     }
//     objective->SetMinimization();
   
//     const MPSolver::ResultStatus result_status = solver->Solve();
//     // Check that the problem has an optimal solution.
//     if (result_status != MPSolver::OPTIMAL) {
//         std::cout << "The problem does not have an optimal solution!";
//     }
   
//     std::cout << "\n\nSolution:\n";
//     std::cout << "Objective value = " << objective->Value();
//     for(int i = 0; i < routes.size(); i++){
//         if(Xs[i]->solution_value() > 0){
//             printf("\nRoute: ");
//             for(auto c : routes[i].clients)
//                 std::cout << c << " ";
//         }
//     }
   
//     std::cout << "\nAdvanced usage:";
//     std::cout << "Problem solved in " << solver->wall_time() << " milliseconds";
//     std::cout << "Problem solved in " << solver->iterations() << " iterations";
//     std::cout << "Problem solved in " << solver->nodes()
//               << " branch-and-bound nodes";
// }
