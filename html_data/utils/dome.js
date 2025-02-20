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
        if(!this.validateDome()){
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
        this.parseObjectToInt(this.dome)
        //input open
        let err = false;

        //input open
        err |= new Validator(this.dome.pinOpen.pin,"dome_in_open").invalidInputPin().evaluate()
        err |= new Validator(this.dome.pinOpen.dOn,"dome_in_open_don").negativeValue().evaluate()
        err |= new Validator(this.dome.pinOpen.dOff,"dome_in_open_doff").negativeValue().evaluate()
        err |= new Validator(this.dome.pinOpen.invert,"dome_in_open_invert").negativeValue().greaterThen(1).evaluate()
        //input close
        err |= new Validator(this.dome.pinClose.pin,"dome_in_close").invalidInputPin().evaluate()
        err |= new Validator(this.dome.pinClose.dOn,"dome_in_close_don").negativeValue().evaluate()
        err |= new Validator(this.dome.pinClose.dOff,"dome_in_close_doff").negativeValue().evaluate()
        err |= new Validator(this.dome.pinClose.invert,"dome_in_close_invert").negativeValue().greaterThen(1).evaluate()

        //outputs
        err |= new Validator(this.dome.pinStart.pin,"dome_out_start").invalidInputPin().evaluate()
        err |= new Validator(this.dome.pinStart.invert,"dome_out_start_invert").negativeValue().greaterThen(1).evaluate()

        //outputs
        err |= new Validator(this.dome.pinHalt.pin,"dome_out_halt").invalidInputPin().evaluate()
        err |= new Validator(this.dome.pinHalt.invert,"dome_out_halt_invert").negativeValue().greaterThen(1).evaluate()

        //timers
        err |= new Validator(this.dome.movTimeOut,"dome_timeout").negativeValue().greaterThen(1000).evaluate()
        err |= new Validator(this.dome.autoclose.minutes,"dome_autoclose_time").negativeValue().greaterThen(1).evaluate()

        return err
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