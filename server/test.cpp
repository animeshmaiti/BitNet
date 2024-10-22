
#include <iostream>
#include <cstdio>  // for popen and pclose
#include <memory>
#include <string>
#include <unistd.h> // for chdir

std::string runInference(const std::string& model_path, const std::string& prompt, int num_tokens, float temp) {
    // Change the working directory to BitNet/ where the command should run
    if (chdir("../") != 0) {  // "../" will move up from server/ to BitNet/
        std::cerr << "Failed to change directory to BitNet/." << std::endl;
        return "Error: Failed to change directory.";
    }

    // Construct the command string
    std::string command = "python run_inference.py -m " + model_path + 
                          " -p \"" + prompt + "\" -n " + std::to_string(num_tokens) + 
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
    // std::cout << "Full Output: " << result << std::endl;
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


int main()
{
    // Example usage of the function
    std::string model_path = "models/Llama3-8B-1.58-100B-tokens/ggml-model-i2_s.gguf";
    std::string prompt = "A, B, C, D and E are sitting on a bench. A is sitting next to B, C is sitting next to D, D is not sitting with E who is on the left end of the bench. C is on the second position from the right. A is to the right of B and E. A and C are sitting together. In which position A is sitting ? Between B and D\\n Between B and C\\n Between E and D\\n Between C and E\\nAnswer:"; // Your prompt
    int num_tokens = 6;
    float temp = 0.0;

    std::string output = runInference(model_path, prompt, num_tokens, temp);

    // Display the extracted answer
    std::cout << "Extracted Answer:" << output << std::endl;

    return 0;
}
