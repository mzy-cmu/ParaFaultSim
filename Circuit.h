#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <string>

using namespace std;

struct Gate {
    string type;                // Gate type (e.g., INPUT, AND, OR, NOT)
    vector<int> inputs;         // Input signal id
};

enum GATETYPE {
    BUFF,
    NOT,
    AND,
    NAND,
    OR,
    NOR,
    XOR,
    XNOR
};

int addSignal(const string name,
              vector<string> &signals,
              unordered_map<string, int> &signal_map);
void parseGate(const string line,
               vector<string> &signals,
               unordered_map<string, int> &signal_map,
               vector<Gate> &gates,
               vector<vector<int>> &dependent_signals,
               vector<int> &dependency_degree,
               vector<int> &gate_type,
               int *num_gate_input,
               vector<vector<int>> &gate_input,
               vector<int> &gate_input_size,
               vector<int> &gate_input_startidx);
void parseInputOutput(const string line,
                      const bool isOutput,
                      vector<int> &inputs,
                      vector<int> &outputs,
                      vector<string> &signals,
                      unordered_map<string, int> &signal_map,
                      vector<Gate> &gates,
                      vector<int> &dependency_degree);
void parseISCAS89(const string filename,
                  vector<int> &inputs,
                  vector<int> &outputs,
                  vector<string> &signals,
                  unordered_map<string, int> &signal_map,
                  vector<Gate> &gates,
                  vector<vector<int>> &dependent_signals,
                  vector<int> &dependency_degree,
                  vector<int> &gate_type,
                  int *num_gate_input,
                  vector<vector<int>> &gate_input,
                  vector<int> &gate_input_size,
                  vector<int> &gate_input_startidx);
int parseTestcase(const string &testcase_filename,
                  vector<vector<bool>> &tests,
                  size_t num_inputs);
vector<int> popSignals(vector<bool> &check_todo,
                       vector<vector<int>> &dependent_signals,
                       vector<int> &dependency_degree);

#endif
