//
// Created by craig on 7/11/2024.
//

#ifndef SWAGGER_UI_HPP_
#define SWAGGER_UI_HPP_


#pragma once
#include <string>

namespace swagger {
inline std::string get_html() {
  return R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>Swagger UI</title>
    <link rel="stylesheet" type="text/css" href="https://cdn.jsdelivr.net/npm/swagger-ui-dist@5/swagger-ui.css">
    <script src="https://cdn.jsdelivr.net/npm/swagger-ui-dist@5/swagger-ui-bundle.js"></script>
</head>
<body>
    <div id="swagger-ui"></div>
    <script>
        window.onload = function() {
            SwaggerUIBundle({
                url: "/api/v1/swagger",
                dom_id: '#swagger-ui',
                presets: [SwaggerUIBundle.presets.apis],
                layout: "BaseLayout"
            });
        }
    </script>
</body>
</html>
        )";
}
}

#endif //SWAGGER_UI_HPP_
