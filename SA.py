
import os
import numpy as np
import subprocess as sp
from shutil import copy, copytree, rmtree
from datetime import datetime
from itertools import product

_main = "/scratch/mhuang_lab/ISING_MACHINES/"
gset = {}

def process() :
    params               = {}
    params['iter']       = list(range(20))
    params['init']       = ['RND']
    # params['work']       = ["combo_int_full"]
    #params['work']       = ["scaling_int_full"]
    params['work']       = ["K_graphs"]
    params['gset']       = ["K8192"]
    # params['gset']       = ["G001"]
    #params['gset']       = ['G{:03}'.format(x) for x in list(range(2, 5))]
    #params['gset']      += ['G{:03}'.format(x) for x in list(range(25, 48))]

    # params['sweeps']     = ["10", "100", "500", "1000", "2000", "5000", "10000", "50000", "100000", "500000"]
    params['sweeps']     = ["1000000", "5000000", "10000000"]
    #params['trace_energy']= ["true"]
    #params['beta0']      = ["1", "0.1", "0.01", "0.001"]
    #params['beta1']      = []
    #params['istop']      = []
    #params['scale']      = []
    #params['cut_time']   = []
    #params['acpt_steps'] = []
    #params['seed']       = []
    #params['debug']      = []
    #params['dump_ene']   = []
    #params['track_best'] = []
    #params['track_spins']= []

    # Fixed config
    _this          = os.path.dirname(os.path.abspath(__file__))
    _here          = "/".join(_this.split('/')[0:-1]) + "/ISING_RESULTS"
    _file          = _this.split("/")[-1]
    
    # Flexible config
    _local         = False
    _time          = 1
    _memo          = 8
    _simm          = 10
    _config        = "scaling_softhard"
    _name          = "sim{:03}_{:}".format(_simm, _config)
    _jobs          = 0

    # Queueing
    _queue         = ["ising"]
    #_queue         = ["phi"]
    #_queue         = ["reserved --reservation=rafoakwa-20210409"]
    _batch         = "ising"
    _pque          = "{:}/queues/{:}_{:}"
    _date          = datetime.now().strftime("%Y_%m_%d_%H_%M_%S")
    _pque          = _pque.format(_here, _name, _date)
    create_dir(_pque)
    
    # Configurations
    _configurations= {}

    #
    # Process Parameters
    #
    _dic = dict((k, params[k]) for k in params.keys() if type(params[k]) != list)
    _keys= tuple([k for k in params.keys() if type(params[k]) == list])
    for _vals in product(*list([v for v in params.values() if type(v) == list])) :

        ## Create parameters
        assert(len(_vals) == len(_keys))
        dic = dict((_keys[i], _vals[i]) for i in range(len(_vals)))
        dic.update(_dic)

        append  = ""
        cfgs    = base_config()
        cfgs["seed"]["value"] = dic["iter"]

        for c in dic :
            if "init" == c :
                cfgs[c] = {
                            "flag"  : False,
                            "value" : "{:}/INITS/{:}/{:03}.txt".format(_main, dic["init"], dic["iter"])
                          }
            elif "gset" == c :
                cfgs[c] = {
                            "flag"  : False,
                            "value" : "{:}/GSET/{:}/{:}".format(_main, dic["work"], dic["gset"])
                          }
            else :
                if c != "iter" and c != "work" :
                    if c in cfgs :
                        cfgs[c]["value"] = dic[c]
                        append += "{:}_{:}_".format(c, dic[c])
                    else :
                        cfgs[c] = {"flag" : False, "value" : dic[c]}

        if len(append) == 0 : append = "generic"
        else : append = append[:-1]

        # Create working directories
        _cpath = "{:}/{:}/{:}/{:}".format(_here, _file, _name, append)
        _fpath = "{:}/{:}/{:}/{:03}".format(_cpath, dic["work"], dic["gset"], dic["iter"])
        create_dir(_fpath)
        cfgs["output"] = {"flag" : False, "value" : _fpath}

        # Extract best solutions
        if dic["gset"] not in gset : gset[dic["gset"]] = {}
        if dic["work"] not in gset[dic["gset"]] : read_best_cuts(_main, dic["work"])

        # Add best solution
        if dic["gset"] in gset and dic["work"] in gset[dic["gset"]] :
            cuts = float(gset[dic["gset"]][dic["work"]]["ct"])
            cfgs["cuts"] = {"flag" : False, "value" : cuts }
        
        # Create code directory
        if append not in _configurations :
            _configurations[append] = 0
            code_build(cfgs, _this, _cpath, _fpath)

        # Job name
        _jobs += 1
        _jobn = "{:04}_{:}".format(_jobs, _file.upper())

        # Create launch script
        launch_this  = "#!/bin/bash\n"
        launch_this += "#SBATCH --partition=THE_QUEUE\n"
        launch_this += "#SBATCH --job-name={:}\n".format( _jobn )
        launch_this += "#SBATCH --error={:}.err.%j\n".format( _jobn )
        launch_this += "#SBATCH --output={:}.run.%j\n".format( _jobn )
        launch_this += "#SBATCH --mem-per-cpu={0}GB\n".format( _memo )
        launch_this += "#SBATCH --time={0}:00:00\n".format( _time )
        launch_this += "\n"
        launch_this += "module unload gcc;\n"
        launch_this += "module load gcc/7.3.0;\n"
        launch_this += "rm -rf sa;\n"
        launch_this += 'g++ -std=c++17 -O3 -funroll-loops -I{:}/include/ -I{:}/ -DPARAMS=\\"params.hh\\" -o sa {:}/src/*.cc;\n'.format(_cpath + '/code', _fpath, _cpath + '/code')
        # launch_this += "cd {:};\n".format(_fpath)
        # launch_this += "{:}/code/sa".format(_cpath)
        launch_this += "./sa"
        launch_this += " \\\n\t--gset {:}".format(cfgs["gset"]["value"])
        launch_this += " \\\n\t--init {:}".format(cfgs["init"]["value"])
        if "cuts" in cfgs :
            launch_this += " \\\n\t--best {:}".format(cfgs["cuts"]["value"])
        launch_this += " \\\n\t-m"
        launch_this += "\n"

        create_script(_path = _fpath, _name = "launch_this", _data = launch_this)

        # Create params file
        par = code_gen(cfgs)
        write_file(_fpath, par, "params.hh")

        # Queue file 
        launch_queu = ""
        if _jobs == 1 :  launch_queu = "#!/bin/bash\n"
        launch_queu+= "CMD+=(\"{:}\")\n".format(_fpath)
        create_script(_path = _pque, _name = "launch", _data = launch_queu)

    #
    # Queue script
    #
    if not _local :
        launch_queu = queue_script(_queue)
    else :
        launch_queu = "\n"
        launch_queu+= "module load parallel\n"
        launch_queu+= "\n"
        launch_queu+= "NUMJOBS=16\n"
        launch_queu+= "SECONDS=0\n"
        launch_queu+= "\n"
        launch_queu+= "parallel -j ${NUMJOBS} --progress 'echo \"Running {} ...\"; "
        launch_queu+= "cd {}; ./launch_this > run' ::: ${CMD[@]}\n"
        launch_queu+= "\n"

    launch_queu+= "_duration=$SECONDS\n"
    launch_queu+= "_hours=$(($_duration / 3600))\n"
    launch_queu+= "_diffs=$(($_duration % 3600))\n"
    launch_queu+= "_minut=$(($_diffs / 60))\n"
    launch_queu+= "_secon=$(($_diffs % 60))\n"
    launch_queu+= "\n"
    launch_queu+= "echo \"All Done!\"\n"
    launch_queu+= "\n"
    launch_queu+= "echo \"Time: ${_hours}h ${_minut}m ${_secon}s\"\n"
    launch_queu+= "\n"
    create_script(_path = _pque, _name = "launch", _data = launch_queu)

    _queues     = ["ising", "standard", "preempt", "phi"]
    assert(_queues)
    _queue_time = 1
    if   _batch == "standard" : _queue_time = 5
    elif _batch == "phi"      : _queue_time = 5
    elif _batch == "preempt"  : _queue_time = 2
    elif _batch == "ising"    : _queue_time = 5

    launch_queu = "#!/bin/bash\n"
    launch_queu+= "#SBATCH --partition={:}\n".format(_batch)
    launch_queu+= "#SBATCH --job-name=AS_QUEUE\n"
    launch_queu+= "#SBATCH --error={:}/err.%j\n".format(_pque)
    launch_queu+= "#SBATCH --output={:}/run.%j\n".format(_pque)
    launch_queu+= "#SBATCH --mem-per-cpu=1GB\n"
    launch_queu+= "#SBATCH --time={:}-00:00:00\n".format(_queue_time)
    launch_queu+= "#SBATCH -n 1\n"
    launch_queu+= "\n"
    launch_queu+= "unset TMPDIR\n"
    launch_queu+= "\n"
    launch_queu+= "{:}/launch\n".format(_pque)
    launch_queu+= "\n"
    create_script(_path = _pque, _name = "launch_queue", _data = launch_queu)

    print("Path: {:}".format(_pque))
    print("\nDone, Jobs: {:}!!!".format(_jobs))
