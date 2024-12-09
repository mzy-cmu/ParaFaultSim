#include "Circuit.h"

// Add a signal to the circuit, return id
int addSignal(const string name,
              vector<string> &signals,
              unordered_map<string, int> &signal_map) {
    // Avoid redundent add
    if (signal_map.find(name) != signal_map.end()) {
        return signal_map[name];
    }
    int id = signals.size();
    signals.push_back(name);
    signal_map[name] = id;
    return id;
}

void parseGate(const string line,
               vector<string> &signals,
               unordered_map<string, int> &signal_map,
               vector<Gate> &gates,
               vector<vector<int>> &dependent_signals,
               vector<int> &dependency_degree,
               vector<GATETYPE> &gate_type,
               int *num_gate_input,
               vector<vector<int>> &gate_input,
               vector<int> &gate_input_size,
               vector<int> &gate_input_startidx) {
    string output, type, input_list;
    stringstream ss(line);

    // Output signal
    getline(ss, output, '=');
    output.erase(std::remove(output.begin(), output.end(), ' '), output.end());
    int output_id = addSignal(output, signals, signal_map);

    // Gate type and inputs
    getline(ss, type, '(');
    type.erase(std::remove(type.begin(), type.end(), ' '), type.end());
    getline(ss, input_list, ')');

    // Input signals
    vector<int> input_ids;
    stringstream inputStream(input_list);
    string input;
    while (getline(inputStream, input, ',')) {
        input.erase(std::remove(input.begin(), input.end(), ' '), input.end());
        input_ids.push_back(addSignal(input, signals, signal_map));
    }

    // Update CUDA signals
    gates.resize(signals.size());
    gate_type.resize(signals.size());
    gate_input.resize(signals.size());
    gate_input_size.resize(signals.size());
    gate_input_startidx.resize(signals.size());

    GATETYPE type_enum;
    if (type == "BUFF") {
        type_enum = BUFF;
    } else if (type == "NOT") {
        type_enum = NOT;
    } else if (type == "AND") {
        type_enum = AND;
    } else if (type == "NAND") {
        type_enum = NAND;
    } else if (type == "OR") {
        type_enum = OR;
    } else if (type == "NOR") {
        type_enum = NOR;
    } else if (type == "XOR") {
        type_enum = XOR;
    } else if (type == "XNOR") {
        type_enum = XNOR;
    }
    gate_type[output_id] = type_enum;

    Gate gate = {type_enum, input_ids};
    gates[output_id] = gate;

    gate_input[output_id] = input_ids;
    gate_input_size[output_id] = input_ids.size();
    *num_gate_input += input_ids.size();
    gate_input_startidx[output_id] = output_id == 0 ? 0 : gate_input_startidx[output_id - 1] + gate_input_size[output_id - 1];

    dependency_degree.resize(signals.size(), 0);
    dependency_degree[output_id] = input_ids.size();
    dependent_signals.resize(signals.size());
    for (size_t i = 0; i < input_ids.size(); i++) {
        dependent_signals[input_ids[i]].push_back(output_id);
    }
}

void parseInputOutput(const string line,
                      const bool isOutput,
                      vector<int> &inputs,
                      vector<int> &outputs,
                      vector<string> &signals,
                      unordered_map<string, int> &signal_map,
                      vector<Gate> &gates,
                      vector<int> &dependency_degree,
                      vector<GATETYPE> &gate_type,
                      vector<vector<int>> &gate_input,
                      vector<int> &gate_input_size,
                      vector<int> &gate_input_startidx) {
    string name;
    stringstream ss(line);
    getline(ss, name, '('); // Skip "INPUT" "OUTPUT"
    getline(ss, name, ')');

    int id = addSignal(name, signals, signal_map);
    if (!isOutput) {
        inputs.push_back(id);
        Gate gate = {INPUT, {}};
        gates.push_back(gate);

        gate_type.push_back(INPUT);
        gate_input.push_back({});
        gate_input_size.push_back(0);
        gate_input_startidx.push_back(0);

        dependency_degree.push_back(0);
    }
    else
        outputs.push_back(id);
}

