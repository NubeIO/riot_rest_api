//
// Created by craig on 11/11/2024.
//

#ifndef PACKAGE_ROUTES_HPP_
#define PACKAGE_ROUTES_HPP_

#include "crow.h"
#include "engine_service.hpp"
#include "open_api_builder.hpp"

class PackageRoutes {
 public:
  static void registerRoutes(crow::App<crow::CORSHandler>& app, EngineService& engineService, OpenAPIBuilder& apiBuilder) {
    setupSwaggerDocs(apiBuilder);
    setupRoutes(app, engineService);
  }

 private:
  static void setupSwaggerDocs(OpenAPIBuilder& apiBuilder) {
    // GET available packages endpoint
    apiBuilder.addEndpoint(
        "/api/packages",
        "GET",
        "Get all available packages",
        crow::json::wvalue(),
        {{"200", {
            {"description", "List of available packages"},
            {"content", {
                {"application/json", {
                    {"schema", {
                        {"type", "array"},
                        {"items", OpenAPIBuilder::createObjectSchema({
                                                                         {"packageId", "integer"},
                                                                         {"packageName", "string"},
                                                                         {"packageVersion", "string"}
                                                                     })}
                    }}
                }}
            }}
        }}}
    );

    // GET package JSON endpoint
    std::vector<crow::json::wvalue> packageParameters = {
        OpenAPIBuilder::createParameter(
            "packageId",
            "path",
            true,
            "integer",
            "Package ID to fetch JSON for"
        )
    };

    apiBuilder.addEndpoint(
        "/api/packages/{packageId}/json",
        "GET",
        "Get package JSON schema",
        crow::json::wvalue(),
        {{"200", {
            {"description", "Package JSON schema"},
            {"content", {
                {"application/json", {
                    {"schema", {
                        {"type", "object"}
                    }}
                }}
            }}
        }}},
        packageParameters
    );
  }

  static void setupRoutes(crow::App<crow::CORSHandler>& app, EngineService& engineService) {
    CROW_ROUTE(app, "/api/packages")
        .methods("GET"_method)
            ([&engineService]() {
              try {
                capnp::EzRpcClient client(kj::str("unix:", "/tmp/engine-socket").cStr());
                Engine::Client engine = client.getMain<Engine>();

                auto request = engine.getAvailablePackagesRequest();
                auto response1 = request.send().wait(client.getWaitScope());

                auto packages = response1.getAvailablePackages();

                crow::json::wvalue response;
                for (size_t i = 0; i < packages.size(); i++) {
                  response[i]["packageId"] = packages[i].getPackageId();
                  response[i]["packageName"] = packages[i].getPackageName().cStr();
                  response[i]["packageVersion"] = packages[i].getPackageVersion().cStr();
                }

                return crow::response(response);
              } catch (const std::exception& e) {
                return crow::response(500, e.what());
              }
            });

    CROW_ROUTE(app, "/api/packages/<uint>/json")
        .methods("GET"_method)
            ([&engineService](uint32_t packageId) {
              try {
                std::string jsonData = engineService.GetPackageJson(packageId);
                return crow::response(jsonData);
              } catch (const std::exception& e) {
                return crow::response(500, e.what());
              }
            });


  }
};

#endif //PACKAGE_ROUTES_HPP_
