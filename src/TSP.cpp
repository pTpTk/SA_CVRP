#include "TSP.h"
#include <vector>

TSP::TSP(Params* p, V<int>& vi, V<double>& vd)
: params(p), Coeff(vd), r_in(vi)
{}

void TSP::test()
{
    spin[ 0] = V<bool>{0, 0, 1};
    spin[ 1] = V<bool>{0, 1, 0};
    spin[ 2] = V<bool>{1, 0, 0};
    spin[ 3] = V<bool>{0, 0, 0};
    qubo_ene();
    spin[ 0] = V<bool>{0, 1, 0};
    spin[ 1] = V<bool>{1, 0, 0};
    spin[ 2] = V<bool>{0, 0, 1};
    spin[ 3] = V<bool>{0, 0, 0};
    qubo_ene();
    spin[ 0] = V<bool>{1, 0, 0};
    spin[ 1] = V<bool>{0, 0, 1};
    spin[ 2] = V<bool>{0, 1, 0};
    spin[ 3] = V<bool>{0, 0, 0};
    qubo_ene();
    spin[ 0] = V<bool>{1, 0, 0};
    spin[ 1] = V<bool>{0, 1, 0};
    spin[ 2] = V<bool>{0, 0, 0};
    spin[ 3] = V<bool>{0, 0, 1};
    qubo_ene();
    spin[ 0] = V<bool>{0, 0, 0};
    spin[ 1] = V<bool>{1, 0, 0};
    spin[ 2] = V<bool>{0, 1, 0};
    spin[ 3] = V<bool>{0, 0, 1};
    qubo_ene();
    spin[ 0] = V<bool>{0, 1, 0};
    spin[ 1] = V<bool>{0, 0, 0};
    spin[ 2] = V<bool>{1, 0, 0};
    spin[ 3] = V<bool>{0, 0, 1};
    qubo_ene();
}

void TSP::work(int iter=8000)
{
    sweeps = iter;
    scale = sweeps > 1 ? (beta1 - beta0) / (sweeps - 1) : 0.00;
    for(int i = 0; i < r_in.size(); i++){
        init(i);
        SA();
    }
}

void TSP::fill(int i){
    V<int> n_i(params->correlatedVertices[i]);
    for(int n : nodelist){
        auto pos = std::find(n_i.begin(), n_i.end(), n);
        if(pos != n_i.end())
            n_i.erase(pos);
    }

    nodelist.insert(nodelist.end(), n_i.begin(), n_i.end());
    nodelist.emplace_back(0);
#ifdef DEBUG
    printf("nodelist.size() = %d\n", nodelist.size());
#endif
}

/* Last item of the matrix is the depot node. Since the route has to start from 
 * the depot, it's implied in the assignment. The depot node is to satisfy other
 * properties of the solution. */

void TSP::init(int i)
{
    nodelist = r_in;
    fill(r_in[i]);
#ifdef DEBUG
    for(auto& n : nodelist){
        std::cout << n << ' ';
    }
    std::cout << std::endl;
#endif
    
    // Dist[i][j] = dist(i -> j)
    graph_size_i = nodelist.size();
    graph_size_j = nodelist.size()-1;

    Distance.clear();
    Demand.clear();

    Distance.resize(graph_size_i); // final row for depot
    Demand.resize(graph_size_i);
   
    for(auto& d : Distance)
        d.resize(graph_size_i);    // final column for depot

    for(int j = 0; j < graph_size_i; j++){
        auto nj = nodelist[j];
        Distance[j][j] = 0;
        for(int k = j+1; k < graph_size_i; k++){
            auto nk = nodelist[k];
            Distance[j][k] = Distance[k][j] = params->timeCost[nj][nk];
        }
        Demand[j] = params->cli[nj].demand;
    }
    
    spin.resize(graph_size_i);

    srand(time(NULL));

    for(auto& s : spin){
        s.resize(graph_size_j);
        for(int j = 0; j < graph_size_j; j++)
            s[j] = rand() % 2;
    }

    beta = beta0;
}

void TSP::SA()
{
    int counter(0);
    double d, ene;

    ene = qubo_ene();

    for(int iter = 0; iter < sweeps; iter++){
        //counter++;
        for(int i = 0; i < graph_size_i; i++){
            for(int j = 0; j < graph_size_j; j++){
                d = diff(i, j);
                if(accept(d)){
                    spin[i][j] = !spin[i][j];
                    ene += d;
#ifdef DEBUG
    assert(ene == qubo_ene());
    printf("spin[%d][%d] flipped\n", i, j);
    
    for(int i = 0; i < graph_size_i; i++){
        for(int j = 0; j < graph_size_j; j++){
            std::cout << spin[i][j] << ' ';
        }
        std::cout << std::endl;
    }
#endif
                }
            }
        }
        beta += scale;
        //if(counter > 50) break;
    }

    assert(ene == qubo_ene());
    update_out();
}