#----------------
#
#----------------
def code_build(configs, this = "", cpath = "", fpath = ""):
    
    print("New Config: " + cpath)

    # Create code directory
    path = cpath + "/code"
    if os.path.exists(path) : rmtree(path)
    create_dir(cpath + "/code")
    
    # Copy the header code
    _code = "include"
    _src  = "{:}/{:}".format(this, _code)
    assert(os.path.exists(_src))
    _dst  = "{:}/code/{:}".format(cpath, _code)
    copytree(_src, _dst)
    
    # Copy the src code
    _code = "src"
    _src  = "{:}/{:}".format(this, _code)
    assert(os.path.exists(_src))
    _dst  = "{:}/code/{:}".format(cpath, _code)
    
    if os.path.exists(_dst) : rmtree(_dst)
    copytree(_src, _dst)

    # Create makefile
    # _sa   = ["sa.hh", "structs.hh", "default.hh"]
    # _main = ["sa.hh", "structs.hh", "default.hh"]

    # _code = "#!/bin/bash\n"
    # _code+= "CXX = g++\n"
    # _code+= "CXXFLAGS = -std=c++17 -O3 -Wall -funroll-loops -Iinclude\n"
    # _code+= "TARGET = sa\n"
    # _code+= "\n"
    # _code+= "SRC := $(wildcard src/*.cc)\n"
    # _code+= "OBJ := $(patsubst %.cc,%.o, $(SRC))\n"
    # _code+= "\n"
    # _code+= "all : $(TARGET)\n"
    # _code+= "\n"
    # _code+= "src/sa.o : include/{:}\n".format(" include/".join(_sa))
    # _code+= "src/main.o : include/{:}\n".format(" include/".join(_main))
    # _code+= "\n"
    # _code+= "$(TARGET) : $(OBJ)\n"
    # _code+= "\t@module unload gcc; \\\n"
    # _code+= "\tmodule load gcc/7.3.0; \\\n"
    # _code+= "\t$(CXX) $(CXXFLAGS) -o $@ $^\n"
    # _code+= "\n"
    # _code+= "%.o : %.cc\n"
    # _code+= "\t@module unload gcc; \\\n"
    # _code+= "\tmodule load gcc/7.3.0; \\\n"
    # _code+= "\t$(CXX) $(CXXFLAGS) -DPARAMS=\\\"default.hh\\\" -o $@ -c $<\n"
    # _code+= "\n"
    # _code+= "clean :\n"
    # _code+= "\trm -r $(OBJ) $(TARGET)\n"

    # write_file(cpath + "/code", _code, "Makefile")
    # _file = cpath + "/code/Makefile"
    # os.chmod(_file, os.stat(_file).st_mode | 0o777)

    # # Build
    # cmd = "make -C {:}/code".format(cpath)
    # [out, err] = sp.Popen(cmd, stdout=sp.PIPE, stderr=sp.STDOUT, shell = True).communicate()
    # if not os.path.exists(cpath + "/code/sa") :
    #     print("Compilation failed!")
    #     print("Command: {:}".format(cmd))
    #     print("Error: {:}".format(out))
    #     exit()

