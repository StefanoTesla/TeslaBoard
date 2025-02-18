export function dome(){
    return {

    getOriginalDomeConfig(){
        this.dome = this.copy(this.domeOrig)

    },

    getDomeConfig(){
        const ip = import.meta.env.VITE_BOARD_IP
        fetch(ip + '/api/dome/cfg')
        .then(response => response.json())
        .then(data => {

            this.domeOrig = this.copy(data);
            this.dome = data;
            this.load.dome = true;
            
        })
        .catch(error => console.error('Error fetching board data:', error));
    },


    saveDomeSetting(){
        if(this.validateDome()){
            const ip = import.meta.env.VITE_BOARD_IP
            fetch(ip + '/api/dome/cfg', {
                method: 'POST',
                headers: {
                  'Accept': 'application/json, text/plain, */*',
                  'Content-Type': 'application/json'
                },
                body: JSON.stringify(this.dome)
                })
                .then(async res => {
                    if (!res.ok) { 
                        const errorData = await res.json();
                        throw new Error(JSON.stringify(errorData)); 
                    }
                    return res.json();
                })
                .then(res => {
                    this.addToast({ type:"success", text: this.text.gen.configSaved, time:3 })
                    this.reboot.dome = res.reboot ? true : false
                    this.modal = res.reboot

                })
                .catch(err => {
                    try {
                        const errorData = JSON.parse(err.message);
                        console.log("Errors:", errorData.errors);
                        this.addToast({ type: "error", text: "Errore: " + errorData.errors.join(", "), time:3 });
                    } catch (parseError) {
                        console.log("Errore sconosciuto:", err);
                        this.addToast({ type: "error", text: "Errore sconosciuto.", time:3 });
                    }
                });
        }
    },

    validateDome(){
        let valid = true
        this.parseObjectToInt(this.dome)
        //input open
        if(this.invalidInputPin(this.dome.pinOpen.pin,"dome_in_open")){ valid = false}
        if(this.negativeValue( this.dome.pinOpen.dOn,"dome_in_open_don")){ valid = false }
        if(this.negativeValue( this.dome.pinOpen.dOff,"dome_in_open_doff")){ valid = false }
        if(this.negativeValue( this.dome.pinOpen.invert,"dome_in_open_invert" || this.greaterThen(this.dome.pinOpen.invert,1,"dome_in_open_invert"))){ valid = false }

        //input close
        if(this.invalidInputPin( this.dome.pinClose.pin,"dome_in_close")){ valid = false }
        if(this.negativeValue( this.dome.pinClose.dOn,"dome_in_close_don")){ valid = false }
        if(this.negativeValue( this.dome.pinClose.dOff,"dome_in_close_doff")){ valid = false }
        if(this.negativeValue( this.dome.pinOpen.invert,"dome_in_close_invert" || this.greaterThen(this.dome.pinOpen.invert,1,"dome_in_close_invert"))){ valid = false }
        //outputs
        if(this.invalidOutputPin( this.dome.pinStart.pin,"dome_out_start")){ valid = false }
        if(this.negativeValue( this.dome.pinStart.invert,"dome_out_start_invert" || this.greaterThen(this.dome.pinHalt.invert,1,"dome_out_start_invert"))){ valid = false }

        if(this.invalidOutputPin( this.dome.pinHalt.pin,"dome_out_halt")){ valid = false }
        if(this.negativeValue( this.dome.pinHalt.invert,"dome_out_halt_invert") || this.greaterThen(this.dome.pinHalt.invert,1,"dome_out_halt_invert")){ valid = false }
        //timers
        if(this.negativeValue( this.dome.movTimeOut,"dome_timeout")){ valid = false }
        if(this.negativeValue( this.dome.autoclose.minutes,"dome_autoclose_time")){ valid = false }
        return valid
    },

    
    getDomeStatus(){
        const ip = import.meta.env.VITE_BOARD_IP
        fetch(ip+'/api/dome/status')
        .then(response => response.json())
        .then(data => {
            this.dome = data;
            this.load.dome = true;
            setTimeout(() => {this.getDomeStatus()}, 3000)
        })
        .catch(error => {
            console.error('Error fetching board data:', error)
            setTimeout(() => {this.getDomeStatus()}, 10000)
        }
        );
    },

    openDome(){
        const ip = import.meta.env.VITE_BOARD_IP
        fetch(ip + '/api/dome/open', {
            method: 'POST',
            headers: {
              'Accept': 'application/json, text/plain, */*',
              'Content-Type': 'application/x-www-form-urlencoded'
            },
        })
        .then(res => res.json())
        .then(res => {
            if(res.execute){
                this.addToast({ type:"success", text: this.text.gen.cmdAck})
            } else {
                if(res.error){
                    this.addToast({ type:"error", text:this.text.errors?.[res.error] || "undefined error", time:3})
                } else {
                    this.addToast({ type:"error", text: this.text.gen.cmdRefused,time:3})
                }
            }
        })
        .catch(error => console.error('Error opening dome: ', error));
    },

    closeDome(){
        const ip = import.meta.env.VITE_BOARD_IP
        fetch(ip + '/api/dome/close', {
            method: 'POST',
            headers: {
              'Accept': 'application/json, text/plain, */*',
              'Content-Type': 'application/x-www-form-urlencoded'
            },
        })
        .then(res => res.json())
        .then(res => {
            if(res.execute){
                this.addToast({ type:"success", text: this.text.gen.cmdAck})
            } else {
                this.addToast({ type:"error", text: this.text.gen.cmdRefused})
            }
        })
        .catch(error => console.error('Error opening dome: ', error));
    },

    haltDome(){
        const ip = import.meta.env.VITE_BOARD_IP
        fetch(ip + '/api/dome/halt', {
            method: 'POST',
            headers: {
              'Accept': 'application/json, text/plain, */*',
              'Content-Type': 'application/x-www-form-urlencoded'
            },
        })
        .then(res => res.json())
        .then(res => {
            if(res.execute){
                this.addToast({ type:"success", text: this.text.gen.cmdAck})
            } else {
                this.addToast({ type:"error", text: this.text.gen.cmdRefused})
            }
        })
        .catch(error => console.error('Error halting dome: ', error));
    }
    }
}