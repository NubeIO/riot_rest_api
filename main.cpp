#include "crow.h"
#include <crow/middlewares/cors.h>
#include "open_api_builder.hpp"
#include "swagger_ui.hpp"
#include "engine_service.hpp"
#include "node_routes.hpp"
#include "edge_routes.hpp"
#include "package_routes.hpp"

const char *SOCKET_PATH = "/tmp/engine-socket";
int main() {

  crow::App<> app;
  app.loglevel(crow::LogLevel::INFO);
  app.concurrency(1);

  EngineService engineService;
  OpenAPIBuilder apiBuilder;

  NodeRoutes::registerRoutes(app, engineService, apiBuilder);
  EdgeRoutes::registerRoutes(app, engineService, apiBuilder);
  PackageRoutes::registerRoutes(app, engineService, apiBuilder);


  // Your existing Swagger routes
  CROW_ROUTE(app, "/api/v1/swagger")
      .methods("GET"_method)
          ([&apiBuilder]{
            return apiBuilder.getSchema();
          });

  CROW_ROUTE(app, "/swagger")
      ([]{
        return crow::response(swagger::get_html());
      });

  app.port(8080).run();
  return 0;
}