#include "Dome.h"


/* initialize the dome */
void DomeModule::begin(){
    JsonDocument doc;
    Preferences pref;
    
    if(!pref.begin(DOME_SCHEMA_NAME)){
        initNVS(pref);
    };

    moduleEnable = pref.getBool("enable");
    uiOrder = pref.getInt("order",1);
    identifier = pref.getString("identifier","Dome");

    if(!moduleEnable){

        pref.end();
        return;
    }
    
    int schemaVersion = pref.getInt("schema");
    Serial.print(schemaVersion);

    if(schemaVersion < DOME_SCHEMA_VERSION){

        switch (schemaVersion)
        {
            case 0:
                updateNVS1(pref);
                break;
            
            default:
                break;
        }
    }

    /* basic data for dome is taken, module bening*/


    String cfg = pref.getString("shutter","{}");
    
    DeserializationError error = deserializeJson(doc, cfg);
    pref.end();

    if(!error){
        shutter.begin(doc);
    }

    if(shutter.isEnable()){
        moduleEnable = true;
    } else {
        moduleEnable = false;
    }

}

void DomeModule::updateNVS1(Preferences pref){
    pref.end();
    pref.begin(DOME_SCHEMA_NAME);
    pref.putBool("enable",false);
    pref.putInt("schema",1);
    pref.putInt("order",DOME_SCHEMA_VERSION);
    pref.putString("shutter","{}");
    pref.end();
    pref.begin(DOME_SCHEMA_NAME,true);
}

void DomeModule::initNVS(Preferences pref){

    if(!pref.begin(DOME_SCHEMA_NAME, false)){
        Serial.println("unable to write NVS page");
        pref.end();
        return;
    };


    pref.putBool("enable",false);
    pref.putInt("order",1);
    pref.putInt("schema",1);
    pref.putString("shutter","{}");

    pref.end();
    pref.begin(DOME_SCHEMA_NAME,true);
    Serial.println("NVS initialized");
}

bool DomeModule::isEnable(){
    Serial.println("");
    Serial.print("Dome module is: ");
    Serial.println(moduleEnable);
    return moduleEnable;
}


void DomeModule::loop(){
    
    if(shutter.isEnable()){
        shutter.loop();
    }
}


void DomeModule::getConfiguration(JsonObject dest){

    dest["enable"] = moduleEnable;
    dest["uiOrder"] = uiOrder;
    dest["identifier"] = identifier;
    JsonObject shutterObj = dest["shutter"].to<JsonObject>();
    shutter.getConfiguration(shutterObj);

}


void DomeModule::validateConfiguration(const JsonObject &toBeValidated, JsonObject response){

    response["reboot"] = false;

    JsonArray err = response["errors"].to<JsonArray>();
    if(!toBeValidated["enable"].is<bool>()){
        err.add("Enable is not a boolean");
        return;
    }

    if(moduleEnable != toBeValidated["enable"]){
        response["reboot"] = true;
    }

    if(!toBeValidated["uiOrder"].is<int>()){
        err.add("Order is not a numeber");
        return;
    }

    if(!toBeValidated["identifier"].is<String>()){
        err.add("Identifier is not a string");
        return;
    }

    if(!toBeValidated["shutter"].is<JsonObject>()){
        err.add("Shutter Configuration is missing");
        return;
    }
    
    Serial.println("shutter exist");

    shutter.validateConfiguration(toBeValidated["shutter"],response);

}


void DomeModule::storeConfiguration(JsonObject toBeStored){

    Preferences pref;

    pref.begin(DOME_SCHEMA_NAME);

    bool inEnable = toBeStored["enable"].as<bool>();

    pref.putBool("enable",inEnable);
    pref.putInt("uiOrder",toBeStored["uiOrder"].as<int>());
    pref.putInt("schema",DOME_SCHEMA_VERSION);
    pref.putString("identifier",toBeStored["identifier"].as<String>());
 
    pref.end();

    /* apply only the changes that don't require a reboot */
    uiOrder = toBeStored["uiOrder"].as<int>();
    identifier = toBeStored["identifier"].as<String>();

    /* if module is not enable don't write anymore*/
    if(!inEnable ){
        return;
    }

    shutter.storeConfiguration(toBeStored["shutter"],DOME_SCHEMA_NAME);

}