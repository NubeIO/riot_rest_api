//
// Created by craig on 8/11/2024.
//

#ifndef ENGINE_ROUTES_HPP_
#define ENGINE_ROUTES_HPP_


#include "crow.h"
#include "engine_service.hpp"
#include "open_api_builder.hpp"

class NodeRoutes {
 public:
  static void registerRoutes(crow::App<>& app, EngineService& engineService, OpenAPIBuilder& apiBuilder) {
    setupSwaggerDocs(apiBuilder);
    setupRoutes(app, engineService);
  }

 private:
  static void setupSwaggerDocs(OpenAPIBuilder& apiBuilder) {
    auto addNodeDetailsSchema = OpenAPIBuilder::createObjectSchema({
                                                                    {"nodeId", "integer"},
                                                                    {"packageId", "integer"},
                                                                    {"parentId", "integer"},
                                                                    {"posX", "integer"},
                                                                    {"posY", "integer"},
                                                                });

    auto updateNodeDetailsSchema = OpenAPIBuilder::createObjectSchema({
                                                                       {"instanceId", "integer"},
                                                                       {"posX", "integer"},
                                                                       {"posY", "integer"},
                                                                   });

    std::vector<crow::json::wvalue> nodeParameters = {
        OpenAPIBuilder::createParameter(
            "instanceId",
            "path",
            true,
            "integer",
            "Instance ID of the node to remove"
        )
    };

    apiBuilder.addEndpoint(
        "/api/nodes",
        "POST",
        "Add a new node",
        addNodeDetailsSchema,
        {{"200", {
            {"description", "Node added successfully"},
            {"content", {
                {"application/json", {
                    {"schema", OpenAPIBuilder::createObjectSchema({
                                                                      {"instanceId", "integer"},
                                                                      {"name", "string"}
                                                                  })}
                }}
            }}
        }}}
    );

    apiBuilder.addEndpoint(
        "/api/nodes",
        "PUT",
        "Update node position",
        updateNodeDetailsSchema,
        {{"200", {
            {"description", "Node position updated successfully"},
            {"content", {
                {"application/json", {
                    {"schema", OpenAPIBuilder::createObjectSchema({
                                                                      {"instanceId", "integer"},
                                                                      {"name", "string"}
                                                                  })}
                }}
            }}
        }}}
    );



    apiBuilder.addEndpoint(
        "/api/nodes/{instanceId}",
        "DELETE",
        "Remove a node",
        crow::json::wvalue(),
        {{"200", {
            {"description", "Node removed successfully"},
            {"content", {
                {"application/json", {
                    {"schema", OpenAPIBuilder::createObjectSchema({
                                                                      {"instanceId", "integer"}
                                                                  })}
                }}
            }}
        }}},
        nodeParameters
    );

    apiBuilder.addEndpoint(
        "/api/nodes",
        "GET",
        "Get all nodes",
        crow::json::wvalue(),  // no request body
        {{"200", {
            {"description", "List of all nodes"},
            {"content", {
                {"application/json", {
                    {"schema", {
                        {"type", "array"},
                        {"items", OpenAPIBuilder::createObjectSchema({
                                                                         {"instanceId", "integer"},
                                                                         {"nodeName", "string"},
                                                                         {"inputs", "array"},
                                                                         {"outputs", "array"}
                                                                     })}
                    }}
                }}
            }}
        }}}
    );
  }

  static crow::json::wvalue convertFlexValueToJson(const FlexValueCap::Reader& flex) {
    crow::json::wvalue value;

    if (flex.isIntVal()) {
      value = flex.getIntVal();
    } else if (flex.isUintVal()) {
      value = flex.getUintVal();
    } else if (flex.isBoolVal()) {
      value = flex.getBoolVal();
    } else if (flex.isDoubleVal()) {
      // Convert double to string to preserve precision
      std::ostringstream ss;
      ss << std::fixed << std::setprecision(3); // or whatever precision you need
      ss << flex.getDoubleVal();
      value = ss.str();
    } else if (flex.isStringVal()) {
      value = flex.getStringVal().cStr();
    }

    return value;
  }

  static crow::json::wvalue convertIOToJson(const IO::Reader& io) {
    crow::json::wvalue json;
    json["name"] = io.getName().cStr();
    json["value"] = convertFlexValueToJson(io.getValue());
    return json;
  }

  static crow::json::wvalue convertNodeToJson(const Node::Reader& node) {
    crow::json::wvalue json;
    json["instanceId"] = node.getInstanceId();
    json["nodeName"] = node.getNodeName().cStr();

    auto inputs = node.getInputs();
    for (size_t i = 0; i < inputs.size(); i++) {
      json["inputs"][i] = convertIOToJson(inputs[i]);
    }

    auto outputs = node.getOutputs();
    for (size_t i = 0; i < outputs.size(); i++) {
      json["outputs"][i] = convertIOToJson(outputs[i]);
    }

    return json;
  }


  static void setupRoutes(crow::App<>& app, EngineService& engineService) {
    CROW_ROUTE(app, "/api/nodes")
        .methods("POST"_method)
            ([&engineService](const crow::request& req) {
              auto x = crow::json::load(req.body);
              if (!x)
                return crow::response(400, "Invalid JSON");

              try {
                auto [instanceId, name] = engineService.AddNode(
                    x["packageId"].u(),
                    x["nodeId"].u(),
                    x["parentId"].u(),
                    x["posX"].u(),
                    x["posY"].u()
                );

                crow::json::wvalue response;
                response["instanceId"] = instanceId;
                response["name"] = name;

                return crow::response(response);
              } catch (const std::exception& e) {
                return crow::response(500, e.what());
              }
            });

    CROW_ROUTE(app, "/api/nodes")
        .methods("PUT"_method)
            ([&engineService](const crow::request& req) {
              auto x = crow::json::load(req.body);
              if (!x)
                return crow::response(400, "Invalid JSON");

              try {
                auto [instanceId, name] = engineService.UpdateNode(
                    x["instanceId"].u(),
                    x["posX"].u(),
                    x["posY"].u()
                );

                crow::json::wvalue response;
                response["instanceId"] = instanceId;
                response["name"] = name;

                return crow::response(response);
              } catch (const std::exception& e) {
                return crow::response(500, e.what());
              }
            });

    CROW_ROUTE(app, "/api/nodes/<uint>")
        .methods("DELETE"_method)
            ([&engineService](uint32_t instanceId) {
              try {
                auto resultId = engineService.removeNode(instanceId);

                crow::json::wvalue response;
                response["instanceId"] = resultId;

                return crow::response(response);
              } catch (const std::exception& e) {
                return crow::response(500, e.what());
              }
            });

    CROW_ROUTE(app, "/api/nodes")
        .methods("GET"_method)
            ([&engineService]() {
              try {
                capnp::EzRpcClient client(kj::str("unix:", "/tmp/engine-socket").cStr());
                Engine::Client engine = client.getMain<Engine>();

                auto request = engine.getAllValuesRequest();
                auto response = request.send().wait(client.getWaitScope());
                auto nodes = response.getNodes();

                crow::json::wvalue result;
                for (size_t i = 0; i < nodes.size(); i++) {
                  result[i] = convertNodeToJson(nodes[i]);
                }

                return crow::response(result);
              } catch (const std::exception& e) {
                return crow::response(500, e.what());
              }
            });

  }
};

#endif //ENGINE_ROUTES_HPP_
