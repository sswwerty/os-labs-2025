// child.cpp
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <output_file>\n";
        return 1;
    }

    std::string output_file = argv[1];
    std::ofstream out(output_file);
    if (!out.is_open()) {
        std::cerr << "Cannot open output file: " << output_file << "\n";
        return 1;
    }

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        float num, sum = 0.0f;
        int count = 0;

        while (iss >> num) {
            sum += num;
            ++count;
        }

        if (count > 0) {
            out << "Sum: " << sum << "\n";
            out.flush(); // чтобы сразу записалось
        }
    }

    out.close();
    return 0;
}