#----------------
#
#----------------
def base_config() :
    cfg = {
        "beta1"         : {"flag" : True, "type" : "double",    "value" : "3.0"},
        "beta0"         : {"flag" : True, "type" : "double",    "value" : "0.1"},
        "istop"         : {"flag" : True, "type" : "double",    "value" : "0"},
        "acpt_steps"    : {"flag" : True, "type" : "uint64_t",  "value" : "0"},
        "scale"         : {"flag" : True, "type" : "double",    "value" : "0"},
        "seed"          : {"flag" : True, "type" : "uint64_t",  "value" : "0"},
        "cut_time"      : {"flag" : True, "type" : "double",    "value" : "0"},
        "max_mem"       : {"flag" : True, "type" : "double",    "value" : "0.001"},
        "sweeps"        : {"flag" : True, "type" : "uint64_t",  "value" : "1e3"},
        "debug"         : {"flag" : True, "type" : "bool",      "value" : "false"},
        "dump_ene"      : {"flag" : True, "type" : "bool",      "value" : "false"},
        "track_best"    : {"flag" : True, "type" : "bool",      "value" : "false"},
        "track_spins"   : {"flag" : True, "type" : "bool",      "value" : "false"},
        "trace_energy"  : {"flag" : True, "type" : "bool",      "value" : "false"},
        "count_acpt"    : {"flag" : True, "type" : "bool",      "value" : "false"},
        "bias"          : {"flag" : True, "type" : "bool",      "value" : "false"},
    }
    return cfg
