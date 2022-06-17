#include "Genetic.h"
#include "commandline.h"
#include "LocalSearch.h"
#include "Split.h"
#include "TSP.h"
#include "Route.h"
#include "MIP.h"
#include "List.h"
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
			List l;
			l.add(population.getBestFound()->chromR);
			while(l.route_list.size() < 20){
				for(auto& r : l.route_list){
					TSP tsp(&params, r, A);
					tsp.work(commandline.nbIter);
					l.add(tsp.r_out);
				}
				cout << "route pool size: " << l.route_list.size() << endl;
				for(auto& lr : l.route_list){
					for(auto& c : lr){
						cout << c << " ";
					}
					cout << endl;
				}
			}
			vector<vector<int>> r_out(l.out());
#ifdef DEBUG
			for(int i = 0; i < r_out.size(); i++) {
				auto& ro = r_out.at(i);
				printf("Route[%d]: ", i);
				double demand(0);
				for(auto n : ro){
					demand += params.cli[n].demand;
					std::cout << n << ' ';
				}
				std::cout << "valid: " << (demand <= params.vehicleCapacity) << std::endl;
			}
#endif
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
