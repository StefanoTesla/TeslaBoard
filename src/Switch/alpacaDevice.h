#ifndef SWITCH_ALPACA_DEVICE
#define SWITCH_ALPACA_DEVICE
#include "libraries.h"
#include "header.h"


bool canBeWritten(unsigned int id){
      if(
      Switch.data[id].property.type == SwTypeDOutput ||
      Switch.data[id].property.type == SwTypePWM ||
      Switch.data[id].property.type == SwTypeServo
      ){ 
            return true; 
      }
      return false;
}

/* Not writable error related */
void unWritableIdErrorMessage(AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      char message[100];
      int tmp = Switch.config.configuredSwitch - 1;
      int id = request->getAttribute("id").toInt();
      sprintf(message, "Switch n: %d, cannot be written", id);
      doc["ErrorNumber"] = 1025;
      doc["ErrorMessage"] = message;
      doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
      doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
      response->setLength();
      response->setCode(400);
      request->send(response);
}

/* ID error realted */
void missingIdErrorMessage(AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      
      doc["ErrorNumber"] = 1025;
      if (Global.config.language.locale == "it"){
            doc["ErrorMessage"] = "ID non fornito";
      } else {
            doc["ErrorMessage"] = "ID not provided";
      }
      doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
      doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
      response->setLength();
      response->setCode(400);
      request->send(response);
}

void IdOutOfRangeErrorMessage(AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      char message[100];
      int tmp = Switch.config.configuredSwitch - 1;
      int id = request->getAttribute("id").toInt();
      if (Global.config.language.locale == "it"){
            sprintf(message, "L'ID fornito: %d, è fuori range, il massimo è: %d", id, tmp);
      } else {
            sprintf(message, "ID provided: %d, outside range, maximum is: %d", id, tmp);
      }
      doc["ErrorNumber"] = 1025;
      doc["ErrorMessage"] = message;
      doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
      doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
      response->setLength();
      response->setCode(400);
      request->send(response);
}

AsyncMiddlewareFunction getID([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
      int paramsNr = request->params();
      bool exist;
      String parameter;
      int id = -1;
      for (int i = 0; i < paramsNr; i++) {
            const AsyncWebParameter* p = request->getParam(i);
            parameter = p->name();
            parameter.toLowerCase();
            if (parameter == "id") {
                  id = p->value().toInt();
                  if( id >= 0 && id < Switch.config.configuredSwitch){
                        request->setAttribute("id", static_cast<long>(id));
                        next();
                        return;
                  } else {
                       IdOutOfRangeErrorMessage(request); 
                       return;
                  }
            }
      }
      missingIdErrorMessage(request);    
});

/* State error realted */
void missingStateErrorMessage(AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      char message[100];
      int tmp = Switch.config.configuredSwitch - 1;
      if (Global.config.language.locale == "it"){
            sprintf(message,"Parametro \"State\" non fornito");
      } else {
            sprintf(message,"\"State\" parameter not provided");
      }
      doc["ErrorNumber"] = 1025;
      doc["ErrorMessage"] = message;
      doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
      doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
      response->setLength();
      response->setCode(400);
      request->send(response);
}

AsyncMiddlewareFunction getState([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
      int paramsNr = request->params();
      String parameter;

      for (int i = 0; i < paramsNr; i++) {
            const AsyncWebParameter* p = request->getParam(i);
            parameter = p->name();
            parameter.toLowerCase();
            if (parameter == "state") {
                  request->setAttribute("state",  static_cast<long>(0));
                  String value = p->value();
                  value.toLowerCase();
                  if(value == "true"){
                        request->setAttribute("state",  static_cast<long>(1));
                  }
                  next();
                  return;
            }
      }
      missingStateErrorMessage(request);
      
});

/* Value error realted */
void missingValueErrorMessage(AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      char message[100];
      int tmp = Switch.config.configuredSwitch - 1;
      sprintf(message, "\"Value\" parameter not provided");
      doc["ErrorNumber"] = 1025;
      doc["ErrorMessage"] = message;
      doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
      doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
      response->setLength();
      response->setCode(400);
      request->send(response);
}

void valueOutOfRangeErrorMessage(AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = new AsyncJsonResponse();
      JsonObject doc = response->getRoot().to<JsonObject>();
      char message[100];
      int id = request->getAttribute("id").toInt();
      int value = request->getAttribute("value").toInt();
      sprintf(message, "ID provided: %d, outside range, maximum is: %d", id, value);
      doc["ErrorNumber"] = 1025;
      doc["ErrorMessage"] = message;
      doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
      doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
      response->setLength();
      response->setCode(400);
      request->send(response);
}

