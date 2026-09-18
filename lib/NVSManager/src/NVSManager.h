#ifndef NVS_MANAGER_H
#define NVS_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include <esp_err.h>
// Enumerativo abbreviato e pulito
enum NvsState { NVS_CLOSED, NVS_READ, NVS_WRITE };

class NvsManager {
private:
    Preferences nvs;
    NvsState nvsStatus;
    const char* currentNamespace; // Puntatore puro per azzerare lo spreco di RAM
    bool isBusy;                  // Flag per il controllo manuale dei moduli

    // Costruttore privato (Pattern Singleton)
    NvsManager() {
        nvsStatus = NVS_CLOSED;
        currentNamespace = "";
        isBusy = false;

        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            Serial.println("W: NVS corrotta o nuova versione rilevata. Formatto la flash...");
            if (nvs_flash_erase() == ESP_OK) {
                ret = nvs_flash_init();
            }
        }

        if (ret != ESP_OK) {
            Serial.printf("E: Errore critico hardware inizializzazione NVS: 0x%X\n", ret);
        } else {
            Serial.println("I: Hardware NVS inizializzato con successo.");
        }
    };

public:
    NvsManager(const NvsManager&) = delete;
    void operator=(const NvsManager&) = delete;

    static NvsManager& getInstance() {
        static NvsManager instance;
        return instance;
    }


    bool isModuleBusy() { return isBusy; }
    void lockModule()   { isBusy = true; }
    void unlockModule() { isBusy = false; }

    bool openNVS(bool readOnly, const char* namespaceName) {
        if (strlen(namespaceName) > 15) {
            Serial.println("E: Il nome del namespace supera i 15 caratteri!");
            return false;
        }

        if (isBusy && nvsStatus != NVS_CLOSED && strcmp(currentNamespace, namespaceName) != 0) {
            Serial.println("E: Tentativo di accesso negato. NVS occupata da un altro modulo!");
            return false; 
        }

        if (nvsStatus != NVS_CLOSED && strcmp(currentNamespace, namespaceName) != 0) {
            closeNVS();
        }


        switch (nvsStatus) {
        case NVS_CLOSED:
            if (nvs.begin(namespaceName, readOnly)) {
                currentNamespace = namespaceName;
                nvsStatus = readOnly ? NVS_READ : NVS_WRITE;
                isBusy = true;
                return true;
            }
            return false;

        case NVS_READ:
            if (!readOnly) {
                closeNVS();
                if (nvs.begin(namespaceName, false)) {
                    currentNamespace = namespaceName;
                    nvsStatus = NVS_WRITE;
                    isBusy = true;
                    return true;
                }
                return false;
            }
            return true;

        case NVS_WRITE:
            if (readOnly) {
                closeNVS();
                if (nvs.begin(namespaceName, true)) {
                    currentNamespace = namespaceName;
                    nvsStatus = NVS_READ;
                    isBusy = true;
                    return true;
                }
                return false;
            }
            return true;
        }
        return false;
    }

    void closeNVS() {
        if (nvsStatus != NVS_CLOSED) {
            nvs.end();
            nvsStatus = NVS_CLOSED;
            currentNamespace = "";
            isBusy = false;
        }
    }

    int getInt(const char* key, int defaultValue = 0) { return nvs.getInt(key, defaultValue); }
    bool putInt(const char* key, int value) { return nvs.putInt(key, value) > 0; }

    bool getBool(const char* key, bool defaultValue = false) { return nvs.getBool(key, defaultValue); }
    bool putBool(const char* key, bool value) { return nvs.putBool(key, value) > 0; }

    String getString(const char* key, String defaultValue = "") { return nvs.getString(key, defaultValue); }
    bool putString(const char* key, String value) { return nvs.putString(key, value.c_str()) > 0; }

    bool removeKey(const char* key) { return nvs.remove(key);}
};

#endif