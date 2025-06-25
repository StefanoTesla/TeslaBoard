<template>

  <Card
    v-if="props.txt.board"
    moduleName="TeslaBoard"
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
    
  >
  <div class="card">
    <div class="setting_table">
      <div class="sw_header">
        <p>{{ props.txt.board.setting.locale }}</p>
      </div>
      <div class="setting_row">
        <select id="board_locale" :class="[localeUnvalid ? 'validation_error' : '']" v-model="board.locale" @change="validate()">
          <option value="it">Italiano</option>
          <option value="en">English</option>
          <option value="fr">Français</option>
          <option value="de">Deutsch</option>
          <option value="es">Español</option>
      </select> 
      </div>
    </div>
  </div>

  <div class="card">
    <div class="sw_header">
        <p>{{ props.txt.board.setting.wifi }}</p>
      </div>
      <div class="setting_row">
        <p>{{ props.txt.board.setting.waitToReconnect }}</p>
        <div class="input_with_unit">
          <span class="unit">sec</span><input :id="`dome_timeout`" :class="['with_unit', reconTimeUnvalid ? 'validation_error' : '']" type="number" v-model="board.wifi.reconTime" @change="validate()"/>
        </div> 
      </div>
      <div class="setting_row">
        <p>{{ props.txt.board.setting.ipAddress }} {{  board.wifi.actualip }}</p>
        <p>{{ props.txt.board.home.wifi.macAddress }} {{  board.wifi.mac }}</p>
        <p>{{ props.txt.board.setting.staticIp }}</p>
          <label class="toggle" for="board_static_ip">
            <input class="toggle__input" name="" type="checkbox" id="board_static_ip" v-model="board.address.enStaticIP" @change="validate()">
            <div class="toggle__fill"></div>
          </label> 
      </div>
      <div class="setting_row" v-if="board.address.enStaticIP">
        <p>{{ props.txt.board.setting.ipAddress }}</p>
        <input type="number" :class="['input_address', ipUnvalid[0] ? 'validation_error' : '']" v-model="board.address.staticIp[0]" @change="validate()">.
        <input type="number" :class="['input_address', ipUnvalid[1] ? 'validation_error' : '']" v-model="board.address.staticIp[1]" @change="validate()">.
        <input type="number" :class="['input_address', ipUnvalid[2] ? 'validation_error' : '']" v-model="board.address.staticIp[2]" @change="validate()">.
        <input type="number" :class="['input_address', ipUnvalid[3] ? 'validation_error' : '']" v-model="board.address.staticIp[3]" @change="validate()">
      </div>
      <div class="setting_row" v-if="board.address.enStaticIP">
        <p>{{ props.txt.board.setting.gateway }}</p>
        <input type="number" :class="['input_address', gwUnvalid[0] ? 'validation_error' : '']" v-model="board.address.staticGateway[0]" @change="validate()">.
        <input type="number" :class="['input_address', gwUnvalid[1] ? 'validation_error' : '']" v-model="board.address.staticGateway[1]" @change="validate()">.
        <input type="number" :class="['input_address', gwUnvalid[2] ? 'validation_error' : '']" v-model="board.address.staticGateway[2]" @change="validate()">.
        <input type="number" :class="['input_address', gwUnvalid[3] ? 'validation_error' : '']" v-model="board.address.staticGateway[3]" @change="validate()">
      </div>
      <div class="setting_row" v-if="board.address.enStaticIP">
        <p>{{ props.txt.board.setting.subnet }}</p>
        <input type="number" :class="['input_address', subUnvalid[0] ? 'validation_error' : '']" v-model="board.address.staticSubnet[0]" @change="validate()">.
        <input type="number" :class="['input_address', subUnvalid[1] ? 'validation_error' : '']" v-model="board.address.staticSubnet[1]" @change="validate()">.
        <input type="number" :class="['input_address', subUnvalid[2] ? 'validation_error' : '']" v-model="board.address.staticSubnet[2]" @change="validate()">.
        <input type="number" :class="['input_address', subUnvalid[3] ? 'validation_error' : '']" v-model="board.address.staticSubnet[3]" @change="validate()">
      </div>
      <p v-if="board.address.enStaticIP">{{ props.txt.board.setting.warningIP }}</p>
      <div class="setting_row">
        <button class="red cursor-pointer" @click="deleteWiFiData()">{{ props.txt.board.setting.wifiReset }}</button>
      </div>
  </div>

  <div class="card">
    <div class="title">
      <p>{{ props.txt.IOBase.pin }}</p>
    </div>

    <div class="grid grid-cols-2 justify-items-center">
      <div>
        <p>Input:</p>
        <ul>
          <li v-for="el in inputGPIO">
            <p :class="[el.used===0 ? 'text-green-400!' : 'text-red-400!']">{{ el.pin }} <span v-for="module in el.module">{{ props.txt.IOBase.moduleEnum[module] }}</span></p>
            
          </li>
        </ul>
      </div>
      <div>
      <p>Output:</p>
        <ul>
          <li v-for="el in outputGPIO">
            <p :class="[el.used===0 ? 'text-green-400!' : 'text-red-400!']">{{ el.pin }} <span v-for="module in el.module">{{ props.txt.IOBase.moduleEnum[module] }}</span></p>
            
          </li>
        </ul>
      </div>
    </div>
  </div>

  <div class="card">
      <div class="sw_header">
        <p>Update</p>
      </div>
    <div class="setting_row">
      <button class="green cursor-pointer" @click="checkUpdate()">{{ props.txt.board.setting.checkUpdate }}</button>
      <p>{{ props.txt.board.setting.currentVersion }} {{ swVersion  }}</p>
      <template v-if="newVersion.check == true">
        <p v-if="newVersion.version == 0">{{ props.txt.board.setting.noUpdate }}</p>
        <p v-else>{{ props.txt.board.setting.newUpdate }} {{ newVersion.version }}</p>
      </template>

    </div>
  </div>


  <div class="card">
    <div class="setting_row">
      <button class="red cursor-pointer" @click="rebootBoard()">{{ props.txt.board.setting.reboot }}</button>
    </div>
  </div>

  <div class="config_buttons">
      <button class="green cursor-pointer" @click="getOriginal()">{{ props.txt.gen?.loadFromBoard }}</button>
      <a href="./board/boarcfg.txt" class="f_button orange" download>{{ props.txt.gen?.downloadFile }}</a>
      <button :class="[validationState ? 'red cursor-pointer' : 'black cursor-not-allowed']" @click="saveData()">{{ props.txt.gen?.save }}</button>
    </div>
  </Card>
