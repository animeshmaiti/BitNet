#include <iostream>
#include <cstdio>  // for popen and pclose
#include <memory>
#include <string>
#include <unistd.h> // for chdir, getcwd
#include <limits.h> // for PATH_MAX

std::string runInference(const std::string& model_path, const std::string& prompt, int num_tokens, float temp) {
    static bool directoryChanged = false;  // Static flag to track if the directory has been changed
    char currentDir[PATH_MAX];  // Buffer to store the current directory

    // Get the current working directory
    if (getcwd(currentDir, sizeof(currentDir)) != nullptr) {
        std::string currentDirStr(currentDir);
        if (currentDirStr.find("/server") != std::string::npos && !directoryChanged) {
            // If not already in BitNet and directory hasn't been changed yet
            if (chdir("../") != 0) {  // "../" will move up from server/ to BitNet/
                std::cerr << "Failed to change directory to BitNet/." << std::endl;
                return "Error: Failed to change directory.";
            }
            directoryChanged = true;  // Mark the directory as changed
        }
    } else {
        std::cerr << "Failed to get current directory." << std::endl;
        return "Error: Failed to get current directory.";
    }

    // Construct the command string
    std::string command = "python run_inference.py -m " + model_path + 
                          " -p \"" + prompt +"\\nAnswer:"+ "\" -n " + std::to_string(num_tokens) + 
                          " -temp " + std::to_string(temp) + " 2>&1";  // Redirect stderr to stdout

    // Open the process for reading
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        std::cerr << "Failed to run inference command." << std::endl;
        return "Error: Failed to run command.";
    }

    // Read the output from the command
    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
    std::cout << "Full Output: " << result << std::endl;
    // Find the answer part in the output
    size_t pos = result.find("Answer:");
    if (pos != std::string::npos) {
        size_t end_pos = result.find("\n", pos);
        std::string answer = result.substr(pos+7, end_pos - (pos+7));
        return answer;
    } else {
        return "Answer not found in output.";
    }
}
