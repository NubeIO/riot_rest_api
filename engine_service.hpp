//
// Created by craig on 7/11/2024.
//

#ifndef ENGINE_SERVICE_HPP_
#define ENGINE_SERVICE_HPP_
#include <capnp/ez-rpc.h>
#include <iostream>
#include "schemas/package.capnp.h"

class EngineService {
 private:


 public:
  EngineService() {}
  const char *SOCKET_PATH = "/tmp/engine-socket";
  std::pair<uint32_t, std::string> AddNode(uint32_t package_id, uint32_t node_id,
                                           uint32_t parent_id, uint32_t pos_x, uint32_t pos_y) {
    capnp::EzRpcClient client(kj::str("unix:", SOCKET_PATH).cStr());
    Engine::Client engine = client.getMain<Engine>();

    auto request = engine.addNodeRequest();
    auto node_details = request.getNodeDetails();
    node_details.setPackageId(package_id);
    node_details.setNodeId(node_id);
    node_details.setParentId(parent_id);
    node_details.setPosX(pos_x);
    node_details.setPosY(pos_y);

    auto response = request.send().wait(client.getWaitScope());
    return {response.getInstanceId(), response.getName().cStr()};
  }

  std::pair<uint32_t, std::string> UpdateNode(uint32_t instance_id, uint32_t pos_x, uint32_t pos_y) {
    capnp::EzRpcClient client(kj::str("unix:", SOCKET_PATH).cStr());
    Engine::Client engine = client.getMain<Engine>();

    auto request = engine.updateNodeRequest();
    auto node_details = request.getNodeDetails();
    node_details.setInstanceId(instance_id);
    node_details.setPosX(pos_x);
    node_details.setPosY(pos_y);

    auto response = request.send().wait(client.getWaitScope());
    return {response.getInstanceId(), response.getName().cStr()};
  }

  uint32_t removeNode(uint32_t instanceId) {
    capnp::EzRpcClient client(kj::str("unix:", SOCKET_PATH).cStr());
    Engine::Client engine = client.getMain<Engine>();

    auto request = engine.removeNodeRequest();
    request.setInstanceId(instanceId);

    auto response = request.send().wait(client.getWaitScope());
    return response.getInstanceId();
  }

  struct EdgeResult {
    uint32_t edge_id;
    bool data_only;
  };

  EdgeResult AddEdge(uint32_t from_instance_id, uint32_t to_instance_id,
                     const std::string& out_name, const std::string& in_name) {
    capnp::EzRpcClient client(kj::str("unix:", SOCKET_PATH).cStr());
    Engine::Client engine = client.getMain<Engine>();

    auto request = engine.addEdgeRequest();
    auto edge = request.getEdge();
    edge.setFromInstanceId(from_instance_id);
    edge.setToInstanceId(to_instance_id);
    edge.setOutName(out_name);
    edge.setInName(in_name);

    auto response = request.send().wait(client.getWaitScope());
    return EdgeResult{
        .edge_id = response.getEdgeId(),
        .data_only = response.getDataOnly()
    };
  }

  uint32_t RemoveEdge(uint32_t edge_id) {
    capnp::EzRpcClient client(kj::str("unix:", SOCKET_PATH).cStr());
    Engine::Client engine = client.getMain<Engine>();

    auto request = engine.removeEdgeRequest();
    request.setEdgeId(edge_id);

    auto response = request.send().wait(client.getWaitScope());
    return response.getEdgeId();
  }

  capnp::Response<Engine::GetAllValuesResults> GetAllNodes() {
    capnp::EzRpcClient client(kj::str("unix:", SOCKET_PATH).cStr());
    Engine::Client engine = client.getMain<Engine>();

    auto request = engine.getAllValuesRequest();
    auto response = request.send().wait(client.getWaitScope());

    return response;
  }

  std::vector<PackageDetails::Reader> GetAvailablePackages() {
    capnp::EzRpcClient client(kj::str("unix:", SOCKET_PATH).cStr());
    Engine::Client engine = client.getMain<Engine>();

    auto request = engine.getAvailablePackagesRequest();
    auto response = request.send().wait(client.getWaitScope());

    auto packages = response.getAvailablePackages();
    std::vector<PackageDetails::Reader> result;
    result.reserve(packages.size());

    for (auto package : packages) {
      result.push_back(package);
    }

    return result;
  }

  std::string GetPackageJson(uint32_t packageId) {
    capnp::EzRpcClient client(kj::str("unix:", SOCKET_PATH).cStr());
    Engine::Client engine = client.getMain<Engine>();

    auto request = engine.getPackageJsonRequest();
    request.setPackageId(packageId);

    auto response = request.send().wait(client.getWaitScope());
    return response.getJsonData();
  }


  std::string GetFlowJson() {
    capnp::EzRpcClient client(kj::str("unix:", SOCKET_PATH).cStr());
    Engine::Client engine = client.getMain<Engine>();

    auto request = engine.getFlowJsonRequest();
    auto response = request.send().wait(client.getWaitScope());

    return response.getJsonData().cStr();
  }

  void SetDefault(uint32_t instance_id, const std::string& name, const crow::json::rvalue& value) {
    capnp::EzRpcClient client(kj::str("unix:", SOCKET_PATH).cStr());
    Engine::Client engine = client.getMain<Engine>();

    auto request = engine.setDefaultRequest();
    request.setInstanceId(instance_id);

    auto io = request.getDefault();
    io.setName(name);

    auto flex_value = io.getValue();
    setFlexValue(flex_value, value);

    request.send().wait(client.getWaitScope());
  }

  void SetOverride(uint32_t instance_id, const std::string& name,
                   const crow::json::rvalue& value, uint32_t duration, bool active, bool input) {
    capnp::EzRpcClient client(kj::str("unix:", SOCKET_PATH).cStr());
    Engine::Client engine = client.getMain<Engine>();

    auto request = engine.setOverrideRequest();
    request.setInstanceId(instance_id);
    request.setDuration(duration);
    request.setActive(active);
    request.setInput(input);

    auto io = request.getOverride();
    io.setName(name);

    auto flex_value = io.getValue();
    setFlexValue(flex_value, value);

    request.send().wait(client.getWaitScope());
  }

  void SetFallback(uint32_t instance_id, const std::string& name, const crow::json::rvalue& value) {
    capnp::EzRpcClient client(kj::str("unix:", SOCKET_PATH).cStr());
    Engine::Client engine = client.getMain<Engine>();

    auto request = engine.setFallbackRequest();
    request.setInstanceId(instance_id);

    auto io = request.getFallback();
    io.setName(name);

    auto flex_value = io.getValue();
    setFlexValue(flex_value, value);

    request.send().wait(client.getWaitScope());
  }

  void setFlexValue(FlexValueCap::Builder flex_value, const crow::json::rvalue& value) {
    if (value.t() == crow::json::type::Number) {
      if (value.d() == std::floor(value.d())) {
        // Integer value
        if (value.i() >= 0) {
          flex_value.setUintVal(value.u());
        } else {
          flex_value.setIntVal(value.i());
        }
      } else {
        // Double value
        flex_value.setDoubleVal(value.d());
      }
    } else if (value.t() == crow::json::type::String) {
      std::string str_val = value.s();
      flex_value.setStringVal(kj::str(str_val));
    } else if (value.t() == crow::json::type::True ||
        value.t() == crow::json::type::False) {
      flex_value.setBoolVal(value.b());
    }
  }




};

#endif //ENGINE_SERVICE_HPP_