</template>


<script setup>
import { ref,onMounted,watch } from 'vue'
import { toast } from 'vue3-toastify';
import Card from '../Card.vue';
import { useValidator } from '../../composables/validator';

const { isNegative,isGreaterThan,isInvalidPin  } = useValidator();
const props = defineProps({
  txt: Object,
  reboot: Boolean,
  gpio: Array
})

const emit = defineEmits(['update:reboot']);

const swVersion = ref()
const newVersion = ref({check:false,version:0})
const board = ref({})
const originalData = ref({})
const inputGPIO = ref([])
const outputGPIO = ref([])

let dataLoaded = ref(false)
let statusClass = ref('green')
let validationState = ref(true)

let localeUnvalid = ref(false)
let reconTimeUnvalid = ref(false)
let ipUnvalid = ref([false,false,false,false])
let gwUnvalid = ref([false,false,false,false])
let subUnvalid = ref([false,false,false,false])


const getOriginal = () => {
  board.value = JSON.parse(JSON.stringify(originalData.value));
}

const fetchData = async () => {
  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+'/api/board/cfg')
    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    const data = await response.json()
    board.value = data
    dataLoaded.value = true
    if(data.reboot){
      statusClass.value = 'orange'
    }

    originalData.value = JSON.parse(JSON.stringify(board.value));

  } catch (error) {
    console.error('Errore durante la chiamata API:', error)
  }
}

const rebootBoard = () => {
  emit('update:reboot', true);
}

const deleteWiFiData = async() => {

  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+'/api/board/wifi-reset') 
    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    const data = await response.json()

    if(data.execute){
      cmdExecutedNotify()
    }
  } catch (error) {
    errorResponseNotify(error)
  }

}

const saveData = async () => {
  validate()
  if(!validationState.value){
    errorResponseNotify(props.txt.errors.general.validationFailed)
    return
  }

  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+"/api/board/cfg", {
        method: "POST",
        headers: {
        'Accept': 'application/json, text/plain, */*',
        'Content-Type': 'application/json'
        },
        body: JSON.stringify(board.value)
      });

    if (!response.ok) { 
      if (response.status === 500) {
        throw new Error(props.txt.errors.general.configRejected)
      } else {
        throw new Error('Network response was not ok')
      }
    }

    const data = await response.json()
    cmdExecutedNotify()

    if(data.reboot){
      emit('update:reboot', true);
      statusClass.value = 'orange'
    }

  } catch (error) {
    errorResponseNotify(error)
  }
}