double TSP::qubo_ene(){
    double ene(0);
    int ene_row(0), ene_col(1), ene_depot(0);
    double ene_dis(0), ene_dem(params->vehicleCapacity*params->vehicleCapacity);

    // ensure only one 1 each column (each round has only 1 destination)
    for(int i = 0; i < graph_size_i; i++){
        for(int j = 0; j < graph_size_j; j++){
            for(int k = 0; k < graph_size_i; k++){
                if(i != k)
                ene_col += spin[i][j] * spin[k][j];
            }
            ene_col -= spin[i][j];
        }
    }
    ene += Coeff[0] * ene_col;

    for(int i = 0; i < graph_size_i-1; i++){
        for(int j = 0; j < graph_size_j; j++){
            for(int k = 0; k < graph_size_j; k++){
                if(j != k)
                ene_row += spin[i][j] * spin[i][k];
            }
        }
    }
    ene += Coeff[1] * ene_row;

    for(int i = 0; i < graph_size_i-1; i++){
        for(int j = 0; j < graph_size_j-1; j++){
            for(int k = j+1; k < graph_size_j; k++){
                ene_depot += spin[graph_size_i-1][j] * spin[i][k];
            }
        }
    }
    ene += Coeff[2] * ene_depot;

    for(int i = 0; i < graph_size_i; i++){
        for(int j = 0; j < graph_size_j; j++){
            ene_dem += Demand[i] * Demand[i] * spin[i][j];
            ene_dem -= Demand[i] * 2 * params->vehicleCapacity * spin[i][j];
            for(int k = 0; k < graph_size_i; k++){
                for(int l = 0; l < graph_size_j; l++){
                    if(i != k || j != l)
                    ene_dem += Demand[i] * Demand[k] * spin[i][j] * spin[k][l];
                }
            }
        }
    }
    ene += Coeff[3] * ene_dem;
    
    for(int i = 0; i < graph_size_i; i++){
        ene_dis += spin[i][0]     * Distance[i].back();
        ene_dis += spin[i].back() * Distance[i].back();
        
        for(int k = 0; k < graph_size_i; k++){
            for(int j = 0; j < graph_size_j-1; j++){
                ene_dis += spin[i][j] * spin[k][j+1] * Distance[i][k];
            }
        }
    }
    ene += Coeff[4] * ene_dis;

#ifdef DEBUG
    printf("ene_col = %.1f\n", Coeff[0] * ene_col);
    printf("ene_row = %.1f\n", Coeff[1] * ene_row);
    printf("ene_depot = %.1f\n", Coeff[2] * ene_depot);
    printf("ene_dis = %.1f\n", Coeff[4] * ene_dis);
    printf("ene_dem = %.1f\n", Coeff[3] * ene_dem);
    printf("qubo_ene = %.1f\n", ene);
#endif
    return ene;
}

double TSP::diff(int i, int j)
{
    int diff_row(0), diff_col(0), diff_depot(0);
    double diff_dis(0), diff_dem, diff;
    double dem_i, dem_k;

    // ensure only one 1 each column (each round has only 1 destination)
    for(int k = 0; k < graph_size_i; k++)
        if(k != i)
            diff_col += spin[k][j];
    diff_col = 2 * diff_col - 1;

    // ensure no more than one 1 each row (go to a client no more than once)
    if(i != graph_size_i-1){
        for(int k = 0; k < graph_size_j; k++)
            if(k != j)
                diff_row += spin[i][k];
        diff_row *= 2;
    }

    // depot constraint
    if(i == graph_size_i-1){
        if(j != graph_size_j-1){
            for(int k = 0; k < graph_size_i-1; k++){
                for(int l = j+1; l < graph_size_j; l++){
                    diff_depot += spin[k][l];
                }
            }
        }
    }
    else{
        for(int k = 0; k < j; k++){
            diff_depot += spin[graph_size_i-1][k];
        }
    }

    // capacity constraint
    dem_i = Demand[i];
    diff_dem = dem_i * dem_i - 2 * params->vehicleCapacity * dem_i;
    for(int k = 0; k < graph_size_i; k++){
        dem_k = Demand[k];
        for(int l = 0; l < graph_size_j; l++){
            if(k != i || l != j)
            diff_dem += 2 * dem_i * dem_k * spin[k][l];
        }
    }
    
    // distance constraint
    if(j == 0 || j == graph_size_j-1)
        diff_dis += params->timeCost[0][nodelist[i]];
    for(int k = 0; k < graph_size_i; k++){
        diff_dis += (j == 0)              ? 0 : Distance[k][i] * spin[k][j-1];
        diff_dis += (j == graph_size_j-1) ? 0 : Distance[i][k] * spin[k][j+1];
    }


    diff = Coeff[0] * diff_col + Coeff[1] * diff_row + Coeff[2] * diff_depot + Coeff[3] * diff_dem + Coeff[4] * diff_dis;
#ifdef DEBUG
    printf("\ndiff_col = %d\n", diff_col);
    printf("diff_row = %d\n", diff_row);
    printf("diff_depot = %d\n", diff_depot);
    printf("diff_dem = %f\n", diff_dem * Coeff[3]);
    printf("diff_dis = %f\n", diff_dis * Coeff[4]);
    printf("diff = %f\n", diff);
#endif                    
    return (spin[i][j]) ? -diff : diff;
}

