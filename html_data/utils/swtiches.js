export function switches(){
    return{
    
    /* home page */
    getSwitchStatus(){
        const ip = import.meta.env.VITE_BOARD_IP
        fetch(ip +'/api/switch/status')
        .then(response => response.json())
        .then(data => {
            this.swi = data;
            this.load.switch = true;
            setTimeout(() => {this.getSwitchStatus()}, 3000)
        })
        .catch(error => {
            console.error('Error fetching board data:', error)
            setTimeout(() => {this.getSwitchStatus()}, 10000)
        });
    },

    switchChangeDigital(index){
        if(this.swi.Switches[index].boValue){
            this.swi.Switches[index].intValue = 1
        } else {
            this.swi.Switches[index].intValue = 0
        }
        const ip = import.meta.env.VITE_BOARD_IP
        fetch(ip + '/api/switch/set-value', {
            method: 'POST',
            headers: {
              'Accept': 'application/json, text/plain, */*',
              'Content-Type': 'application/x-www-form-urlencoded'
            },
            body:"id="+index+"&value="+this.swi.Switches[index].intValue
        })
        .then(response => response.json())
        .then(data => {
            if(data.execute){
                this.addToast({ type:"success", text: this.text.gen.cmdAck })
            } else {
                if(data.error){
                    this.addToast({ type:"error", text:this.text.errors?.[data.error] || "undefined error", time:3})
                } else {
                    this.addToast({ type:"error", text: this.text.gen.cmdRefused,time:3})
                }
                
            }
        })
        .catch(error => console.error('Error change switch value:', error));

    },
    switchChangeValue(index){
        this.swi.Switches[index].intValue = parseInt(this.swi.Switches[index].intValue)
        const ip = import.meta.env.VITE_BOARD_IP
        fetch(ip + '/api/switch/set-value', {
            method: 'POST',
            headers: {
              'Accept': 'application/json, text/plain, */*',
              'Content-Type': 'application/x-www-form-urlencoded'
            },
            body:"id="+index+"&value="+this.swi.Switches[index].intValue
        })
        .then(response => response.json())
        .then(data => {
            if(data.execute){
                this.addToast({ type:"success", text: this.text.gen.cmdAck })
            } else {
                if(data.error){
                    this.addToast({ type:"error", text:this.text.errors?.[data.error] || "undefined error", time:3})
                } else {
                    this.addToast({ type:"error", text: this.text.gen.cmdRefused,time:3})
                }
                
            }
        })
        .catch(error => console.error('Error change switch value:', error));
    },
    
    /* switch config */
    getSwitchConfig(){
        const ip = import.meta.env.VITE_BOARD_IP
        fetch(ip +'/api/switch/cfg')
        .then(response => response.json())
        .then(data => {

            this.switchCOrig = this.copy(data);
            this.swi = data;
            this.load.switch = true;
        })
        .catch(error => console.error('Error fetching board data:', error));
    },

    getOriginalSwitchesConfig(){
        this.swi = this.copy(this.switchCOrig)
    },

    addNewSwitch(){
        if(this.swi.Switches.length <= 14){
            this.swi.Switches.push({"name":"","desc":"","type":0,"pin":0})
        } else {
            this.addToast({type:"error", text:"Limite Raggiunto" })
        }
        
    },

    deleteSwitch(id){
        this.swi.Switches.splice(id,1)
    },

    moveUp(id){
        [this.swi.Switches[id], this.swi.Switches[id - 1]] = [this.swi.Switches[id - 1], this.swi.Switches[id]];
    },

    moveDown(id){
        [this.swi.Switches[id], this.swi.Switches[id + 1]] = [this.swi.Switches[id + 1], this.swi.Switches[id]];
    },

    validateSwitch(index){
        this.swi.Switches[index].name = this.swi.Switches[index].name.replace(/[@#$*<>:;!]/g, '')
        this.swi.Switches[index].desc = this.swi.Switches[index].desc.replace(/[@#$*<>:;!]/g, '')
        this.swi.Switches[index].type = parseInt(this.swi.Switches[index].type)
        this.swi.Switches[index].pin = parseInt(this.swi.Switches[index].pin)


        switch (this.swi.Switches[index].type) {
            case 0:
                return true;
                break;
            case 1: //di
                this.swi.Switches[index].invert = parseInt(this.swi.Switches[index].invert)
                this.swi.Switches[index].dOn = parseInt(this.swi.Switches[index].dOn)
                this.swi.Switches[index].dOff = parseInt(this.swi.Switches[index].dOff)
                if(this.invalidInputPin(this.swi.Switches[index].pin,'sw_'+ index +'_pin')){ return false }
                if(this.negativeValue(this.swi.Switches[index].dOn,'sw_'+ index +'_dOn')){ return false }
                if(this.negativeValue(this.swi.Switches[index].dOff,'sw_'+ index +'_dOff')){ return false }
                if(this.negativeValue(this.swi.Switches[index].invert,'sw_'+ index +'_invert')){ console.log("failed"); return false }
                if(this.greaterThen(this.swi.Switches[index].invert,1,'sw_'+ index +'_invert')){ console.log("failed"); return false }
                return true
                break;
            case 2://do
                this.swi.Switches[index].invert = parseInt(this.swi.Switches[index].invert)
                console.log("chek DO setting")
                if(this.invalidOutputPin(this.swi.Switches[index].pin,'sw_'+ index +'_pin')){ return false }
                if(this.negativeValue(this.swi.Switches[index].invert,'sw_'+ index +'_invert')){ return false }
                if(this.greaterThen(this.swi.Switches[index].invert,1,'sw_'+ index +'_invert')){ return false }
                return true
            case 4://servo
                this.greaterThen(this.swi.Switches[index].maxDeg,270,'sw_'+ index +'_max')
                if(this.swi.Switches[index].maxDeg <= this.swi.Switches[index].min){
                    this.addValidationErrorClass('sw_'+ index +'_max')
                    return false
                } else {
                    this.removeValidationErrorClass('sw_'+ index +'_max')
                }
                if(this.swi.Switches[index].min >= this.swi.Switches[index].max){
                    this.addValidationErrorClass('sw_'+ index +'_min')
                    return false
                } else {
                    this.removeValidationErrorClass('sw_'+ index +'_min')
                }
            case 3://pwm
                if(this.invalidOutputPin(this.swi.Switches[index].pin,'sw_'+ index +'_pin')){ return false }
                return true
                break;

            default:
                // codice da eseguire se nessun case corrisponde
                break;
        }

    },



    saveSwitchesSetting(){
  
    let error = false;
        this.swi.Switches.forEach((element,index) => {
            if(!this.validateSwitch(index)){
                error = true;
            }
        });
    if(error){
        return;
    }

    const ip = import.meta.env.VITE_BOARD_IP
    fetch(ip + '/api/switch/cfg', {
        method: 'POST',
        headers: {
          'Accept': 'application/json, text/plain, */*',
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(this.swi)
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
        this.reboot.switch = res.reboot
        this.modal = res.reboot 
        this.addToast({ type: "success", text: this.text.gen.configSaved, time:3 });
    })
    .catch(err => {
        if (err.errors) {
            err.errors.forEach((error, index) => {
                setTimeout(() => {
                    console.log(`Errore ${index + 1}: ${error}`);
                    this.addToast({ type: "error", text: `Errore: ${error}`, time:3});
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