#--------------------
def code_gen(configs) :

    #
    # Sanity checks
    #
    assert(os.path.exists(configs["gset"]["value"]))
    
    #
    # Pre computations
    #
    _sweeps = float(configs["sweeps"]["value"])
    configs["scale"]["value"] = "{:g}".format(
            0 if _sweeps <= 1 else (float(configs["beta1"]["value"]) - float(configs["beta0"]["value"])) / (_sweeps - 1))

    #
    # Read gset header
    #
    cmd = "head -n1 {:}".format(configs["gset"]["value"])
    [out, err] = sp.Popen(cmd, stdout=sp.PIPE, stderr=sp.STDOUT, shell = True).communicate()
    assert(not err)
    out = str(out, "utf-8")
    out = out.strip().split(" ")
    assert(len(out) == 2)
    configs["nodes"] = {"flag" : False, "value" : out[0]}
    configs["edges"] = {"flag" : False, "value" : out[1]}
    
    #
    # Create params header file
    #
    _code = """
#ifndef __DEFAULT_HH__
#define __DEFAULT_HH__

#include <cmath>

/******************************
* The data structure holding *
* BRIM specific parameters   *
******************************/

struct _Params
{
"""
    _tab1 = " " * 4
    _tba2 = " " * 8
    for _name in configs :
        if configs[_name]["flag"] :
            _type = configs[_name]["type"]
            _code += "{:}{:10} {:};\n".format(_tab1, _type, _name)

    _code += "\n{:}constexpr _Params() :\n".format(_tab1)

    for _name in configs :
        if configs[_name]["flag"] :
            _type = configs[_name]["type"]
            _valu = configs[_name]["value"]
            _code += "{:}{:}({:}),\n".format(_tba2, _name, _valu)

    _code = _code[:-2] 
    _code += "\n{:}{{}}\n".format(_tab1)
    _code += "};\n"
    _code += "\n"
    _code += "using Params = struct _Params;\n"
    _code += "\n"
    _code += "#endif\n"

    #
    # Create the header file
    #
    return _code

# --------------
#
# --------------
def write_file(_path, _data, _name) :
    assert(os.path.exists(_path))
    _type = "w"
    _file = open(_path + "/" + _name, _type)
    _file.write(_data)
    _file.close()
# --------------
#
# --------------
def create_dir(path):
    if os.path.exists(path) : return
    os.makedirs(path)
    mode = os.stat(path).st_mode
    os.chmod(path, mode | 0o777)
# --------------
#
# --------------
def read_best_cuts(_this, _work):
    
    # Get the spins info
    _pbst = "{:}/GSET/{:}/summary.txt".format(_this, _work)
    if not os.path.exists(_pbst) : return False
    
    f = open(_pbst, "r")
    d = f.read(); f.close(); assert(len(d) > 0)
    d = d.strip().split('\n');

    # Read header
    h = d[0]
    d = d[1:]

    # Read data into global gset directory
    ret = False
    for l in d:
        l = l.split(" ")
        assert(len(l))
        g = l[0]
        n = l[1]
        e = l[2]
        c = l[3]
        if g not in gset : gset[g] = {}
        if _work not in gset[g] : gset[g][_work] = {}
        gset[g][_work]['ct'] = c
        gset[g][_work]['mm'] = "8"
        gset[g][_work]['no'] = n
        gset[g][_work]['ed'] = e
        ret = True

    return ret
# --------------
#
# --------------
def create_script(_path, _name, _data) :
    _script= "{:}/{:}".format(_path, _name)
    if not os.path.exists(_path) :
        print("Path does not exist, " + _path)
        exit(1)
    _type  = "w" if "bash" in _data else "a"
    _file  = open(_script, _type)
    _file.write(_data)
    _file.close()
    st = os.stat(_script)
    os.chmod(_script, st.st_mode | 0o777)
