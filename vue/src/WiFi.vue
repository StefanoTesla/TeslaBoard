
<template>

  <div v-if="txtLoaded">

    <WiFiComponent
      :txt="translations"
    /> 
  </div>
</template>


<script setup>
import WiFiComponent from './components/WiFi/WiFi.vue';

import { ref, onMounted } from 'vue'
import { useTranslations } from './composables/translation'


const { translations, loadTranslations } = useTranslations()
const txtLoaded = ref(false)

const loadInitConfig = async () => {
  try {
    await loadTranslations("en")
    txtLoaded.value = true
  } catch (error) {
    console.error('Error loading translations:', error)
  }
}


// Fetch dei dati
onMounted(async () => {
  loadInitConfig()
})
</script>
