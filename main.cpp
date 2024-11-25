#include "crow.h"
#include <crow/middlewares/cors.h>
#include "open_api_builder.hpp"
#include "swagger_ui.hpp"
#include "engine_service.hpp"
#include "node_routes.hpp"
#include "edge_routes.hpp"
#include "package_routes.hpp"
#include "engine_routes.hpp"

const char *SOCKET_PATH = "/tmp/engine-socket";
int main() {

  crow::App<crow::CORSHandler> app;
  app.loglevel(crow::LogLevel::INFO);
  app.concurrency(1);

  // Add CORS headers directly in a catchall route
  auto& cors = app.get_middleware<crow::CORSHandler>();
  cors
      .global()
      .origin("*")  // Allow all origins for testing
      .methods("GET"_method, "POST"_method, "PUT"_method, "DELETE"_method, "OPTIONS"_method)
      .headers("Content-Type", "Authorization");



  EngineService engineService;
  OpenAPIBuilder apiBuilder;

  NodeRoutes::registerRoutes(app, engineService, apiBuilder);
  EdgeRoutes::registerRoutes(app, engineService, apiBuilder);
  PackageRoutes::registerRoutes(app, engineService, apiBuilder);
  EngineRoutes::registerRoutes(app, engineService, apiBuilder);


  // Your existing Swagger routes
  CROW_ROUTE(app, "/api/v1/swagger")
      .methods("GET"_method)
          ([&apiBuilder](const crow::request& req){
            // Get the host from request headers
            std::string host = req.get_header_value("Host");
            std::string scheme = "http://";  // or "https://" if you're using SSL

            // Update the server URL dynamically
            apiBuilder.updateServerUrl(scheme + host);

            return apiBuilder.getSchema();
          });

  CROW_ROUTE(app, "/swagger")
      ([]{
        return crow::response(swagger::get_html());
      });

  CROW_ROUTE(app, "/debug")
      ([]{
        std::ifstream file("../debug/debug_graph.html");
        if (!file.is_open()) {
          return crow::response(404, "File not found");
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        crow::response resp(content);
        resp.set_header("Content-Type", "text/html");
        return resp;
      });

  CROW_ROUTE(app, "/graph.svg")
      ([]{
        std::ifstream file("../debug/graph.svg");
        if (!file.is_open()) {
          return crow::response(404, "SVG file not found");
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        crow::response resp(content);
        resp.set_header("Content-Type", "image/svg+xml");  // Important for SVG files
        return resp;
      });

  app.port(1668).run();
  return 0;
};