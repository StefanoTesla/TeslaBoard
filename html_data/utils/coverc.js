export function coverc(){
    return {

        getCoverCStatus(){
            const ip = import.meta.env.VITE_BOARD_IP
            fetch(ip+'/api/coverc/status')
            .then(response => response.json())
            .then(data => {
                this.coverC = data;
                this.load.cover = true
                setTimeout(() => {this.getCoverCStatus()}, 3000)
            })
            .catch(error => {
                console.error('Error fetching board data:', error)
                setTimeout(() => {this.getCoverCStatus()}, 10000) }
            );
        },
    
        coverClose(){
            const ip = import.meta.env.VITE_BOARD_IP
            fetch(ip + '/api/coverc/close', {
                method: 'POST',
                headers: {
                  'Accept': 'application/json, text/plain, */*',
                },
              }).then(res => res.json())
                .then(res => {
                    if(res.execute){
                        this.addToast({ type:"success", text: this.text.gen.cmdAck })
                        this.coverC.cover.status = 2;
                    } else {
                        if(res.error) {
                            this.addToast({ type:"error", text:this.text.errors?.[res.error] || "undefined error", time:3})
                        } else {
                            this.addToast({ type:"error", text: this.text.gen.cmdRefused,time:3})
                        }
                    }

                })
        },
        coverOpen(){
            const ip = import.meta.env.VITE_BOARD_IP
            fetch(ip + '/api/coverc/open', {
                method: 'POST',
                headers: {
                  'Accept': 'application/json, text/plain, */*',
                },
              }).then(res => res.json())
                .then(res => {
                    if(res.execute){
                        this.addToast({ type:"success", text: this.text.gen.cmdAck })
                        this.coverC.cover.status = 2;
                    }
                    if(res.error) {
                        this.addToast({ type:"error", text:this.text.errors?.[res.error] || "undefined error", time:3})
                    }
    
                })
                .catch(err => {
                        console.log("Errore sconosciuto:", err);
                        this.addToast({ type: "error", text: "Errore sconosciuto." });
                });
        },
    
        calibratorBrightness(value){
            const ip = import.meta.env.VITE_BOARD_IP
            fetch(ip + '/api/coverc/brightness', {
                method: 'POST',
                headers: {
                  'Accept': 'application/json, text/plain, */*',
                  'Content-Type': 'application/x-www-form-urlencoded'
                },
                body:"brightness="+value
              }).then(res => res.json())
                .then(res => {
                    if(res.execute){
                        this.addToast({ type:"success", text: this.text.gen.cmdAck })
                        this.coverC.calibrator.brightness = value;
                    }
                    if(res.error) {
                        this.addToast({ type:"error", text:this.text.errors?.[res.error] || "undefined error", time:3})
                    }
                })
                .catch(err => {
                        console.log("Errore sconosciuto:", err);
                });
        },
    
        calibratorOff(){
            const ip = import.meta.env.VITE_BOARD_IP
            fetch(ip + '/api/coverc/off', {
                method: 'POST',
                headers: {
                  'Accept': 'application/json, text/plain, */*',
                },
              }).then(res => res.json())
                .then(res => {
                    if (res.execute){
                        this.addToast({ type:"success", text: this.text.gen.cmdAck })
                        this.coverC.calibrator.brightness = 0;
                    }
                })
                .catch(err => {
                    try {
                        const errorData = JSON.parse(err.message);
                        console.log("Errors:", errorData.errors);
                        this.addToast({ type: "error", text: "Errore: " + errorData.errors.join(", ") });
                    } catch (parseError) {
                        console.log("Errore sconosciuto:", err);
                        this.addToast({ type: "error", text: "Errore sconosciuto." });
                    }
                });
        },
    
        calibratorOn(){
            const ip = import.meta.env.VITE_BOARD_IP
            fetch(ip + '/api/coverc/on', {
                method: 'POST',
                headers: {
                  'Accept': 'application/json, text/plain, */*',
                },
              }).then(res => res.json())
                .then(res => {
                    if (res.execute){
                        this.addToast({ type:"success", text: this.text.gen.cmdAck })
                        this.coverC.calibrator.brightness = 4095;
                    }
                })
                .catch(err => {
                    try {
                        const errorData = JSON.parse(err.message);
                        console.log("Errors:", errorData.errors);
                        this.addToast({ type: "error", text: "Errore: " + errorData.errors.join(", ") });
                    } catch (parseError) {
                        console.log("Errore sconosciuto:", err);
                        this.addToast({ type: "error", text: "Errore sconosciuto." });
                    }
                });
        },
    
        getOriginalCoverCConfig(){
            this.coverC = this.copy(this.coverCOrig)

        },


        getCoverCConfig(){
            const ip = import.meta.env.VITE_BOARD_IP
            fetch(ip +'/api/coverc/cfg')
            .then(response => response.json())
            .then(data => {

                this.coverCOrig = this.copy(data);
                this.coverC = data;
                this.load.cover = true;
            })
            .catch(error => console.error('Error fetching board data:', error));
        },

        validateCoverC(){
            let valid = true
            this.parseObjectToInt(this.coverC)

            if(this.coverC.calibrator.present){
                if(this.invalidOutputPin( this.coverC.calibrator.pin, "coverc_calibrator_out" )){ valid = false }
            } else {
                this.removeValidationErrorClass("coverc_calibrator_out")
            }
            if(this.coverC.cover.present){
                if(this.invalidOutputPin(this.coverC.cover.pin,"coverc_cover_out")){ valid = false }
                if(this.negativeValue(this.coverC.cover.maxDeg)){ valid = false }
                if(this.negativeValue(this.coverC.cover.closeDeg)){ valid = false }
                if(this.negativeValue(this.coverC.cover.openDeg) ){ valid = false }
                if(this.coverC.cover.maxDeg>360) { this.addValidationErrorClass("coverc_cover_max_deg");  valid = false } else { this.removeValidationErrorClass("coverc_cover_max_deg") }
                if(this.coverC.cover.closeDeg > this.coverC.cover.maxDeg){ this.addValidationErrorClass("coverc_cover_close_deg");  valid = false } else { this.removeValidationErrorClass("coverc_cover_close_deg") }
                if( this.coverC.cover.openDeg > this.coverC.cover.maxDeg){ this.addValidationErrorClass("coverc_cover_open_deg");  valid = false } else { this.removeValidationErrorClass("coverc_cover_open_deg") }
            } else {
                this.removeValidationErrorClass("coverc_cover_out")
                this.removeValidationErrorClass("coverc_cover_max_deg")
                this.removeValidationErrorClass("coverc_cover_close_deg")
                this.removeValidationErrorClass("coverc_cover_open_deg")
            }
            return valid
        
        },

        saveCoverCSetting(){
            if(this.validateCoverC()){
                const ip = import.meta.env.VITE_BOARD_IP
                fetch(ip + '/api/coverc/cfg', {
                    method: 'POST',
                    headers: {
                    'Accept': 'application/json, text/plain, */*',
                    'Content-Type': 'application/json'
                    },
                    body: JSON.stringify(this.coverC)
                }).then(res => {
                    // Controllo dello stato HTTP
                    if (!res.ok) {
                        // Analizzo la risposta JSON anche per errori 500
                        return res.json().then(errorResponse => {
                            throw { status: res.status, ...errorResponse };
                        });
                    }
                    return res.json();
                })
                .then(res => {
                    if(res.reboot){
                        this.reboot.cover = res.reboot
                        this.modal = true
                    } else {
                        this.reboot.cover = false
                    }
                    
                    this.addToast({ type: "success", text: this.text.gen.configSaved, time:3 });
                })
                .catch(err => {
                    if (err.errors) {
                        err.errors.forEach((error, index) => {
                            setTimeout(() => {
                                console.log(`Errore ${index + 1}: ${error}`);
                                this.addToast({ type: "error", text: `Errore: ${error}` });
                            }, 1 * index);  // put a delay to avoid toat crash
                        });
                    } else {
                        console.log("Errore sconosciuto:", err);
                        this.addToast({ type: "error", text: "Errore sconosciuto." });
                    }
                });
            }
        }

    }
}