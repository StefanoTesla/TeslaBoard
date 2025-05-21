
<template>
  <Navigation 
    :home = true  
  />
  <div v-if="txtLoaded">
    <Switch 
      v-if="components.switch" 
      :txt="translations"/>

    <CoverCalibrator 
      v-if="components.coverc"
      :txt="translations"/>

    <Dome
      v-if="components.dome"
      :txt="translations"/> 

    <BoardHome 
      :txt="translations" 
    /> 
  </div>
</template>


<script setup>
import Navigation from './components/Navigation.vue';

import Switch from './components/Switch/SwitchHome.vue'
import CoverCalibrator from './components/CoverCalibrator/CoverCalibratorHome.vue'
import Dome from './components/Dome/DomeHome.vue';
import { ref, onMounted } from 'vue'
import { useTranslations } from './composables/translation'
import BoardHome from './components/Board/BoardHome.vue';

const components = ref([]) // Variabile reattiva
const { translations, loadTranslations } = useTranslations()
const txtLoaded = ref(false)

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


// Fetch dei dati
onMounted(async () => {
  loadInitConfig()
})
</script>
