
<template>
  <Navigation 
    :home = false
  />
 

  <div v-if="txtLoaded" :class="[modal ? 'blur' : '']">

    <Switch 
      v-if="components.switch" 
      :txt="translations"
      v-model:reboot="modal"
      @update:pinUsed="handleSwitchPinUpdate"
      />

    <CoverCalibrator 
      v-if="components.coverc"
      :txt="translations"
      v-model:reboot="modal" 
      @update:pinUsed="handleCoverCPinUpdate"
      />

    <Dome
      v-if="components.dome"
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

const gpioObserver = ref(Array.from({ length: 40 }, () => ({ type: -1, module: -1 })));
const domeGPIO = ref([])
const coverCGPIO = ref([])
const switchGPIO = ref([])

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


const initGPIOList = () => {
  gpioObserver.value = Array.from({ length: 40 }, () => ({ type: 0, module: 0 }));
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
    let i = element.pin
    coverCGPIO.value[i] = {}
    coverCGPIO.value[i].type = element.type
    coverCGPIO.value[i].module = 2
  });
  rebuildGPIOPinList()
}

const handleSwitchPinUpdate = (data) => {
  switchGPIO.value = []
  data.forEach(element => {
    let i = element.pin
    switchGPIO.value[i] = { type: data.type, module: 0 };
  });
  rebuildGPIOPinList()
}

const rebuildGPIOPinList = () => {
  initGPIOList()
  if(domeGPIO.value.length > 0){
    domeGPIO.value.forEach((data,pin) =>{
      Object.assign(gpioObserver.value[pin], { type: data.type, module: data.module });
    });
  }
  if(coverCGPIO.value.length > 0){
    coverCGPIO.value.forEach((data,pin) =>{
      Object.assign(gpioObserver.value[pin], { type: data.type, module: data.module });
    });
  }
  if(switchGPIO.value.length > 0){
    switchGPIO.value.forEach((data,pin) =>{
      Object.assign(gpioObserver.value[pin], { type: data.type, module: data.module });
    });
  }
}

</script>
