#ifndef SAFETY_WEBSERVER
#define SAFETY_WEBSERVER

extern SafetyModule Safety;


void SafetyApi() {

    #pragma region webApi

    server.on("/api/safety/cfg", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = new AsyncJsonResponse();
        JsonObject doc = response -> getRoot().to < JsonObject > ();
        doc["enable"] = Safety.isEnable();
        doc["order"] = Safety.getUiOrder();
        doc["identifier"] = Safety.getIdentifier();

        Safety.getConfiguration(doc);

        response -> setLength();
        request -> send(response);
    });

    server.on("/api/safety/status", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = new AsyncJsonResponse();
        JsonObject doc = response -> getRoot().to < JsonObject > ();

        doc["isSafe"] = Safety.isSafe();
        JsonArray array = doc["Conditions"].to < JsonArray > ();

        for (size_t i = 0; i < Safety.getConfiguredConditions(); i++) {
            JsonObject cnd = array.add < JsonObject > ();
            Safety.reportConditionState(i, cnd);
        }

        response -> setLength();
        request -> send(response);
    });

    AsyncCallbackJsonWebHandler * safetyConfigHandler = new AsyncCallbackJsonWebHandler("/api/safety/cfg");

    safetyConfigHandler -> setMethod(HTTP_POST | HTTP_PUT);
    safetyConfigHandler -> onRequest([](AsyncWebServerRequest * request, JsonVariant & root) {
        AsyncJsonResponse * response = new AsyncJsonResponse();
        JsonObject doc = response -> getRoot().to < JsonObject > ();

        // convert the JsonVariant to JsonObject
        const JsonObject & incomingObj = root.as < JsonObject > ();

        JsonArray err = doc["errors"].to < JsonArray > ();

        Safety.validateConfiguration(incomingObj, doc);

        if (err.size() > 0) {
            response -> setCode(500);
            response -> setLength();
            request -> send(response);
            return;
        }

        Safety.storeConfiguration(incomingObj);

        response -> setLength();
        request -> send(response);
    });

    server.addHandler(safetyConfigHandler);



    #pragma endregion

    #pragma region AlpachaManagement

    alpaca.on("/api/v1/safetymonitor/0/name", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();

        doc["Value"] = Safety.getIdentifier() + " - TeslaBoard";

        response -> setLength();
        request -> send(response);
    }).addMiddleware( & getAlpParams);

    alpaca.on("/api/v1/safetymonitor/0/description", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();

        doc["Value"] = "Safety handled by Stefano TeslaBoard";

        response -> setLength();
        request -> send(response);
    }).addMiddleware( & getAlpParams);

    alpaca.on("/api/v1/safetymonitor/0/driverversion", HTTP_GET,  [](AsyncWebServerRequest * request) {
            AsyncJsonResponse * response = prepareAlpacaResponse(request);
            JsonObject doc = response -> getRoot();

            doc["Value"] = "4.0.0";

            response -> setLength();
            request -> send(response);
    });

    alpaca.on("/api/v1/safetymonitor/0/driverinfo", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();

        doc["Value"] = "Safety module to relax during the night";

        response -> setLength();
        request -> send(response);
    }).addMiddleware( & getAlpParams);

    alpaca.on("/api/v1/safetymonitor/0/interfaceversion", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();

        doc["Value"] = 3;

        response -> setLength();
        request -> send(response);
    }).addMiddleware( & getAlpParams);

    #pragma endregion

    #pragma region AlpacaDevice

    alpaca.on("/api/v1/safetymonitor/0/issafe", HTTP_GET, [](AsyncWebServerRequest * request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response -> getRoot();

        doc["Value"] = Safety.isSafe();

        response -> setLength();
        request -> send(response);
    }).addMiddlewares({&getAlpParams});

    #pragma region connection

    alpaca.on("/api/v1/safetymonitor/0/connect", HTTP_PUT, [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            response->setLength();
            request->send(response);
    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/safetymonitor/0/disconnect", HTTP_PUT, [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);

            response->setLength();
            request->send(response);
    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/safetymonitor/0/connecting", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse *response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();

        doc["Value"] = false;

        request->send(response);
    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/safetymonitor/0/connected", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse *response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();

        doc["Value"] = Safety.isEnable();

        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpParams);

    alpaca.on("/api/v1/safetymonitor/0/connected", HTTP_PUT, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse *response = prepareAlpacaResponse(request);

        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpParams);

    #pragma endregion

    alpaca.on("/api/v1/safetymonitor/0/devicestate", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse *response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
        doc["isSafe"] = Safety.isSafe();

        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpParams);

    /* Methods not implemented:*/
    alpaca.on("/api/v1/safetymonitor/0/commandblind", HTTP_PUT, alpacaMethodNotImplemented).addMiddleware(&getAlpParams);
    alpaca.on("/api/v1/safetymonitor/0/commandbool", HTTP_PUT, alpacaMethodNotImplemented).addMiddleware(&getAlpParams);
    alpaca.on("/api/v1/safetymonitor/0/commandstring", HTTP_PUT,alpacaMethodNotImplemented).addMiddleware(&getAlpParams);
    alpaca.on("/api/v1/safetymonitor/0/supportedactions", HTTP_GET, alpacaNoActions).addMiddleware(&getAlpParams);
    alpaca.on("/api/v1/safetymonitor/0/action", HTTP_PUT, alpacaActionNotImplemented).addMiddleware(&getAlpParams);
    #pragma endregion
}
#endif