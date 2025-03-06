
import { toast } from './utils/toast.js'
//import { validation } from './utils/validation.js'
import { translations } from './utils/translations.js'
import { dome } from './utils/dome.js'
import { coverc } from './utils/coverc.js'
import { switches } from './utils/swtiches.js'
import { checkFirmwareUpdate } from './utils/checkFirmwareUpdate.js'


export default function GlobalData() {
    return {
        locale:"",
        textLoaded:false,
        exist: {dome:false,cover:false,switch:false},
        dome:{},
        coverC:{},
        board:{},
        open:  {switch:false,dome:false,cover:false,statistics:false},
        load:{switch:false,dome:false,cover:false,statistics:false},
        manual:{
            cover:0,
            brightness:0,
        },
        dataLoaded :false,
        update: false,
        version: {
            actual:0,
            available:0,
            upgrade:false
        },
        notices: [],
        visible: [],

    init(){
        const ip = import.meta.env.VITE_BOARD_IP
        // get the defined boards
        fetch(ip+'/api/cfg')
        .then(response => response.json())
        .then(data => {
            this.locale = data.locale
            this.fetchTexts(data.locale);
            this.exist = data.define;
            this.updateData()
            this.version.actual = data.version

            this.updateBoard()

            this.initFWUpdate()            
        })
        .catch(error => console.error('Error fetching board data:', error))
    },

    updateData(){
        if (this.exist.switch){
            this.getSwitchStatus()
        }

        if (this.exist.dome){
            this.getDomeStatus()
        }
        if (this.exist.coverc){
            this.getCoverCStatus()
        }
        this.dataLoaded = true;
        
      },

    updateBoard(){
        const ip = import.meta.env.VITE_BOARD_IP
        fetch(ip+'/api/board/status')
        .then(response => response.json())
        .then(data => {
            this.board = data;
            this.board.memory.freeHeapPerc =  parseFloat(((this.board.memory.freeHeap / this.board.memory.heapSize) * 100).toFixed(2));
            this.board.memory.minHeapPerc =  parseFloat(((this.board.memory.minHeap / this.board.memory.heapSize) * 100).toFixed(2));
            this.board.board.days = Math.floor(this.board.board.uptime / (24 * 60));
            this.board.board.hours = Math.floor((this.board.board.uptime % (24 * 60)) / 60);
            this.board.board.minutes = this.board.board.uptime % 60;
            this.board.wifi.days = Math.floor(this.board.wifi.uptime / (24 * 60));
            this.board.wifi.hours = Math.floor((this.board.wifi.uptime % (24 * 60)) / 60);
            this.board.wifi.minutes = this.board.wifi.uptime % 60;  

        })
        .catch(error => console.error('Error fetching board data:', error));

        setTimeout(() => {this.updateBoard()}, 10000)
    },


    ...toast(),
    ...checkFirmwareUpdate(),
    ...translations(),
//    ...validation(),
    ...dome(),
    ...coverc(),
    ...switches()

    };
}