// Parse ISCAS89 and create circuit data structure
void parseISCAS89(const string filename,
                  vector<int> &inputs,
                  vector<int> &outputs,
                  vector<string> &signals,
                  unordered_map<string, int> &signal_map,
                  vector<Gate> &gates,
                  vector<vector<int>> &dependent_signals,
                  vector<int> &dependency_degree,
                  vector<GATETYPE> &gate_type,
                  int *num_gate_input,
                  vector<vector<int>> &gate_input,
                  vector<int> &gate_input_size,
                  vector<int> &gate_input_startidx) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Error: Could not open file " + filename);
    }

    string line;
    while (getline(file, line)) {
        if (line.empty())
            continue;
        // Parse IO
        if (line.find("INPUT") == 0) {
            parseInputOutput(line, false, inputs, outputs, signals, signal_map, gates, dependency_degree, gate_type, gate_input, gate_input_size, gate_input_startidx);
        } else if (line.find("OUTPUT") == 0) {
            parseInputOutput(line, true, inputs, outputs, signals, signal_map, gates, dependency_degree, gate_type, gate_input, gate_input_size, gate_input_startidx);
        // Parse gates
        } else if (line.find("=") != string::npos) {
            parseGate(line, signals, signal_map, gates, dependent_signals, dependency_degree, gate_type, num_gate_input, gate_input, gate_input_size, gate_input_startidx);
        }
    }

    file.close();
    return;
}

// Parse test case
int parseTestcase(const string &testcase_filename,
                  vector<vector<bool>> &tests,
                  size_t num_inputs) {
    ifstream file(testcase_filename);
    if (!file.is_open()) {
        throw runtime_error("Error: Could not open file " + testcase_filename);
    }

    string line;
    int test_id;
    int num_testcase = 0;

    while (getline(file, line)) {
        // Find the colon to split the test_id and binary string
        size_t colon_pos = line.find(':');
        if (colon_pos == string::npos) {
            throw runtime_error("Invalid format in line: " + line);
        }

        // Extract test_id and binary string
        try {
            test_id = stoi(line.substr(0, colon_pos));
        } catch (const invalid_argument &) {
            throw runtime_error("Invalid test id in line: " + line);
        }

        string binary_str = line.substr(colon_pos + 1);
        binary_str.erase(0, binary_str.find_first_not_of(" \t")); // Trim leading spaces

        // Validate the binary string length
        if (binary_str.size() != num_inputs) {
            throw runtime_error("Error: Testcase " + to_string(test_id) + " has wrong number of inputs");
        }

        // Convert binary string to boolean vector
        vector<bool> test_case;
        for (char ch : binary_str) {
            if (ch == '1') {
                test_case.push_back(true);
            } else if (ch == '0') {
                test_case.push_back(false);
            } else {
                throw runtime_error("Invalid character in binary string: " + ch);
            }
        }

        // Resize the tests vector to accommodate the test_id
        if (tests.size() <= static_cast<size_t>(test_id - 1)) {
            tests.resize(test_id);
        }

        // Store the test case
        tests[test_id - 1] = test_case;
        num_testcase++;
    }

    return num_testcase;
}

// Identify ready signals in this batch, resolve dependency
vector<int> popSignals(vector<bool> &check_todo,
                       vector<vector<int>> &dependent_signals,
                       vector<int> &dependency_degree) {
    vector<int> signals_todo;

    // Enqueue all signals with in-degree 0 that are not proccessed
    for (size_t signalIn = 0; signalIn < dependency_degree.size(); signalIn++) {
        if (!check_todo[signalIn] && dependency_degree[signalIn] == 0) {
            signals_todo.push_back(signalIn);
            check_todo[signalIn] = true;
        }
    }

    // Decrease in_degree of dependent signals
    for (size_t j = 0; j < signals_todo.size(); j++) {
        for (size_t i = 0; i < dependent_signals[signals_todo[j]].size(); i++) {
            int signalOut = dependent_signals[signals_todo[j]][i];
            dependency_degree[signalOut]--;
        }
    }

    return signals_todo;
}