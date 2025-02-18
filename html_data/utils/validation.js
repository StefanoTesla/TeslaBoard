export function validation() {
    return {

        parseObjectToInt(obj){
            for (let key in obj) {
                if (obj.hasOwnProperty(key)) {
                    if (typeof obj[key] === 'object' && obj[key] !== null) {
                        // Se è un oggetto, chiamare ricorsivamente
                        this.parseObjectToInt(obj[key]);
                    } else {
                        // Se è un valore primitivo (numero o stringa), applica parseInt
                        if(typeof obj[key] !== 'boolean' ){
                            obj[key] = parseInt(obj[key], 10);
                        }
                       
                    }
                }
            }
        },
        
        invalidOutputPin(pin,divclass){
            const noUsablePin = [6,7,8,9,10,11,20,24,28,29,30,31,37,38]
            const noUsableOutputPin= [3,34,35,36,39];
            if(isNaN(pin) || pin==null){
                console.log(pin,divclass)
                this.addToast({type:"error", text:"Fill the gap" , time:3 })
                this.addValidationErrorClass(divclass);
                return true
            }
            if(noUsablePin.includes(pin) || noUsableOutputPin.includes(pin) || pin<0 || pin > 39){
                this.addToast({type:"error", text:"PIN " + pin + " "+ this.text.errors.noUsableAsOutput , time:3 })
                this.addValidationErrorClass(divclass);
                return true
            }
            this.removeValidationErrorClass(divclass);
            return false
        },
    
        invalidInputPin(pin,divclass){
            const noUsablePin = [6,7,8,9,10,11,20,24,28,29,30,31,37,38]
            const noUsableOutpuPin= [1];
            if(isNaN(pin) || pin==null){                
                this.addToast({type:"error", text:"Fill the gap" , time:3 })
                this.addValidationErrorClass(divclass);
                return true
            }
            if(noUsablePin.includes(pin) || noUsableOutpuPin.includes(pin) || pin<0 || pin > 39){
                this.addToast({type:"error", text:"PIN " + pin + " "+ this.text.errors.noUsableAsInput, time:3 })
                this.addValidationErrorClass(divclass);
                return true
            }
            this.removeValidationErrorClass(divclass);
            return false
        },
    
        negativeValue(value,divclass){

            if(isNaN(value) || value<0){
                console.log(value);
                console.log(divclass);
                this.addValidationErrorClass(divclass);
                return true
            }
            this.removeValidationErrorClass(divclass);
            return false
        },

        greaterThen(value,maxValue,divclass){
            if(value>maxValue){
                this.addValidationErrorClass(divclass);
                return true
            }
            this.removeValidationErrorClass(divclass);
            return false
        },
    
        addValidationErrorClass(id){
            console.log("metto")
            const el = document.getElementById(id);
            if (el) {el.classList.add('validation_error');
                    console.log("metto")
                    el.scrollIntoView({ behavior: 'smooth', block: 'center' });
                    console.log("scrollo")
                }
        },
    
        removeValidationErrorClass(id){
            console.log("tolgo")
            const el = document.getElementById(id);
            if (el) el.classList.remove('validation_error');
        },


        ipAddress(value,divclass){
            value = parseInt(value)
            if(value<0 || value>255 || value == null || isNaN(value)){
                this.addValidationErrorClass(divclass)
                return true
                ;
            }
            this.removeValidationErrorClass(divclass);
            return false
        },
    
    
    }
}