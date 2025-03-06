export function checkFirmwareUpdate(){
    return {

        initFWUpdate(){
            let av = this.getCookie("version")
            if (av){
                this.version.available = av
                if(av > this.version.actual){
                    this.version.upgrade= true
                }
            } else {
                this.version.available = "?"
            }
        },

        checkFWUpdate(){
            
            let av = this.getCookie("version")
            if (av){
                this.version.available = av
                if(av > this.version.actual){
                    this.version.upgrade= true
                    this.addToast({type:"info",text:"New update availabe",time:30})
                return
                }
            this.fetchVersion()
            }
        },


        fetchVersion(){
            fetch('https://api.stefanotesla.it/api/teslaboard', {
                method: 'GET',
                headers: {
                  'Accept': 'application/json',
                },
            })
            .then(response => {
                if (response.status === 200) {
                  return response.json();
                } else {
                  throw new Error(`Errore: stato ${response.status}`);
                }
              })
            .then(res => {
                    console.log(res);
                    this.setCookie("version", res.version, 1)
                    this.version.available = res.version
                    if(res.version > this.version.actual){
                        this.version.upgrade= true
                        this.addToast({ type:"success", text: "new board update available!", time:10})
                    }
            })
            .catch(error => /*console.clear()*/console.log(error));
        },

        setCookie(name, value, days) {
            let date = new Date();
            date.setTime(date.getTime() + days * 24 * 60 * 60 * 1000);
            document.cookie = `${name}=${value}; expires=${date.toUTCString()}; path=/`;
        },

        getCookie(name) {
            let cookies = document.cookie.split('; ');
            for (let cookie of cookies) {
                let [key, value] = cookie.split('=');
                if (key === name) return value;
            }
            return null;
        },
    }
}