
<template>

  <div>
    <WiFiComponent/> 
  </div>
</template>


<script setup>
import WiFiComponent from './components/WiFi/WiFi.vue';

import { ref, onMounted } from 'vue'

const txtLoaded = ref({})
const loadInitConfig = async () => {
    txtLoaded.value = true
}


// Fetch dei dati
onMounted(async () => {
  loadInitConfig()
})
</script>
