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
    
    
        addValidationErrorClass(id){
            const el = document.getElementById(id);
            if (el) {el.classList.add('validation_error');
                    el.scrollIntoView({ behavior: 'smooth', block: 'center' });
                }
        },
    
        removeValidationErrorClass(id){
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