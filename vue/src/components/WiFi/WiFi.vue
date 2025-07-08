<template>
  <Card 
  v-if="props.txt.dome"
      :moduleName="props.txt.dome.title"
      :dataLoaded="dataLoaded"
      >

    <div class="card">
      <div>
        <p>Seleziona un WiFi dall'elenco:</p>
        <ul>
          <li v-for="wifi in wifiList" :key="wifi.ssid" class="flex">
              <span class="flex cursor-pointer items-center space-x-2 h-8" @click="copySSID(wifi)"> 
              
              <p>{{ wifi.ssid }} </p>
              
              <svg  width="24" height="24" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg" class="inline align-bottom">
                      <rect v-if="wifi.rssi >= -90" x="1" y="18" width="2" height="5" rx="0.5" fill="red" />
                      <rect v-if="wifi.rssi >= -80" x="6" y="14" width="2" height="9" rx="0.5" fill="orange" />
                      <rect v-if="wifi.rssi >= -70" x="11" y="10" width="2" height="13" rx="0.5" fill="orange" />
                      <rect v-if="wifi.rssi >= -65" x="16" y="6" width="2" height="17" rx="0.5" fill="green" />
                      <rect v-if="wifi.rssi >= -60" x="21" y="2" width="2" height="21" rx="0.5" fill="green" />
              </svg>
              <span v-if="wifi.enc=='psw'"></span><svg xmlns="http://www.w3.org/2000/svg" class="inline align-bottom" viewBox="0 0 24 24" width="24px" height="24px"><path d="M 12 1 C 8.6761905 1 6 3.6761905 6 7 L 6 8 C 4.9 8 4 8.9 4 10 L 4 20 C 4 21.1 4.9 22 6 22 L 18 22 C 19.1 22 20 21.1 20 20 L 20 10 C 20 8.9 19.1 8 18 8 L 18 7 C 18 3.6761905 15.32381 1 12 1 z M 12 3 C 14.27619 3 16 4.7238095 16 7 L 16 8 L 8 8 L 8 7 C 8 4.7238095 9.7238095 3 12 3 z M 12 13 C 13.1 13 14 13.9 14 15 C 14 16.1 13.1 17 12 17 C 10.9 17 10 16.1 10 15 C 10 13.9 10.9 13 12 13 z"/></svg>
            </span>
          </li>
        </ul>
        
      </div>

    </div>
    <div class="card">
        Connettiti alla rete Wifi:

          <div class="sw_header">
            <p>SSID:</p>
            <input type="text" class="w-full" v-model="wifiToConnect.ssid"/>
          </div>
          <div class="sw_header">
            <p>Password:</p>
            <input type="password" class="w-full" v-model="wifiToConnect.psw"/>
          </div>
          <div class="sw_header">
            <p>Imposta come predefinita:</p>
              <label class="toggle" for="wifiDefault">
                <input class="toggle__input" name="wifiDefault" id="wifiDefault" type="checkbox" v-model="wifiToConnect.default">
                <div class="toggle__fill cursor-pointer"></div>
              </label>

          </div>
          <button class="green cursor-pointer" @click="connectToWifi()">Connetti</button>
    </div>
  </Card>
</template>


<script setup>
import { ref,onMounted,onUnmounted,computed } from 'vue'
import Card from '../Card.vue';

const props = defineProps({
  txt: Object
})


const wifiList = ref({})
const wifiToConnect = ref({"ssid":"","psw":"", "default":false})
let dataLoaded = ref(false)


const fetchData = async () => {
  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+'/wifi-api/wifi-list')
    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    const data = await response.json()

    const ssidMap = new Map();


    for (const item of data.wifi) {
      const existing = ssidMap.get(item.ssid);
      if (!existing || item.rssi > existing.rssi) {
        ssidMap.set(item.ssid, item);
      }
    }
  
    wifiList.value = [...ssidMap.values()];
    dataLoaded.value = true

    const classes = ['green', 'green', 'orange', 'orange', 'red']
    //statusClass.value = classes[dome.value.status.roofState] 

    
  } catch (error) {
    console.error('Errore durante la chiamata API:', error)
  }
}


const copySSID = async (wifi) => {
  wifiToConnect.value.ssid = wifi.ssid
  wifiToConnect.value.enc = wifi.enc

}

const connectToWifi = async () => {

  if(wifiToConnect.value.ssid == ""){
    console.error("SSID can't be empty")
    return
  }

  if(wifiToConnect.psw == "psw" && wifiToConnect.value.psw == ""){
    console.error("for this wifi password is required")
    return
  }

  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+"/wifi-api/new-wifi", {
        method: "POST",
        headers: {
        'Accept': 'application/json, text/plain, */*',
        'Content-Type': 'application/json'
        },
        body: JSON.stringify(wifiToConnect.value)
      });

    const data = await response.json()
    if (!response.ok) throw { status: response.status, data }
    
  } catch (err) {
    console.error(err)
  }
}

onMounted(() => {
  fetchData()
})

</script>