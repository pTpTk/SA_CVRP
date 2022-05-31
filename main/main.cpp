#include "Genetic.h"
#include "commandline.h"
#include "LocalSearch.h"
#include "Split.h"
#include "TSP.h"
#include "Route.h"
#include "MIP.h"
using namespace std;

int main(int argc, char *argv[])
{
	try
	{
		// Reading the arguments of the program
		CommandLine commandline(argc, argv);

		// Reading the data file and initializing some data structures
		std::cout << "----- READING DATA SET: " << commandline.pathInstance << std::endl;
		Params params(commandline.pathInstance, commandline.nbVeh, commandline.seed);

		// Creating the Split and local search structures
		Split split(&params);
		LocalSearch localSearch(&params);

		// Initial population
		std::cout << "----- INSTANCE LOADED WITH " << params.nbClients << " CLIENTS AND " << params.nbVehicles << " VEHICLES" << std::endl;
		std::cout << "----- BUILDING INITIAL POPULATION" << std::endl;
		Population population(&params, &split, &localSearch);

		// Genetic algorithm
		std::cout << "----- STARTING GENETIC ALGORITHM" << std::endl;
		Genetic solver(&params, &split, &population, &localSearch);
		solver.run(5, commandline.timeLimit);
		std::cout << "----- GENETIC ALGORITHM FINISHED, TIME SPENT: " << (double)clock()/(double)CLOCKS_PER_SEC << std::endl;

		// Exporting the best solution
		if (population.getBestFound() != NULL)
		{
            std::vector<double> A = commandline.A;
			std::vector<std::vector<int>> r_out;
            // auto& r = population.getBestFound()->chromR[0];
            // TSP tsp(&params, r, A);
            // tsp.work(commandline.nbIter);
			for(auto& r : population.getBestFound()->chromR){
                TSP tsp(&params, r, A);
                tsp.work(commandline.nbIter);
				r_out.insert(r_out.end(), tsp.r_out.begin(), tsp.r_out.end());
            }
			cout << r_out.size();
			for(int i = 0 ; i < r_out.size(); i++) {
				auto& ro = r_out.at(i);
				printf("Route[%d]: ", i);
				double demand(0);
				for(auto n : ro){
					demand += params.cli[n].demand;
					std::cout << n << ' ';
				}
				// if(demand > params.vehicleCapacity) {
				// 	printf("i = %d\n", i);
				// 	r_out.erase(r_out.begin()+i); i--;
				// 						printf("i = %d\n", i);

				// }
				std::cout << "valid: " << (demand <= params.vehicleCapacity) << std::endl;
			}
			R r(&params, r_out);
			MIP mip(r.routes, &params);
			mip.work();
		}
		else
			std::cout << "no initial solutions found" << std::endl;
        std::cout << std::endl;
	}
	catch (const string& e) { std::cout << "EXCEPTION | " << e << std::endl; }
	catch (const std::exception& e) { std::cout << "EXCEPTION | " << e.what() << std::endl; }
	return 0;
}
