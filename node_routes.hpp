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
  static void registerRoutes(crow::App<crow::CORSHandler>& app, EngineService& engineService, OpenAPIBuilder& apiBuilder) {
    setupSwaggerDocs(apiBuilder);
    setupRoutes(app, engineService);

  }

 private:
  static crow::json::wvalue createFlexValueSchema() {
    crow::json::wvalue schema;
    auto& types = schema["oneOf"];
    types = crow::json::wvalue::list();
    types[0]["type"] = "integer";
    types[1]["type"] = "number";
    types[2]["type"] = "boolean";
    types[3]["type"] = "string";
    return schema;
  }


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

    auto nodeStatusSchema = OpenAPIBuilder::createObjectSchema({
                                                           {"status", "string"},
                                                           {"count", "integer"},
                                                           {"duration", "integer"}
                                                       });

    auto ioSchema = OpenAPIBuilder::createObjectSchema({
                                                           {"name", "string"},
                                                           {"override", "boolean"}
                                                          });
    ioSchema["properties"]["value"] = createFlexValueSchema();
    ioSchema["properties"]["override_value"] = createFlexValueSchema();

    auto inputSchema = ioSchema;
    inputSchema["properties"]["default_value"] = createFlexValueSchema();

    auto outputSchema = ioSchema;
    inputSchema["properties"]["fallback_value"] = createFlexValueSchema();


    auto defaultSchema = OpenAPIBuilder::createObjectSchema({
                                                           {"name", "string"},
                                                       });
    defaultSchema["properties"]["value"] = createFlexValueSchema();


    auto instanceIdParam = OpenAPIBuilder::createParameter(
        "instanceId",
        "path",
        true,
        "integer",
        "Instance ID of the node"
    );

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
        "/api/nodes/{instanceId}/default",
        "PUT",
        "Set default value for node",
        defaultSchema,
        {{"200", {{"description", "Default value set successfully"}}}},
        {instanceIdParam}  // Add parameter here
    );


    auto overrideSchema = defaultSchema;  // Copy the base IO schema
    overrideSchema["properties"]["duration"] = {{"type", "integer"}};
    overrideSchema["properties"]["active"] = {{"type", "boolean"}};
    overrideSchema["properties"]["input"] = {{"type", "boolean"}};
    apiBuilder.addEndpoint(
        "/api/nodes/{instanceId}/override",
        "PUT",
        "Set override value for node",
        overrideSchema,
        {{"200", {{"description", "Override value set successfully"}}}},
        {instanceIdParam}  // Add parameter here
    );

    apiBuilder.addEndpoint(
        "/api/nodes/{instanceId}/fallback",
        "PUT",
        "Set fallback value for node",
        ioSchema,
        {{"200", {{"description", "Fallback value set successfully"}}}},
        {instanceIdParam}  // Add parameter here
    );

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
                        {"items", {
                            {"type", "object"},
                            {"properties", {
                                {"instanceId", {{"type", "integer"}}},
                                {"nodeName", {{"type", "string"}}},
                                {"hasChildren", {{"type", "boolean"}}},
                                {"nodeStatus", nodeStatusSchema},
                                {"inputs", {
                                    {"type", "array"},
                                    {"items", inputSchema}
                                }},
                                {"outputs", {
                                    {"type", "array"},
                                    {"items", outputSchema}
                                }}
                            }}
                        }}
                    }}
                }}
            }}
        }}}
    );
  }

  static crow::json::wvalue convertFlexValueToJson(const FlexValueCap::Reader& flex) {
    if (flex.isIntVal()) {
      return crow::json::wvalue(static_cast<std::int64_t>(flex.getIntVal()));
    } else if (flex.isUintVal()) {
      return crow::json::wvalue(static_cast<std::uint64_t>(flex.getUintVal()));
    } else if (flex.isBoolVal()) {
      return crow::json::wvalue(flex.getBoolVal());
    } else if (flex.isDoubleVal()) {
      return crow::json::wvalue(flex.getDoubleVal());
    } else if (flex.isStringVal()) {
      return crow::json::wvalue(std::string(flex.getStringVal().cStr()));
    }
    return crow::json::wvalue(nullptr);
  }

  static crow::json::wvalue convertIOToJson(const IO::Reader& io) {
    crow::json::wvalue json;
    json["name"] = std::string(io.getName().cStr());
    json["value"] = convertFlexValueToJson(io.getValue());
    json["override"] = io.getOverride();
    json["override_value"] = convertFlexValueToJson(io.getOverrideValue());
    json["default_value"] = convertFlexValueToJson(io.getDefaultValue());
    return json;
  }
  static crow::json::wvalue convertOutputIOToJson(const IO::Reader& io) {
    crow::json::wvalue json;
    json["name"] = std::string(io.getName().cStr());
    json["value"] = convertFlexValueToJson(io.getValue());
    json["override"] = io.getOverride();
    json["override_value"] = convertFlexValueToJson(io.getOverrideValue());
    json["fallback_value"] = convertFlexValueToJson(io.getDefaultValue());  // renamed for outputs
    return json;
  }

  static crow::json::wvalue convertNodeToJson(const Node::Reader& node) {
    crow::json::wvalue json;
    json["instanceId"] = static_cast<uint32_t>(node.getInstanceId());
    json["nodeName"] = std::string(node.getNodeName().cStr());
    json["hasChildren"] = node.getHasChildren();

    // Add NodeStatus
    auto nodeStatus = node.getNodeStatus();
    json["nodeStatus"]["status"] = std::string(nodeStatus.getStatus().cStr());
    json["nodeStatus"]["count"] = static_cast<uint32_t>(nodeStatus.getCount());
    json["nodeStatus"]["duration"] = static_cast<uint32_t>(nodeStatus.getDuration());

    json["inputs"] = crow::json::wvalue::list();
    auto inputs = node.getInputs();
    for (size_t i = 0; i < inputs.size(); i++) {
      json["inputs"][i] = convertIOToJson(inputs[i]);
    }

    json["outputs"] = crow::json::wvalue::list();
    auto outputs = node.getOutputs();
    for (size_t i = 0; i < outputs.size(); i++) {
      json["outputs"][i] = convertOutputIOToJson(outputs[i]);
    }

    return json;
  }


  static void setupRoutes(crow::App<crow::CORSHandler>& app, EngineService& engineService) {
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

    CROW_ROUTE(app, "/api/nodes/<uint>/default")
        .methods("PUT"_method)
            ([&engineService](const crow::request& req, uint32_t instance_id) {
              auto x = crow::json::load(req.body);
              if (!x || !x.has("name") || !x.has("value"))
                return crow::response(400, "Invalid JSON. Required fields: 'name' and 'value'");

              try {
                engineService.SetDefault(instance_id, x["name"].s(), x["value"]);
                return crow::response(200);
              } catch (const std::exception& e) {
                return crow::response(500, e.what());
              }
            });

    CROW_ROUTE(app, "/api/nodes/<uint>/override")
        .methods("PUT"_method)
            ([&engineService](const crow::request& req, uint32_t instance_id) {
              auto x = crow::json::load(req.body);
              if (!x || !x.has("name") || !x.has("value") || !x.has("duration"))
                return crow::response(400, "Invalid JSON. Required fields: 'name', 'value', and 'duration'");

              try {
                engineService.SetOverride(
                    instance_id,
                    x["name"].s(),
                    x["value"],
                    x["duration"].u(),
                    x["active"].b(),
                    x["input"].b()
                );
                return crow::response(200);
              } catch (const std::exception& e) {
                return crow::response(500, e.what());
              }
            });

    CROW_ROUTE(app, "/api/nodes/<uint>/fallback")
        .methods("PUT"_method)
            ([&engineService](const crow::request& req, uint32_t instance_id) {
              auto x = crow::json::load(req.body);
              if (!x || !x.has("name") || !x.has("value"))
                return crow::response(400, "Invalid JSON. Required fields: 'name' and 'value'");

              try {
                engineService.SetFallback(instance_id, x["name"].s(), x["value"]);
                return crow::response(200);
              } catch (const std::exception& e) {
                return crow::response(500, e.what());
              }
            });


  }
};

#endif //ENGINE_ROUTES_HPP_
