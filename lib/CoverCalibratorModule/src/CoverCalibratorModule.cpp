#include "CoverCalibratorModule.h"


/* initialize the dome */
void CoverCalibratorModule::begin(){
    JsonDocument doc;
    Preferences pref;
    
    if(!pref.begin(COVERC_SCHEMA_NAME,true)){
        initNVS(pref);
    };

    moduleEnable = pref.getBool("enable");
    uiOrder = pref.getInt("order",1);
    identifier = pref.getString("identifier",identifier);

    if(!moduleEnable){

        pref.end();
        return;
    }
    
    int schemaVersion = pref.getInt("schema");


    if(schemaVersion < COVERC_SCHEMA_VERSION){

        switch (schemaVersion)
        {
            case 0:
                updateNVS1(pref);
                break;
            
            default:
                break;
        }
    }

    /* basic data for CoverCalibratorModule is taken, module bening*/
    String cfg = pref.getString("calibrator","{}");
    
    DeserializationError error = deserializeJson(doc, cfg);

    if(!error){
        calibrator.begin(doc);
    }

    cfg.clear();
    cfg = pref.getString("cover","{}");
    

    error = deserializeJson(doc, cfg);
    pref.end();

    if(!error){
        cover.begin(doc);
    }

}

void CoverCalibratorModule::updateNVS1(Preferences pref){
    pref.end();
    pref.begin(COVERC_SCHEMA_NAME);
    pref.putBool("enable",false);
    pref.putInt("schema",1);
    pref.putInt("order",COVERC_SCHEMA_VERSION);
    pref.putString("calibrator","{}");
    pref.putString("cover","{}");
    pref.end();
    pref.begin(COVERC_SCHEMA_NAME,true);
}

void CoverCalibratorModule::initNVS(Preferences pref){

    if(!pref.begin(COVERC_SCHEMA_NAME, false)){
        Serial.println("unable to write NVS page");
        pref.end();
        return;
    };


    pref.putBool("enable",false);
    pref.putInt("order",1);
    pref.putInt("schema",1);
    pref.putString("calibrator","{}");
    pref.putString("cover","{}");

    pref.end();
    pref.begin(COVERC_SCHEMA_NAME,true);
    Serial.println("NVS initialized");
}

bool CoverCalibratorModule::isEnable(){
    return moduleEnable;
}


void CoverCalibratorModule::loop(){
    
    if(isEnable()){
        calibrator.loop();
        cover.loop();
    }
}


void CoverCalibratorModule::getConfiguration(JsonObject dest){

    dest["enable"] = moduleEnable;
    dest["uiOrder"] = uiOrder;
    dest["identifier"] = identifier;
    JsonObject calibObj = dest["calibrator"].to<JsonObject>();
    calibrator.getConfiguration(calibObj);
    JsonObject coverObj = dest["cover"].to<JsonObject>();
    cover.getConfiguration(coverObj);

}


void CoverCalibratorModule::validateConfiguration(const JsonObject &toBeValidated, JsonObject response){

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

    if(!toBeValidated["calibrator"].is<JsonObject>()){
        err.add("Calibrator Configuration is missing");
        return;
    }
    
    calibrator.validateConfiguration(toBeValidated["calibrator"],response);

    if(!toBeValidated["cover"].is<JsonObject>()){
        err.add("Calibrator Configuration is missing");
        return;
    }

    cover.validateConfiguration(toBeValidated["cover"],response);

}


void CoverCalibratorModule::storeConfiguration(JsonObject toBeStored){

    Preferences pref;

    pref.begin(COVERC_SCHEMA_NAME);

    bool inEnable = toBeStored["enable"].as<bool>();

    pref.putBool("enable",inEnable);
    pref.putInt("uiOrder",toBeStored["uiOrder"].as<int>());
    pref.putInt("schema",COVERC_SCHEMA_VERSION);
    pref.putString("identifier",toBeStored["identifier"].as<String>());
 
    pref.end();

    /* apply only the changes that don't require a reboot */
    uiOrder = toBeStored["uiOrder"].as<int>();
    identifier = toBeStored["identifier"].as<String>();

    /* if module is not enable don't write anymore*/
    if(!inEnable ){
        return;
    }

    calibrator.storeConfiguration(toBeStored["calibrator"],COVERC_SCHEMA_NAME);
    cover.storeConfiguration(toBeStored["cover"],COVERC_SCHEMA_NAME);

}