bool TSP::validate(){
    double dem;
    
    // check each col for only one 1
    int tmp_col;
    bool col_bool(true);
    for(int j = 0; j < graph_size_j; j++){
        tmp_col = 0;
        for(int i = 0; i < graph_size_i; i++){
            tmp_col += spin[i][j];
        }
        if(tmp_col != 1){
            col_bool = false;
            break;
        }
    }

    // check each row for at most one 1 (except the last row)
    int tmp_row;
    bool row_bool(true);
    for(int i = 0; i < graph_size_i-1; i++){
        tmp_row = 0;
        for(int j = 0; j < graph_size_j; j++){
            tmp_row += spin[i][j];
        }
        if(tmp_row > 1){
            row_bool = false;
            break;
        }
    }

    // check depot condition
    bool depot_bool(true);
    int first_depot(graph_size_j);
    for(int i = 0; i < graph_size_j; i++){
        if(spin[graph_size_i-1][i]){
            first_depot = i;
            break;
        }
    }
    if(first_depot < graph_size_j){
        for(int i = first_depot; i < graph_size_j; i++){
            for(int j = 0; j < graph_size_i-1; j++){
                if(spin[j][i])
                    depot_bool = false;
                    goto extract;
            }
        }
    }

    extract:
    V<int> r;
    for(int i = 0; i < graph_size_i; i++){
        for(int j = 0; j < graph_size_j; j++){
            if(spin[j][i]){
                if(i != graph_size_i-1)
                    r.emplace_back(nodelist[i]);
                else goto cap_check;
            }
        }
    }

    cap_check:
    dem = 0;
    bool cap_bool(true);
    for(auto c : r){
        dem += params->cli[c].demand;
    }
    if(dem > params->vehicleCapacity) {
        cap_bool = false;
    }

    bool failed(false);
    if(!col_bool){
        failed = true;
#ifdef TUNE
        fprintf(stderr, "column constraint failed\n");
#endif
    }
    if(!row_bool){
        failed = true;
#ifdef TUNE
        fprintf(stderr, "row constraint failed\n");
#endif
    }
    if(!depot_bool){
        failed = true;
#ifdef TUNE
        fprintf(stderr, "depot constraint failed\n");
#endif
    }
    if(!cap_bool){
        failed = true;
#ifdef TUNE
        fprintf(stderr, "capacity constraint failed\n");
#endif
    }
#ifdef TUNE
    fflush(stderr);
#endif
    if(failed) return false;

    r_out.emplace_back(r);
    return true;
}

void TSP::update_out()
{
    V<int> r;
    int tmp(0);
#ifdef DEBUG
    for(int i = 0; i < graph_size_i; i++){
        for(int j = 0; j < graph_size_j; j++){
            std::cout << spin[i][j] << ' ';
        }
        if(i < graph_size_j)
            std::cout << "\t" << nodelist[i];
        else
            std::cout << "\t0";
        std::cout << std::endl;
    }
#endif
    bool valid = validate();
#if defined DEBUG || defined TUNE
    if(!valid){
        std::cout << "Invalid Result\n";
    }
    else{
        std::cout << "new route: ";
        for(auto c : r_out.back()){
            std::cout << c << ' ';
        }
        std::cout << std::endl;
    }
#endif
}

bool TSP::accept(double d)
{
    return (d < 0) || (dis(gen) < exp(-beta * d));
}

