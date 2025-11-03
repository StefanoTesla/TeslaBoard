#ifndef SWITCH_WEBSERVER
#define SWITCH_WEBSERVER

extern SwitchModule Switches

#pragma region Middleware

bool canBeWritten(unsigned int id){
  /*
      if(SwitchObjects[id] == nullptr){ return false; }
      switch (SwitchObjects[id]->getType())
      {
      case SwTypeDOutput:
      case SwTypePWM:
      case SwTypeServo:
            return true;
            break;
      
      default:
            break;
      } */
      return false;
     
}

/* Not writable error related */
void unWritableIdErrorMessage(AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot().to<JsonObject>();
      char message[100];
      int tmp = Switch.config.configuredSwitch - 1;
      int id = request->getAttribute("id").toInt();
      sprintf(message, "Switch n: %d, cannot be written", id);
      doc["ErrorNumber"] = 1025;
      doc["ErrorMessage"] = message;
      response->setLength();
      response->setCode(400);
      request->send(response);
}

/* ID error realted */
void missingIdErrorMessage(AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();
      
      doc["ErrorNumber"] = 1025;
      doc["ErrorMessage"] = "ID not provided";
      
      response->setLength();
      response->setCode(400);
      request->send(response);
}

void IdOutOfRangeErrorMessage(AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();
      char message[100];
      int tmp = Switch.config.configuredSwitch - 1;
      int id = request->getAttribute("id").toInt();
      sprintf(message, "ID provided: %d, outside range, maximum is: %d", id, tmp);
      doc["ErrorNumber"] = 1025;
      doc["ErrorMessage"] = message;

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
                  if( id >= 0 && id < Switch.config.configuredSwitch && SwitchObjects[id] != nullptr){
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
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot().to<JsonObject>();
      char message[100];
      int tmp = Switch.config.configuredSwitch - 1;
      sprintf(message,"\"State\" parameter not provided");
      doc["ErrorNumber"] = 1025;
      doc["ErrorMessage"] = message;
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
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();

      doc["ErrorNumber"] = 1025;
      doc["ErrorMessage"] = "\"Value\" parameter not provided";

      response->setLength();
      response->setCode(400);
      request->send(response);
}

void valueOutOfRangeErrorMessage(AsyncWebServerRequest *request) {
      AsyncJsonResponse* response = prepareAlpacaResponse(request);
      JsonObject doc = response->getRoot();
      char message[100];
      int id = request->getAttribute("id").toInt();
      int value = request->getAttribute("value").toInt();
      sprintf(message, "ID provided: %d, outside range, maximum is: %d", id, value);
      doc["ErrorNumber"] = 1025;
      doc["ErrorMessage"] = message;

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
      if(canBeWritten(id) && (value >= SwitchObjects[id]->getMin() && value <= SwitchObjects[id]->getMax()) ){
            next();
            return;
      }
      valueOutOfRangeErrorMessage(request);
      
});
/* used where state is provided */
AsyncMiddlewareFunction isSettable([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
      int id = request->getAttribute("id").toInt();
      if(canBeWritten(id)){
            next();
            return;
      }
      unWritableIdErrorMessage(request);
      
});


#pragma endregion

