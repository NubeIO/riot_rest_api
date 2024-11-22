//
// Created by craig on 7/11/2024.
//

#ifndef OPEN_API_BUILDER_HPP_
#define OPEN_API_BUILDER_HPP_

class OpenAPIBuilder {
 private:
  crow::json::wvalue schema;

 public:
  OpenAPIBuilder() {
    schema["openapi"] = "3.0.0";
    schema["info"]["title"] = "My API";
    schema["info"]["version"] = "1.0.0";

  }
  void updateServerUrl(const std::string& url) {
    schema["servers"][0]["url"] = url;
    std::cout << url << std::endl;
  }


  void addEndpoint(const std::string& path, const std::string& method,
                   const std::string& summary,
                   crow::json::wvalue requestBody = crow::json::wvalue(),
                   crow::json::wvalue responses = crow::json::wvalue(),
                   std::vector<crow::json::wvalue> parameters = std::vector<crow::json::wvalue>()) {
    auto lowMethod = std::string(method);
    std::transform(lowMethod.begin(), lowMethod.end(), lowMethod.begin(), ::tolower);

    auto& pathMethod = schema["paths"][path][lowMethod];
    pathMethod["summary"] = summary;

    // Add parameters if any
    if (!parameters.empty()) {
      auto& params = pathMethod["parameters"];
      for (size_t i = 0; i < parameters.size(); i++) {
        params[i] = std::move(parameters[i]);
      }
    }

    if (requestBody.dump() != "null") {
      pathMethod["requestBody"]["content"]["application/json"]["schema"] =
          std::move(requestBody);
    }

    if (responses.dump() != "null") {
      pathMethod["responses"] = std::move(responses);
    }
  }

  static crow::json::wvalue createObjectSchema(const std::vector<std::pair<std::string, std::string>>& properties) {
    crow::json::wvalue schema;
    schema["type"] = "object";
    for (const auto& [prop, type] : properties) {
      schema["properties"][prop]["type"] = type;
    }
    return schema;
  }

  static crow::json::wvalue createParameter(
      const std::string& name,
      const std::string& in,
      bool required,
      const std::string& type,
      const std::string& description = "") {

    crow::json::wvalue param;
    param["name"] = name;
    param["in"] = in;
    param["required"] = required;
    param["schema"]["type"] = type;
    if (!description.empty()) {
      param["description"] = description;
    }
    return param;
  }

  crow::json::wvalue getSchema() const {
    return schema;
  }
};

#endif //OPEN_API_BUILDER_HPP_
