#ifndef TEST_API
#define TEST_API

#include "esp_log.h"
#undef LOG_TAG
#define LOG_TAG "TestAPI"
#define LOGV(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGD(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGW(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)


AsyncMiddlewareFunction md1([](AsyncWebServerRequest * request,
    ArMiddlewareNext next) {
    String id = request->getAttribute("id", String("null"));
    LOGV("MD1 string value= %s",id);
    if (id == "null") {
        missingIdErrorMessage(request);
        return;
    } else {
        int i = id.toInt();
        if (i > 10) {
            IdOutOfRangeErrorMessage(request);
            return;
        } else {
            LOGV("next1");
            next();
        }
    }
});
AsyncMiddlewareFunction md2([](AsyncWebServerRequest * request, ArMiddlewareNext next) {
    String ctid = request->getAttribute("ctid", String("null"));
    LOGV("MD2 id= %s",ctid.c_str());
    LOGV("MD2 no c_str id= %s",ctid);
    if (ctid == "null") {
        missingIdErrorMessage(request);
        return;
    } else {
        uint32_t conv = strtoul(request->getAttribute("ctid").c_str(), nullptr, 10);
        LOGV("MD2 client transaction id= %lu",conv);
    }

    LOGV("next2");
    next();
});

void test(){

    server.on("/api/test/mid", HTTP_POST,[](AsyncWebServerRequest *request) {
        AsyncJsonResponse * response = prepareAlpacaResponse(request);
        JsonObject doc = response->getRoot();
            
        doc["execute"] = true;

        response->setLength();
        request->send(response);
    }).addMiddlewares({&getAlpParams,&md1,&md2});
}

#endif