void switchRequestHandler() {


#pragma region alpacaDevice

  alpaca
      .on("/api/v1/switch/0/maxswitch", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] = Switch.config.configuredSwitch;

            response->setLength();
            request->send(response);
          })
      .addMiddleware(&getAlpacaID);

  alpaca
      .on("/api/v1/switch/0/canasync", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] = true;

            request->send(response);
          })
      .addMiddlewares({&getAlpacaID, &getID});

  alpaca
      .on("/api/v1/switch/0/canwrite", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();
            int id = request->getAttribute("id").toInt();

            doc["Value"] = canBeWritten(id);

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpacaID, &getID});

  alpaca
      .on("/api/v1/switch/0/getswitch", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();
            int id = request->getAttribute("id").toInt();

            doc["Value"] = SwitchObjects[id]->status() ? true : false;

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpacaID, &getID});

  alpaca
      .on("/api/v1/switch/0/getswitchdescription", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();
            int id = request->getAttribute("id").toInt();

            doc["Value"] = SwitchObjects[id]->getDescription();

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpacaID, &getID});

  alpaca
      .on("/api/v1/switch/0/getswitchname", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();
            int id = request->getAttribute("id").toInt();

            doc["Value"] = SwitchObjects[id]->getName();

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpacaID, &getID});

  alpaca
      .on("/api/v1/switch/0/getswitchvalue", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();
            int id = request->getAttribute("id").toInt();

            doc["Value"] = SwitchObjects[id]->status();

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpacaID, &getID});

  alpaca
      .on("/api/v1/switch/0/minswitchvalue", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();
            int id = request->getAttribute("id").toInt();

            doc["Value"] = SwitchObjects[id]->getMin();

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpacaID, &getID});

  alpaca
      .on("/api/v1/switch/0/maxswitchvalue", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();
            int id = request->getAttribute("id").toInt();

            doc["Value"] = SwitchObjects[id]->getMax();

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpacaID, &getID});

  alpaca
      .on("/api/v1/switch/0/switchstep", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] = 1;

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpacaID, &getID});

  alpaca
      .on("/api/v1/switch/0/statechangecomplete", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();
            int id = request->getAttribute("id").toInt();

            doc["Value"] = true;

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpacaID, &getID});

  alpaca
      .on("/api/v1/switch/0/devicestate", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();
            JsonArray Value = doc["Value"].to<JsonArray>();

            for (int i = 0; i < Switch.config.configuredSwitch; i++) {
              JsonObject getSwitch = Value.add<JsonObject>();
              char message[15];
              sprintf(message, "GetSwitch%d", i);
              getSwitch["Name"] = message;
              getSwitch["Value"] = SwitchObjects[i]->status() ? true : false;
            }
            // getSwitchValue
            for (int i = 0; i < Switch.config.configuredSwitch; i++) {
              JsonObject getSwitchValue = Value.add<JsonObject>();
              char message[20];
              sprintf(message, "GetSwitchValue%d", i);
              getSwitchValue["Name"] = message;
              getSwitchValue["Value"] = SwitchObjects[i]->status();
            }
            // StateChangeComplete
            for (int i = 0; i < Switch.config.configuredSwitch; i++) {
              JsonObject StateChangeComplete = Value.add<JsonObject>();
              char message[25];
              sprintf(message, "StateChangeComplete%d", i);
              StateChangeComplete["Name"] = message;
              StateChangeComplete["Value"] = true;
            }

            response->setLength();
            request->send(response);
          })
      .addMiddleware(&getAlpacaID);

  alpaca
      .on("/api/v1/switch/0/setswitch", HTTP_PUT,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();
            int state = request->getAttribute("state").toInt();
            int id = request->getAttribute("id").toInt();
            int value = 0;
            if (state) {
              value = SwitchObjects[id]->getMax();
            }

            if (SwitchObjects[id]->getType() == SwTypeServo) {
              ServoOutput *servo =
                  static_cast<ServoOutput *>(SwitchObjects[id]);
              servo->goTo(value, false, true);
            } else {
              SwitchObjects[id]->write(value);
            }

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpacaID, &getID, &getState, &isSettable});

  alpaca
      .on("/api/v1/switch/0/setasync", HTTP_PUT,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            int state = request->getAttribute("state").toInt();
            int id = request->getAttribute("id").toInt();
            int value = 0;
            if (state) {
              value = SwitchObjects[id]->getMax();
            }
            if (SwitchObjects[id]->getType() == SwTypeServo) {
              ServoOutput *servo =
                  static_cast<ServoOutput *>(SwitchObjects[id]);
              servo->goTo(value, false, true);
            } else {
              SwitchObjects[id]->write(value);
            }

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpacaID, &getID, &getState, &isSettable});

  alpaca
      .on("/api/v1/switch/0/setswitchvalue", HTTP_PUT,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            int value = request->getAttribute("value").toInt();
            int id = request->getAttribute("id").toInt();

            if (SwitchObjects[id]->getType() == SwTypeServo) {
              ServoOutput *servo =
                  static_cast<ServoOutput *>(SwitchObjects[id]);
              servo->goTo(value, true);
            } else {
              SwitchObjects[id]->write(value);
            }

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpacaID, &getID, &getValue, &isValueable});

  alpaca
      .on("/api/v1/switch/0/setasyncvalue", HTTP_PUT,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            int value = request->getAttribute("value").toInt();
            int id = request->getAttribute("id").toInt();

            if (SwitchObjects[id]->getType() == SwTypeServo) {
              ServoOutput *servo =
                  static_cast<ServoOutput *>(SwitchObjects[id]);
              servo->goTo(value, false, true);
            } else {
              SwitchObjects[id]->write(value);
            }

            response->setLength();
            request->send(response);
          })
      .addMiddlewares({&getAlpacaID, &getID, &getValue, &isValueable});

  /* Methods not implemented:*/
  alpaca
      .on("/api/v1/switch/0/commandblind", HTTP_PUT, alpacaMethodNotImplemented)
      .addMiddleware(&getAlpacaID);
  alpaca
      .on("/api/v1/switch/0/commandbool", HTTP_PUT, alpacaMethodNotImplemented)
      .addMiddleware(&getAlpacaID);
  alpaca
      .on("/api/v1/switch/0/commandstring", HTTP_PUT,
          alpacaMethodNotImplemented)
      .addMiddleware(&getAlpacaID);
  alpaca
      .on("/api/v1/switch/0/setswitchname", HTTP_PUT,
          alpacaMethodNotImplemented)
      .addMiddleware(&getAlpacaID);

  /* I don't care about connection but we need to declare it*/
  alpaca
      .on("/api/v1/switch/0/connect", HTTP_PUT,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);

            response->setLength();
            request->send(response);
          })
      .addMiddleware(&getAlpacaID);

  alpaca
      .on("/api/v1/switch/0/disconnect", HTTP_PUT,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);

            response->setLength();
            request->send(response);
          })
      .addMiddleware(&getAlpacaID);

  alpaca
      .on("/api/v1/switch/0/connecting", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] = false;

            request->send(response);
          })
      .addMiddleware(&getAlpacaID);

  alpaca
      .on("/api/v1/switch/0/connected", HTTP_GET,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);
            JsonObject doc = response->getRoot();

            doc["Value"] = true;

            response->setLength();
            request->send(response);
          })
      .addMiddleware(&getAlpacaID);

  alpaca
      .on("/api/v1/switch/0/connected", HTTP_PUT,
          [](AsyncWebServerRequest *request) {
            AsyncJsonResponse *response = prepareAlpacaResponse(request);

            response->setLength();
            request->send(response);
          })
      .addMiddleware(&getAlpacaID);

  alpaca.on("/api/v1/switch/0/supportedactions", HTTP_GET, alpacaNoActions)
      .addMiddleware(&getAlpacaID);
  alpaca.on("/api/v1/switch/0/action", HTTP_PUT, alpacaActionNotImplemented)
      .addMiddleware(&getAlpacaID);