const validate = () => {
  validationState.value = false

  const localeAvailables = ["it","en","fr","de","es"]
  localeUnvalid.value = false
  if(!localeAvailables.includes(board.value.locale)){
    errorResponseNotify(props.txt.errors.general.wrongValue)
    localeUnvalid.value = true
    return
  }

  board.value.wifi.reconTime = parseInt(board.value.wifi.reconTime)
  reconTimeUnvalid.value = false
  if(isNegative(board.value.wifi.reconTime)){
    reconTimeUnvalid.value = true;
    errorResponseNotify(props.txt.errors.general.negativeValue)
    return
  }

  // check static ip 
  for (let index = 0; index < 4; index++) {
    ipUnvalid.value[index] = false
    gwUnvalid.value[index] = false
    subUnvalid.value[index] = false

    board.value.address.staticIp[index] = parseInt(board.value.address.staticIp[index])
    board.value.address.staticGateway[index] = parseInt(board.value.address.staticGateway[index])
    board.value.address.staticSubnet[index] = parseInt(board.value.address.staticSubnet[index])
    if(isNegative(board.value.address.staticIp[index])){
      ipUnvalid.value[index] = true
      errorResponseNotify(props.txt.errors.general.negativeValue)
      return
    }
    if(isGreaterThan(board.value.address.staticIp[index],255)){
      ipUnvalid.value[index] = true
      const errorMessage = props.txt.errors.general.greaterThan + " " + 255
      errorResponseNotify(errorMessage)
      return
    }
    if(isNegative(board.value.address.staticGateway[index])){
      gwUnvalid.value[index] = true
      errorResponseNotify(props.txt.errors.general.negativeValue)
      return
    }
    if(isGreaterThan(board.value.address.staticGateway[index],255)){
      gwUnvalid.value[index] = true
      const errorMessage = props.txt.errors.general.greaterThan + " " + 255
      errorResponseNotify(errorMessage)
      return
    }
    if(isNegative(board.value.address.staticSubnet[index])){
      subUnvalid.value[index] = true
      errorResponseNotify(props.txt.errors.general.negativeValue)
      return
    }
    if(isGreaterThan(board.value.address.staticSubnet[index],255)){
      subUnvalid.value[index] = true
      const errorMessage = props.txt.errors.general.greaterThan + " " + 255
      errorResponseNotify(errorMessage)
      return
    }
  }

  validationState.value = true

}

onMounted(() => {
  fetchData()
  let str = String(import.meta.env.VITE_SW_VERSION)
  console.log(str)
  str = str.slice(0, str.length - 2) + '.' + str.slice(str.length - 2);
  str = str.slice(0, str.length - 5) + '.' + str.slice(str.length - 5);
  swVersion.value = str
})

const cmdExecutedNotify = () => {
  toast.success(props.txt.gen.configSaved, {
  autoClose: 500,
});
}

const errorResponseNotify = (errorMessage) => {
 toast.error(errorMessage, {
  autoClose: 3000,
  });
}

const checkUpdate = () => {

fetch('https://api.stefanotesla.it/api/teslaboard')
  .then(response => {
    if (!response.ok){
      newVersion.value.check = true
      return null;
    }
    newVersion.value.check = true
    return response.json(); 
  })
  .then(data => {
    if (data.version > import.meta.env.VITE_SW_VERSION) {
      let str = String(data.version)
      str = str.slice(0, str.length - 2) + '.' + str.slice(str.length - 2);
      str = str.slice(0, str.length - 5) + '.' + str.slice(str.length - 5);
      newVersion.value.version = str
      console.log(newVersion.value)
    }
  })
  .catch(() => {
    newVersion.value.check = true;
    return null
  }

);

}

const checkWherIsUsed = (pin,list) => {
  const ret = [];

  list.forEach(el => {
    if(el.pin === pin){
      ret.push(el.module)
    }
  });

  return ret

}

watch(() => props.gpio, (newValue) => {
  
  const gpios = Array.from({ length: 39 }, (_, i) => ({
    pin: i + 1,
    module: 0
  }));

  const usedPins = newValue
    .filter(obj => obj.pin)
    .map(obj => obj.pin);

  inputGPIO.value = []
  outputGPIO.value = []

  gpios.forEach(element => {
    let res = checkWherIsUsed(element.pin,newValue);
    if(!isInvalidPin(element.pin,'input')){
      if (res.length == 0){
        inputGPIO.value.push({pin:element.pin,used:0,module:[]})
      } else {
        inputGPIO.value.push({pin:element.pin,used:1,module:res})
      }
    }
    if(!isInvalidPin(element.pin,'output')){
      if (res.length == 0){
        outputGPIO.value.push({pin:element.pin,used:0,module:[]})
      } else {
        outputGPIO.value.push({pin:element.pin,used:1,module:res})
      }
    }
  });


}, { deep: true });

</script>