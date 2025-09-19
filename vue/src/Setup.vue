
<template>
  <Navigation 
    :home = false
  />
 
  <div v-if="txtLoaded" :class="[modal ? 'blur' : '']">

    <Advise
    :notifies = "permNotify"
    />

    <Switch 
      :txt="translations"
      v-model:reboot="modal"
      @update:pinUsed="handleSwitchPinUpdate"
      />

    <CoverCalibrator 
      :txt="translations"
      v-model:reboot="modal" 
      @update:pinUsed="handleCoverCPinUpdate"
      />

    <Dome
      :txt="translations"
      v-model:reboot="modal"
      @update:pinUsed="handleDomePinUpdate"
      /> 

    <Board 
      :txt="translations" 
      :gpio="gpioObserver"
      v-model:reboot="modal"
      />

  </div>

  <div class="modal" v-if="modal">
      <div class="modal_box">
        <p class="error">{{ translations.board.reboot.warning  }}</p>
        <p x-text="text.board.reboot.text1">{{ translations.board.reboot.text1 }}</p>
        <p x-text="text.board.reboot.text2">{{ translations.board.reboot.text2  }}</p>
        <div class="modal_selection">
          <button class="green" @click="closeModal()">{{ translations.board.reboot.later }}</button>
          <button class="red"  @click="rebootNow()">{{ translations.board.reboot.now }}</button>
        </div>
      </div>
    </div>
</template>


<script setup>
import { ref, onMounted } from 'vue'
import { useTranslations } from './composables/translation'
import Board from './components/Board/BoardSetup.vue';
import Navigation from './components/Navigation.vue';
import Switch from './components/Switch/SwitchSetup.vue'
import CoverCalibrator from './components/CoverCalibrator/CoverCalibratorSetup.vue'
import Dome from './components/Dome/DomeSetup.vue';
import Advise from './components/Advise/Advise.vue';
  import { useValidator } from './composables/Validator';

  const { isInvalidPin } = useValidator();

const gpioObserver = ref(Array.from({ length: 40 }, () => ({ type: -1, module: -1 })));
const domeGPIO = ref([])
const coverCGPIO = ref([])
const switchGPIO = ref([])
const permNotify = ref([])

const components = ref([])
const { translations, loadTranslations } = useTranslations()
const txtLoaded = ref(false)
const modal = ref(false);

const loadInitConfig = async () => {
  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+'/api/cfg')
    const data = await response.json()

    components.value = data.define
    await loadTranslations(data.locale)
    txtLoaded.value = true
  } catch (error) {
    console.error('Error loading config:', error)
  }
}

onMounted(async () => {
  loadInitConfig()
})

const closeModal = () => {
  modal.value = false
}


const rebootNow = () => {
  modal.value = false
  const ip = import.meta.env.VITE_API_IP;
  fetch(ip+'/api/board/reboot')
  setTimeout(function(){
                location.reload();
            }, 3000);
}


const handleDomePinUpdate = (data) => {
  domeGPIO.value = []
  data.forEach(element => {
    domeGPIO.value.push({pin:element.pin, type:element.type , module:1})
  });

  rebuildGPIOPinList()
}

const handleCoverCPinUpdate = (data) => {
  coverCGPIO.value = []
  data.forEach(element => {
    coverCGPIO.value.push({pin:element.pin, type:element.type , module:2})
  });
  rebuildGPIOPinList()
}

const handleSwitchPinUpdate = (data) => {
  switchGPIO.value = []
  data.forEach(element => {
    switchGPIO.value.push({pin:element.pin, type:element.type , module:3})
  });
  rebuildGPIOPinList()
}

const rebuildGPIOPinList = () => {
  gpioObserver.value = [...domeGPIO.value,...coverCGPIO.value,...switchGPIO.value]


  //performances: STONK!
  //clean oldest message
  gpioObserver.value.forEach(element =>{
    removePermanentNotifiy(
      "warning",
      translations.value.errors.general.error + " " + translations.value.IOBase.pin + element.pin + " " + translations.value.errors.gpio.doubleUsage
      )
  });

  const doubledGipo = gpioObserver.value
    .map(item => item.pin)
    .filter((pin, index, self) => self.indexOf(pin) !== index && self.lastIndexOf(pin) === index);

  doubledGipo.forEach(gpio =>{
    console.log(gpio)
    if(!isInvalidPin(gpio,"")){
    addPermanentNotifiy(
      "warning",
      translations.value.errors.general.error + " " + translations.value.IOBase.pin + gpio + " " + translations.value.errors.gpio.doubleUsage
      )
    }

  });
 
}



const addPermanentNotifiy = (type,message) => {
  if (!permNotify.value.some(n => n.type === type && n.message === message)) {
    permNotify.value.push({type:type,message:message})
  }

}
const removePermanentNotifiy = (type,message) => {
  permNotify.value = permNotify.value.filter(n => !(n.type === type && n.message === message))
}
</script>