#---------------
#
# --------------
def queue_script(queue) :
    launch_queu = "\n"
    launch_queu+= "##Function to launch script\n"
    # launch_queu+= "function launch\n"
    # launch_queu+= "{\n"
    # launch_queu+= "    que=$1\n"
    # launch_queu+= "    dir=$2\n"
    # launch_queu+= "    cd $dir\n"
    # launch_queu+= "    sed -i 's/THE_QUEUE/'\"$que\"'/g' launch_this\n"
    # launch_queu+= "    unset TMPDIR\n"
    # launch_queu+= "    sbatch launch_this\n"
    # launch_queu+= "}\n"
    launch_queu+= "\n"
    launch_queu+= "##Function to loop over jobs\n"
    launch_queu+= "idx_all=0\n"
    launch_queu+= "num_jobs=${#CMD[@]}\n"
    launch_queu+= "QU=({:})\n".format("\"" + "\" \"".join(queue) + "\"")
    launch_queu+= "done_flag=0\n"
    launch_queu+= "\n"
    launch_queu+= "function loop_over\n"
    launch_queu+= "{\n"
    launch_queu+= "    _sq=$1\n"
    # launch_queu+= "    re='^[0-9]+$'\n"
    launch_queu+= "    while ((idx_all < num_jobs)); do\n\n"
    launch_queu+= "        idx=0;\n"
    launch_queu+= "        while ((idx<${#QU[@]})); do\n"
    launch_queu+= "            # Get queue name\n"
    launch_queu+= "            q=${QU[$idx]}; idx=$((idx+1));\n\n"
    launch_queu+= "            # Get queue count\n"
    launch_queu+= "            case ${q} in\n"
    launch_queu+= "                *\"reserved\"*) qc=70; p=\"reserved\";;\n"
    launch_queu+= "                *\"phi\"*) qc=35;;\n"
    launch_queu+= "                *\"ising\"*) qc=2000;;\n"
    launch_queu+= "                *) qc=50;;\n"
    launch_queu+= "            esac\n\n"
    launch_queu+= "            # Assign core count\n"
    launch_queu+= "            v=$(squeue -u $USER -p \"$q\" | awk '{if(NR>1)print}' | wc -l | awk -v qc=${qc} '{print qc-$1}');\n\n"
    # launch_queu+= "            # Continue if v is not a number\n"
    # launch_queu+= "            if ! [[ $v =~ $re ]] ; then\n"
    # launch_queu+= "                echo \".\";\n"
    # launch_queu+= "                continue;\n"
    # launch_queu+= "            fi\n\n"
    launch_queu+= "            # Launch jobs\n"
    launch_queu+= "            if [ \"$v\" -gt \"0\" ]; then\n"
    launch_queu+= "                echo \"State: ${_sq}, Empty: $v, Queue: $q, Index: $idx_all of $num_jobs\";\n"
    launch_queu+= "                curr=0;\n"
    launch_queu+= "                while ((curr < $v)); do\n"
    launch_queu+= "                    dir=${CMD[$idx_all]}\n"
    launch_queu+= "                    printf \"State: %s, Idx: %5d of %5d, Queue: %6s, Path: %s\\n\" ${_sq} $((idx_all+1)) ${num_jobs} \"${q}\" ${dir}\n"
    # launch_queu+= "                    launch \"$q\" $dir\n"
    launch_queu+= "                    cd $dir\n"
    launch_queu+= "                    sed -i 's/THE_QUEUE/'\"${q}\"'/g' launch_this\n"
    launch_queu+= "                    unset TMPDIR\n"
    launch_queu+= "                    CMD=\"sbatch launch_this\"\n"
    launch_queu+= "                    if ! eval ${CMD}; then\n"
    launch_queu+= "                        echo \"Job ${idx_all} launch failed!\"\n"
    launch_queu+= "                        while ! eval ${CMD}; do printf \".\"; done\n"
    launch_queu+= "                    fi\n\n"
    launch_queu+= "                    curr=$((curr+1))\n"
    launch_queu+= "                    idx_all=$((idx_all+1))\n"
    launch_queu+= "                    if [ $idx_all -ge $num_jobs ]; then\n"
    launch_queu+= "                        done_flag=1\n"
    launch_queu+= "                        break;\n"
    launch_queu+= "                    fi\n"
    launch_queu+= "                done\n"
    launch_queu+= "            fi\n"
    launch_queu+= "            if [ $done_flag -eq 1 ]; then break; fi\n"
    launch_queu+= "        done\n"
    launch_queu+= "        if [ $done_flag -eq 1 ]; then break; fi\n"
    launch_queu+= "        if [ \"${_sq}\" == \"Pending\" ]; then sleep 10; fi\n"
    launch_queu+= "    done\n"
    launch_queu+= "}\n"
    launch_queu+= "\n"
    launch_queu+= "SECONDS=0\n"
    launch_queu+= "\n"
    launch_queu+= "#For rest of the time, check my pending jobs in all queues\n"
    launch_queu+= "loop_over \"Pending\"\n"
    launch_queu+= "\n"

    return launch_queu
# --------------
#
# --------------
if __name__ == "__main__" : process()