AsyncMiddlewareFunction getValue([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
      int paramsNr = request->params();
      String parameter;
      for (int i = 0; i < paramsNr; i++) {
            const AsyncWebParameter* p = request->getParam(i);
            parameter = p->name();
            parameter.toLowerCase();
            if (parameter == "value") {
                  request->setAttribute("value", static_cast<long>(p->value().toInt()));
                  next();
                  return;
            }
      }
      missingValueErrorMessage(request);
});
/* used where value is provided */
AsyncMiddlewareFunction isValueable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
      int id = request->getAttribute("id").toInt();
      int value = request->getAttribute("value").toInt();
      if(canBeWritten(id) && (value >= Switch.data[id].property.minValue && value <= Switch.data[id].property.maxValue) ){
            next();
            return;
      }
      valueOutOfRangeErrorMessage(request);
      
});
/* used where state is provided */
AsyncMiddlewareFunction isStatable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
      int id = request->getAttribute("id").toInt();
      if(canBeWritten(id)){
            next();
            return;
      }
      unWritableIdErrorMessage(request);
      
});

void switchAlpacaDevice(){

      alpaca.on("/api/v1/switch/0/maxswitch", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            doc["Value"] = Switch.config.configuredSwitch;
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddleware(&getAlpacaID);

      alpaca.on("/api/v1/switch/0/canasync", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            doc["Value"] = true;
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddlewares({&getAlpacaID,&getID});

      alpaca.on("/api/v1/switch/0/canwrite", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            int id = request->getAttribute("id").toInt();
            doc["Value"] = canBeWritten(id);
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddlewares({&getAlpacaID,&getID});

      alpaca.on("/api/v1/switch/0/getswitch", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            int id = request->getAttribute("id").toInt();
            doc["Value"] = Switch.data[id].actualValue.boValue;
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddlewares({&getAlpacaID,&getID});

      alpaca.on("/api/v1/switch/0/getswitchdescription", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            int id = request->getAttribute("id").toInt();
            doc["Value"] = Switch.data[id].property.Description;
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddlewares({&getAlpacaID,&getID});

      alpaca.on("/api/v1/switch/0/getswitchname", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            int id = request->getAttribute("id").toInt();
            doc["Value"] = Switch.data[id].property.Name;
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddlewares({&getAlpacaID,&getID});

      alpaca.on("/api/v1/switch/0/getswitchvalue", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            int id = request->getAttribute("id").toInt();
            doc["Value"] = Switch.data[id].actualValue.intValue;
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddlewares({&getAlpacaID,&getID});

      alpaca.on("/api/v1/switch/0/minswitchvalue", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            int id = request->getAttribute("id").toInt();
            doc["Value"] = Switch.data[id].property.minValue;
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddlewares({&getAlpacaID,&getID});

      alpaca.on("/api/v1/switch/0/maxswitchvalue", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            int id = request->getAttribute("id").toInt();
            doc["Value"] = Switch.data[id].property.maxValue;
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddlewares({&getAlpacaID,&getID});

      alpaca.on("/api/v1/switch/0/switchstep", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            doc["Value"] = 1;
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddlewares({&getAlpacaID,&getID});

      alpaca.on("/api/v1/switch/0/statechangecomplete", HTTP_GET, [](AsyncWebServerRequest *request){
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            int id = request->getAttribute("id").toInt();
            doc["Value"] = !Switch.data[id].command.execute;
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddlewares({&getAlpacaID,&getID});

      alpaca.on("/api/v1/switch/0/devicestate", HTTP_GET, [](AsyncWebServerRequest *request) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();
            JsonArray Value = doc["Value"].to<JsonArray>();

            
            for (int i = 0; i < Switch.config.configuredSwitch; i++)
            {
                  JsonObject getSwitch = Value.add<JsonObject>();
                  char message[15];
                  sprintf(message, "GetSwitch%d", i);
                  getSwitch["Name"] = message;
                  getSwitch["Value"] = Switch.data[i].actualValue.boValue;
            }
            //getSwitchValue
            for (int i = 0; i < Switch.config.configuredSwitch; i++)
            {
                  JsonObject getSwitchValue = Value.add<JsonObject>();
                  char message[20];
                  sprintf(message, "GetSwitchValue%d", i);
                  getSwitchValue["Name"] = message;
                  getSwitchValue["Value"] = Switch.data[i].actualValue.intValue;
            }
            //StateChangeComplete
            for (int i = 0; i < Switch.config.configuredSwitch; i++)
            {
                  JsonObject StateChangeComplete = Value.add<JsonObject>();
                  char message[25];
                  sprintf(message, "StateChangeComplete%d", i);
                  StateChangeComplete["Name"] = message;
                  StateChangeComplete["Value"] = !Switch.data[i].command.execute;
            }
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddleware(&getAlpacaID);

      alpaca.on("/api/v1/switch/0/setswitch", HTTP_PUT, [](AsyncWebServerRequest *request) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();

            int state = request->getAttribute("state").toInt();
            int id = request->getAttribute("id").toInt();

            if(Switch.data[id].property.type == SwTypeDOutput){
                  Switch.data[id].command.execute = true;
                  Switch.data[id].command.boValue = state ? true : false;  
            } else {
                  Switch.data[id].command.execute = true;
                  Switch.data[id].command.intValue = state ? Switch.data[id].property.maxValue : Switch.data[id].property.minValue;
            }
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddlewares({&getAlpacaID,&getID,&getState,&isStatable});

      alpaca.on("/api/v1/switch/0/setasync", HTTP_PUT, [](AsyncWebServerRequest *request) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();

            int state = request->getAttribute("state").toInt();
            int id = request->getAttribute("id").toInt();

            if(Switch.data[id].property.type == SwTypeDOutput){
                  Switch.data[id].command.execute = true;
                  Switch.data[id].command.boValue = state ? true : false;  
            } else {
                  Switch.data[id].command.execute = true;
                  Switch.data[id].command.intValue = state ? Switch.data[id].property.maxValue : Switch.data[id].property.minValue;
            }
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddlewares({&getAlpacaID,&getID,&getState,&isStatable});

      alpaca.on("/api/v1/switch/0/setswitchvalue", HTTP_PUT, [](AsyncWebServerRequest *request) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();

            int value = request->getAttribute("value").toInt();
            int id = request->getAttribute("id").toInt();

            if(Switch.data[id].property.type == SwTypeDOutput){
                  Switch.data[id].command.execute = true;
                  Switch.data[id].command.boValue = value ? true : false;  
            } else {
                  Switch.data[id].command.execute = true;
                  Switch.data[id].command.intValue = value;
            }
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddlewares({&getAlpacaID,&getID,&getValue,&isValueable});

      alpaca.on("/api/v1/switch/0/setasyncvalue", HTTP_PUT, [](AsyncWebServerRequest *request) {
            AsyncJsonResponse* response = new AsyncJsonResponse();
            JsonObject doc = response->getRoot().to<JsonObject>();

            int value = request->getAttribute("value").toInt();
            int id = request->getAttribute("id").toInt();

            if(Switch.data[id].property.type == SwTypeDOutput){
                  Switch.data[id].command.execute = true;
                  Switch.data[id].command.boValue = value ? true : false;  
            } else {
                  Switch.data[id].command.execute = true;
                  Switch.data[id].command.intValue = value;
            }
            doc["ErrorNumber"] = 0;
            doc["ErrorMessage"] = "";
            doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
            doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
            response->setLength();
            request->send(response);
      }).addMiddlewares({&getAlpacaID,&getID,&getValue,&isValueable});


    /* Methods not implemented:*/
    alpaca.on("/api/v1/switch/0/commandblind",     HTTP_PUT, alpacaMethodNotImplemented).addMiddleware(&getAlpacaID);
    alpaca.on("/api/v1/switch/0/commandbool",      HTTP_PUT, alpacaMethodNotImplemented).addMiddleware(&getAlpacaID);
    alpaca.on("/api/v1/switch/0/commandstring",    HTTP_PUT, alpacaMethodNotImplemented).addMiddleware(&getAlpacaID);
    alpaca.on("/api/v1/switch/0/setswitchname",    HTTP_PUT, alpacaMethodNotImplemented).addMiddleware(&getAlpacaID);

   /* I don't care about connection but we need to declare it*/
    alpaca.on("/api/v1/switch/0/connect", HTTP_PUT, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        doc["ErrorNumber"] = 0;
        doc["ErrorMessage"] = "";
        doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
        doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpacaID);

    alpaca.on("/api/v1/switch/0/disconnect", HTTP_PUT, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        doc["ErrorNumber"] = 0;
        doc["ErrorMessage"] = "";
        doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
        doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
        response->setLength();
        request->send(response);

    }).addMiddleware(&getAlpacaID);

    alpaca.on("/api/v1/switch/0/connecting", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        doc["Value"] = false;
        doc["ErrorNumber"] = 0;
        doc["ErrorMessage"] = "";
        doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
        doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpacaID);

    alpaca.on("/api/v1/switch/0/connected", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        doc["Value"] = true;
        doc["ErrorNumber"] = 0;
        doc["ErrorMessage"] = "";
        doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
        doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpacaID);

    alpaca.on("/api/v1/switch/0/connected", HTTP_PUT, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse* response = new AsyncJsonResponse();
        JsonObject doc = response->getRoot().to<JsonObject>();
        doc["ErrorNumber"] = 0;
        doc["ErrorMessage"] = "";
        doc["ClientTransactionID"] = AlpacaData.clientTransactionID;
        doc["ServerTransactionID"] = AlpacaData.serverTransactionID;
        response->setLength();
        request->send(response);
    }).addMiddleware(&getAlpacaID);

    alpaca.on("/api/v1/switch/0/supportedactions",HTTP_GET, alpacaNoActions).addMiddleware(&getAlpacaID);
    alpaca.on("/api/v1/switch/0/action",HTTP_PUT, alpacaActionNotImplemented).addMiddleware(&getAlpacaID);
}

#endif