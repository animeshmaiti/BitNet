#include "crow.h"
#include <nlohmann/json.hpp> // for JSON parsing
#include <vector>
#include "runInference.h"   // Include the runInference header

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/prompt")
        .methods(crow::HTTPMethod::POST)(
            [](const crow::request &req)
            {
                // Parse incoming JSON
                auto body = nlohmann::json::parse(req.body, nullptr, false);
                nlohmann::json response;

                if (body.is_discarded() || !body.contains("prompt") || !body.contains("model_path"))
                {
                    response["status"] = "error";
                    response["message"] = "Invalid JSON format or missing 'prompt' or 'model_path'";
                    return crow::response{400, response.dump()}; // Bad Request
                }

                // Extract the prompt and model_path
                auto prompt = body["prompt"];
                auto model_path = body["model_path"];
                int num_tokens = body.value("num_tokens", 6); // Default to 6 tokens if not provided
                float temp = body.value("temp", 0.0);         // Default temperature is 0.0 if not provided

                if (!prompt.is_string() || !model_path.is_string())
                {
                    response["status"] = "error";
                    response["message"] = "'prompt' and 'model_path' should be strings";
                    return crow::response{400, response.dump()}; // Bad Request
                }

                // Call runInference function with provided parameters
                std::string answer = runInference(model_path.get<std::string>(), prompt.get<std::string>(), num_tokens, temp);

                if (!answer.empty())
                {
                    response["status"] = "success";
                    response["received_answer"] = answer;
                }
                else
                {
                    response["status"] = "error";
                    response["received_answer"] = "No Answer Found";
                }

                return crow::response{response.dump()};
            });

    app.port(8080).multithreaded().run();
}
