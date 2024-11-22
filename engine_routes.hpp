//
// Created by craig on 8/11/2024.
//

#ifndef ENGINE_ROUTES_HPP22_
#define ENGINE_ROUTES_HPP22_


#include "crow.h"
#include "engine_service.hpp"
#include "open_api_builder.hpp"

class EngineRoutes {
 public:
  static void registerRoutes(crow::App<>& app, EngineService& engineService, OpenAPIBuilder& apiBuilder) {
    setupSwaggerDocs(apiBuilder);
    setupRoutes(app, engineService);
  }

 private:
  static void setupSwaggerDocs(OpenAPIBuilder& apiBuilder) {
    apiBuilder.addEndpoint(
        "/api/flow",
        "GET",
        "Get flow JSON data",
        crow::json::wvalue(),  // no request body
        {{"200", {
            {"description", "Flow JSON data retrieved successfully"},
            {"content", {
                {"application/json", {
                    {"schema", {
                        {"type", "object"},
                        {"description", "Flow graph JSON structure"}
                    }}
                }}
            }}
        }}}
    );
  }


  static void setupRoutes(crow::App<>& app, EngineService& engineService) {
    CROW_ROUTE(app, "/api/flow")
        .methods("GET"_method)
            ([&engineService]() {
              try {
                std::string jsonData = engineService.GetFlowJson();

                // Since the response is already JSON text, we need to parse it
                auto parsedJson = crow::json::load(jsonData);
                if (!parsedJson) {
                  return crow::response(500, "Invalid JSON received from engine");
                }

                return crow::response(jsonData);
              } catch (const std::exception &e) {
                return crow::response(500, e.what());
              }
            });
  }
};

#endif //ENGINE_ROUTES_HPP_
