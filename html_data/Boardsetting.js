import { toast } from './utils/toast.js'
import { translations } from './utils/translations.js'
import { dome } from './utils/dome.js'
import { coverc } from './utils/coverc.js'
import { switches } from './utils/swtiches.js'
import { board} from './utils/board.js'
import { validation } from './utils/validation.js'

export default function BoardSetting() {
    return {
        textLoaded:false,
        exist: {},
        dome:{},
        domeOrig:{},
        coverC:{},
        coverCOrig:{},
        swi:{},
        switchOrig:{},
        board:{},
        open:{switch:false,dome:false,cover:false,statistics:false},
        load:{switch:false,dome:false,cover:false,board:false},
        reboot:{switch:false,dome:false,cover:false,board:false},
        notices: [],
        visible: [],
        modal: false,

    init(){
        const ip = import.meta.env.VITE_BOARD_IP
        fetch(ip + '/api/cfg')
        .then(response => response.json())
        .then(data => {
            this.fetchTexts(data.locale);
            this.exist = data.define;

            this.getBoardConfig()

            if(this.exist.dome){
                this.getDomeConfig()
            }
            if(this.exist.coverc){
                this.getCoverCConfig()
            }
            if(this.exist.switch){
                console.log("eccome");
                console.log(this.exist.switch)
                this.getSwitchConfig()
            }
            
        })
        .catch(error => console.error('Error fetching board data:', error));
    },

    /* used to store loaded configuration*/
    copy(object){
        return JSON.parse(JSON.stringify(object));
    },

    closeModal(){
        this.modal = false
    },
    rebootTheBoard(){
        this.modal = false
        const ip = import.meta.env.VITE_BOARD_IP
        fetch(ip+'/api/board/reboot')
        .then(response => response.json())
        .then(response => {
            setTimeout(function(){
                location.reload();
            }, 3000);
        }
        )
        .catch(error => console.error('Error fetching board data:', error));
    },
    wifiReset(){
        this.modal = false
        const ip = import.meta.env.VITE_BOARD_IP
        fetch(ip+'/api/board/wifi-reset')
        .then(response => response.json())
        .catch(error => console.error('Error fetching board data:', error));
    },


    ...translations(),
    ...validation(),
    ...toast(),
    ...dome(),
    ...coverc(),
    ...switches(),
    ...board()

    };
}