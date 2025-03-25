
<template>
  <Navigation 
    :home = false
  />
 

  <div v-if="txtLoaded" :class="[modal ? 'blur' : '']">

    <Switch 
      v-if="components.switch" 
      :txt="translations"
      v-model:reboot="modal"
      />

    <CoverCalibrator 
      v-if="components.coverc"
      :txt="translations"
      v-model:reboot="modal"
      />

    <Dome
      v-if="components.dome"
      :txt="translations"
      v-model:reboot="modal"
      /> 

    <BoardHome 
      :txt="translations" 

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
import { ref,watch, onMounted } from 'vue'
import { useTranslations } from './composables/translation'
import BoardHome from './components/Board/BoardSetup.vue';
import Navigation from './components/Navigation.vue';
import Switch from './components/Switch/SwitchSetup.vue'
import CoverCalibrator from './components/CoverCalibrator/CoverCalibratorSetup.vue'
import Dome from './components/Dome/DomeSetup.vue';


const components = ref([])
const { translations, loadTranslations } = useTranslations()
const txtLoaded = ref(false)
const modal = ref(false);

const loadInitConfig = async () => {
  try {
    const response = await fetch('http://localhost:3000/api/cfg')
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
  fetch('http://localhost:3000/api/board/reboot')
  setTimeout(function(){
                location.reload();
            }, 3000);
}


</script>