#pragma endregion

#pragma region alpacaManager









#pragma endregion
}




void webApi(){

  server.on("/api/switch/cfg", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    doc["enable"] = Switches.isEnable();
    doc["order"] = Switches.uiOrder;
    doc["identifire"] = Switches.getIdentifier();

    JsonArray array = doc["Switches"].to<JsonArray>();

    /*
    for (int i = 0; i < Switch.config.configuredSwitch; i++) {
      if (SwitchObjects[i] == nullptr) {
        continue;
      }
      JsonObject jsonSwitch = array.add<JsonObject>();
      jsonSwitch["name"] = SwitchObjects[i]->getName();
      jsonSwitch["desc"] = SwitchObjects[i]->getDescription();
      jsonSwitch["type"] = SwitchObjects[i]->getType();
      jsonSwitch["pin"] = SwitchObjects[i]->getPinNumber();
      // uncommon parameters

      // digital input
      if (SwitchObjects[i]->getType() == SwTypeDInput) {
        // digital input
        DigitalInput *input = static_cast<DigitalInput *>(SwitchObjects[i]);
        jsonSwitch["dOn"] = input->dOn;
        jsonSwitch["dOff"] = input->dOff;
        jsonSwitch["invert"] = input->invert;

      } else if (SwitchObjects[i]->getType() == SwTypeDOutput) {
        // digital output
        DigitalOutput *output = static_cast<DigitalOutput *>(SwitchObjects[i]);
        jsonSwitch["invert"] = output->invert;

      } else if (SwitchObjects[i]->getType() == SwTypeServo) {
        // servo
        ServoOutput *servo = static_cast<ServoOutput *>(SwitchObjects[i]);
        jsonSwitch["moveTime"] = servo->getMovingTime();
      }
    }
      */
    response->setLength();
    request->send(response);
  });

  server.on("/api/switch/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonObject doc = response->getRoot().to<JsonObject>();
    JsonArray array = doc["Switches"].to<JsonArray>();
    for (int i = 0; i < Switch.config.configuredSwitch; i++) {
      if (SwitchObjects[i] == nullptr) {
        continue;
      }
      JsonObject jsonSwitch = array.add<JsonObject>();
      jsonSwitch["name"] = SwitchObjects[i]->getName();
      jsonSwitch["desc"] = SwitchObjects[i]->getDescription();
      jsonSwitch["type"] = SwitchObjects[i]->getType();
      jsonSwitch["min"] = SwitchObjects[i]->getMin();
      jsonSwitch["max"] = SwitchObjects[i]->getMax();
      jsonSwitch["boValue"] = SwitchObjects[i]->status() ? true : false;
      jsonSwitch["intValue"] = SwitchObjects[i]->status();
    }

    response->setLength();
    request->send(response);
  });

}
#endif