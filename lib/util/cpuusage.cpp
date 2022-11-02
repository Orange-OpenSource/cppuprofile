// Software Name : uprofile
// SPDX-FileCopyrightText: Copyright (c) 2022 Orange
// SPDX-License-Identifier: BSD
//
// This software is distributed under the BSD License;
// see the LICENSE file for more details.
//
// Author: Cédric CHEDALEUX <cedric.chedaleux@orange.com> et al.

int get_number_cpu_cores()
{
    printf("get_number_cpu_cores\n");
    int nb_cores = 0;
    ifstream meminfo("/proc/cpuinfo");
    string str;
    while (std::getline(meminfo, str)) {
        printf("info = %s\n", str.c_str());
        if (str.rfind("processor", 0) == 0)
        {
            nb_cores++;
        }
    }
    printf("nb cores: 4\n");
    return nb_cores;
}

size_t* initTime()
{
   printf("initTime\n");
   int nbCPUCores = get_number_cpu_cores();
   size_t *arr = new size_t(nbCPUCores);
   for (int i = 0; i < nbCPUCores; ++i)
       arr[i] = 0;
   printf("initTime: return\n");
   return arr;
}

static int nb_cpu_cores = get_number_cpu_cores();
static size_t cpuPreviousIdleTime[4] = { 0, 0, 0, 0 }; //initTime();
static size_t cpuPreviousTotalTime[4] = { 0, 0, 0, 0 }; //initTime();


std::vector<size_t> get_cpu_times(int cpu_index) {
    printf("get_cpu_times\n");
    string line;
    string cpu_name("cpu");
    cpu_name += to_string(cpu_index);
    ifstream proc_stat("/proc/stat");
    vector<size_t> times;
    while (std::getline(proc_stat, line)) {
        if (line.rfind(cpu_name, 0) == 0) {
            //TRACE_D << "CCH:" << line.c_str();
            stringstream ls(line);
            ls.ignore(5, ' '); // Skip the 'cpu<index>' prefix.
            for (size_t time; ls >> time; times.push_back(time));
            return times;
        }
    }
    return times;
}


bool get_cpu_times(int cpu_index, size_t &idle_time, size_t &total_time) {
    printf("get_cpu_times 2\n");
    const vector<size_t> cpu_times = get_cpu_times(cpu_index);
    //TRACE_D << "nb_items:" << cpu_times.size();
    //for (size_t i=0; i < cpu_times.size(); ++i)
    //    TRACE_D << cpu_times[i];
    if (cpu_times.size() < 4)
        return false;
    idle_time = cpu_times[3];
    total_time = accumulate(cpu_times.begin(), cpu_times.end(), 0);
    return true;
}

vector<float> Profiler::getCpuUtilization()
{
    printf("getCpuUtilization\n");
    vector<float> usages;
    for (int i=0; i < nb_cpu_cores; ++i)
    {
        // CPU load is an average value : TO CHECK
        size_t idle_time, total_time;
        get_cpu_times(i, idle_time, total_time);

        //TRACE_D << "CCH: cpu" << i << " idle=" << idle_time << ", total=" << total_time;
        const float idle_time_delta = idle_time - cpuPreviousIdleTime[i];
        const float total_time_delta = total_time - cpuPreviousTotalTime[i];
        const float utilization = 100.0 * (1.0 - idle_time_delta / total_time_delta);
        cpuPreviousIdleTime[i] = idle_time;
        cpuPreviousTotalTime[i] = total_time;
        usages.push_back(utilization);
    }
    return usages;
}