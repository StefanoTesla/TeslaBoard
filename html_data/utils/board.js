import { Validator } from "./Validator";

export function board(){
    return {

        getBoardConfig(){
            const ip = import.meta.env.VITE_BOARD_IP
            fetch(ip+'/api/board/cfg')
            .then(response => response.json())
            .then(data => {
                this.board = data;
                this.load.board = true
            })
            .catch(error => console.error('Error fetching board data:', error));
        },

        /* board config */

        saveBoardConfig(){

            if(!this.validateBoardCfg()){

                return
            }

            const ip = import.meta.env.VITE_BOARD_IP
            fetch(ip + '/api/board/cfg', {
                method: 'POST',
                headers: {
                'Accept': 'application/json, text/plain, */*',
                'Content-Type': 'application/json'
                },
                body: JSON.stringify(this.board)
            }).then(res => res.json())
            .then(res => {
                this.reboot.board = res.reboot
                this.modal = true
                if(res.ok){
                    this.addToast({ type:"success", text: this.text.setting.board.configSaved })
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

        validateBoardCfg(){
            let err=false;

            if(this.board.address.enStaticIP){
                this.board.address.staticIp[0] = parseInt(this.board.address.staticIp[0])
                console.log(this.board.address.staticIp[0])
                err |= new Validator(this.board.address.staticIp[0],"board_wifi_staticIP_0").negativeValue().greaterThan(255).evaluate()
                err |= new Validator(this.board.address.staticIp[1],"board_wifi_staticIP_1").negativeValue().greaterThan(255).evaluate()
                err |= new Validator(this.board.address.staticIp[2],"board_wifi_staticIP_2").negativeValue().greaterThan(255).evaluate()
                err |= new Validator(this.board.address.staticIp[3],"board_wifi_staticIP_3").negativeValue().greaterThan(255).evaluate()

                err |= new Validator(this.board.address.staticSubnet[0],"board_wifi_staticSub_0").negativeValue().greaterThan(255).evaluate()
                err |= new Validator(this.board.address.staticSubnet[1],"board_wifi_staticSub_1").negativeValue().greaterThan(255).evaluate()
                err |= new Validator(this.board.address.staticSubnet[2],"board_wifi_staticSub_2").negativeValue().greaterThan(255).evaluate()
                err |= new Validator(this.board.address.staticSubnet[3],"board_wifi_staticSub_3").negativeValue().greaterThan(255).evaluate()

                err |= new Validator(this.board.address.staticGateway[0],"board_wifi_staticGat_0").negativeValue().greaterThan(255).evaluate()
                err |= new Validator(this.board.address.staticGateway[1],"board_wifi_staticGat_1").negativeValue().greaterThan(255).evaluate()
                err |= new Validator(this.board.address.staticGateway[2],"board_wifi_staticGat_2").negativeValue().greaterThan(255).evaluate()
                err |= new Validator(this.board.address.staticGateway[3],"board_wifi_staticGat_3").negativeValue().greaterThan(255).evaluate()
            }

            if(err){
                console.error("not valid")
                this.addToast({"type":"error","text":"Error in the static ip"})
                return false;
            }

            this.board.wifi.reconTime = parseInt(this.board.wifi.reconTime)

            this.board.address.staticIp[0] = parseInt(this.board.address.staticIp[0])
            this.board.address.staticIp[1] = parseInt(this.board.address.staticIp[1])
            this.board.address.staticIp[2] = parseInt(this.board.address.staticIp[2])
            this.board.address.staticIp[3] = parseInt(this.board.address.staticIp[3])
            this.board.address.staticGateway[0] = parseInt(this.board.address.staticGateway[0])
            this.board.address.staticGateway[1] = parseInt(this.board.address.staticGateway[1])
            this.board.address.staticGateway[2] = parseInt(this.board.address.staticGateway[2])
            this.board.address.staticGateway[3] = parseInt(this.board.address.staticGateway[3])
            this.board.address.staticSubnet[0] = parseInt(this.board.address.staticSubnet[0])
            this.board.address.staticSubnet[1] = parseInt(this.board.address.staticSubnet[1])
            this.board.address.staticSubnet[2] = parseInt(this.board.address.staticSubnet[2])
            this.board.address.staticSubnet[3] = parseInt(this.board.address.staticSubnet[3])

            return true;
        }
    }
}
