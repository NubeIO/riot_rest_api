//
// Created by craig on 8/11/2024.
//

#ifndef EDGE_ROUTES_HPP_
#define EDGE_ROUTES_HPP_

#include "crow.h"
#include "engine_service.hpp"
#include "open_api_builder.hpp"

class EdgeRoutes {
 public:
  static void registerRoutes(crow::App<>& app, EngineService& engineService, OpenAPIBuilder& apiBuilder) {
    setupSwaggerDocs(apiBuilder);
    setupRoutes(app, engineService);
  }

 private:
  static void setupSwaggerDocs(OpenAPIBuilder& apiBuilder) {
    auto edgeSchema = OpenAPIBuilder::createObjectSchema({
                                                             {"fromInstanceId", "integer"},
                                                             {"toInstanceId", "integer"},
                                                             {"outName", "string"},
                                                             {"inName", "string"}
                                                         });

    std::vector<crow::json::wvalue> edgeParameters = {
        OpenAPIBuilder::createParameter(
            "edgeId",
            "path",
            true,
            "integer",
            "ID of the edge to remove"
        )
    };

    apiBuilder.addEndpoint(
        "/api/edges",
        "POST",
        "Add a new edge",
        edgeSchema,
        {{"200", {
            {"description", "Edge added successfully"},
            {"content", {
                {"application/json", {
                    {"schema", OpenAPIBuilder::createObjectSchema({
                                                                      {"edgeId", "integer"}
                                                                  })}
                }}
            }}
        }}}
    );

    apiBuilder.addEndpoint(
        "/api/edges/{edgeId}",
        "DELETE",
        "Remove an edge",
        crow::json::wvalue(),
        {{"204", {
            {"description", "Edge removed successfully"}
        }}},
        edgeParameters
    );
  }

  static void setupRoutes(crow::App<>& app, EngineService& engineService) {
    CROW_ROUTE(app, "/api/edges")
        .methods("POST"_method)
            ([&engineService](const crow::request& req) {
              auto x = crow::json::load(req.body);
              if (!x)
                return crow::response(400, "Invalid JSON");

              try {
                auto edge_id = engineService.AddEdge(
                    x["fromInstanceId"].u(),
                    x["toInstanceId"].u(),
                    x["outName"].s(),
                    x["inName"].s()
                );

                crow::json::wvalue response;
                response["edgeId"] = edge_id;

                return crow::response(response);
              } catch (const std::exception& e) {
                return crow::response(500, e.what());
              }
            });

    CROW_ROUTE(app, "/api/edges/<uint>")
        .methods("DELETE"_method)
            ([&engineService](uint32_t edgeId) {
              try {
                engineService.RemoveEdge(edgeId);
                return crow::response(204); // No content
              } catch (const std::exception& e) {
                return crow::response(500, e.what());
              }
            });
  }
};

#endif //EDGE_ROUTES_HPP_
