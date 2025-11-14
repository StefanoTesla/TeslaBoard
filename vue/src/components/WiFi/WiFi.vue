<template>
  <Card 
      :moduleName="title"
      :dataLoaded="dataLoaded"
      >

    <div class="card">
      <div>
        <div class="title">
          <p>Choose the WiFi network you want to connect</p>
        </div>
          <div v-for="wifi in wifiList" :key="wifi.ssid" class="wifi_networks cursor-default" @click="copySSID(wifi)">
            <div class="ssid">
              <p>{{ wifi.ssid }} </p>
            </div> 
            <div>
              <svg  width="24" height="24" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg" class="inline align-bottom">
                      <rect v-if="wifi.rssi >= -90" x="1" y="18" width="2" height="5" rx="0.5" fill="red" />
                      <rect v-if="wifi.rssi >= -80" x="6" y="14" width="2" height="9" rx="0.5" fill="orange" />
                      <rect v-if="wifi.rssi >= -70" x="11" y="10" width="2" height="13" rx="0.5" fill="orange" />
                      <rect v-if="wifi.rssi >= -65" x="16" y="6" width="2" height="17" rx="0.5" fill="green" />
                      <rect v-if="wifi.rssi >= -60" x="21" y="2" width="2" height="21" rx="0.5" fill="green" />
              </svg>
            </div>
            <div>
              <span v-if="wifi.enc=='psw'">
                <svg xmlns="http://www.w3.org/2000/svg" class="inline align-bottom" viewBox="0 0 24 24" width="24px" height="24px" fill="#fff">
                  <path d="M 12 1 C 8.6761905 1 6 3.6761905 6 7 L 6 8 C 4.9 8 4 8.9 4 10 L 4 20 C 4 21.1 4.9 22 6 22 L 18 22 C 19.1 22 20 21.1 20 20 L 20 10 C 20 8.9 19.1 8 18 8 L 18 7 C 18 3.6761905 15.32381 1 12 1 z M 12 3 C 14.27619 3 16 4.7238095 16 7 L 16 8 L 8 8 L 8 7 C 8 4.7238095 9.7238095 3 12 3 z M 12 13 C 13.1 13 14 13.9 14 15 C 14 16.1 13.1 17 12 17 C 10.9 17 10 16.1 10 15 C 10 13.9 10.9 13 12 13 z"/>
                </svg>
              </span>
            </div>
          </div>        
      </div>

    </div>
    <div class="card my-4">
        Connect or Add a WiFi:
        
          <div class="sw_header">
            <p>SSID:</p>
            <input type="text" class="w-full identifier" v-model="wifiToConnect.ssid"/>
          </div>
          <div class="sw_header">
            <p>Password:</p>
            <input type="password" class="w-full identifier" v-model="wifiToConnect.psw"/>
          </div>
          <div class="flex justify-around">
            <button class="green cursor-pointer" @click="connectToWifi()">Connect</button>
          </div>
          
    </div>
  </Card>


  <Card
      :moduleName="setting"
      :dataLoaded="dataLoaded"
      >

      <div class="card">
        <div>
          <div class="title">
            <p>Stored WiFi:</p>
          </div>
          <div v-for="wifi in configuredWiFi" :key="wifi.ssid" class="wifi_networks">
              <div class="ssid">
                <p>{{ wifi.ssid }} </p>
              </div> 
              <div>
                <span class="inline-block" v-if="wifi.default">
                  <svg xmlns="http://www.w3.org/2000/svg" width="21.87" height="20.801" fill="#fff"><path d="m4.178 20.801 6.758-4.91 6.756 4.91-2.58-7.946 6.758-4.91h-8.352L10.936 0 8.354 7.945H0l6.758 4.91-2.58 7.946z"/></svg>
                </span>
              </div>
              <div @click="deleteWifi(wifi)">
                <svg xmlns="http://www.w3.org/2000/svg" fill="#fff" viewBox="0 0 128 128" width="24px" height="24px"><path d="M 49 1 C 47.34 1 46 2.34 46 4 C 46 5.66 47.34 7 49 7 L 79 7 C 80.66 7 82 5.66 82 4 C 82 2.34 80.66 1 79 1 L 49 1 z M 24 15 C 16.83 15 11 20.83 11 28 C 11 35.17 16.83 41 24 41 L 101 41 L 101 104 C 101 113.37 93.37 121 84 121 L 44 121 C 34.63 121 27 113.37 27 104 L 27 52 C 27 50.34 25.66 49 24 49 C 22.34 49 21 50.34 21 52 L 21 104 C 21 116.68 31.32 127 44 127 L 84 127 C 96.68 127 107 116.68 107 104 L 107 40.640625 C 112.72 39.280625 117 34.14 117 28 C 117 20.83 111.17 15 104 15 L 24 15 z M 24 21 L 104 21 C 107.86 21 111 24.14 111 28 C 111 31.86 107.86 35 104 35 L 24 35 C 20.14 35 17 31.86 17 28 C 17 24.14 20.14 21 24 21 z M 50 55 C 48.34 55 47 56.34 47 58 L 47 104 C 47 105.66 48.34 107 50 107 C 51.66 107 53 105.66 53 104 L 53 58 C 53 56.34 51.66 55 50 55 z M 78 55 C 76.34 55 75 56.34 75 58 L 75 104 C 75 105.66 76.34 107 78 107 C 79.66 107 81 105.66 81 104 L 81 58 C 81 56.34 79.66 55 78 55 z"/></svg>
              </div>
            </div>  
          </div>
        </div>

  </Card>
</template>


<script setup>
import { ref,onMounted, onUnmounted } from 'vue'
import Card from '../Card.vue';
import { toast } from 'vue3-toastify';

const title = "WiFi"
const setting = "Setup"

const props = defineProps({
  txt: Object
})

const configuredWiFi = ref({})
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

    //delete double ssid
    for (const item of data.wifi) {
      const existing = ssidMap.get(item.ssid);
      if (!existing || item.rssi > existing.rssi) {
        ssidMap.set(item.ssid, item);
      }
    }
    //sort by signal
    wifiList.value = [...ssidMap.values()].sort((a, b) => b.rssi - a.rssi);
    configuredWiFi.value = data.stored;

    dataLoaded.value = true

    
  } catch (error) {
    console.error('Errore durante la chiamata API:', error)
  }
}

const copySSID = async (wifi) => {
  wifiToConnect.value.ssid = wifi.ssid
  wifiToConnect.value.enc = wifi.enc
  wifiToConnect.value.psw = ""
}

const connectToWifi = async () => {

  if(wifiToConnect.value.ssid == ""){
      toast.error("SSID can't be empty",{
        autoClose: 3000,
      })
      return
  }

  if(wifiToConnect.psw == "psw" && wifiToConnect.value.psw == ""){
    console.error("for this wifi, password is required")
    return
  }

  try {
    const ip = import.meta.env.VITE_API_IP;
    const url = `${ip}/wifi-api/new-wifi`;
    const response = await fetch(url, {
        method: "POST",
        headers: {
        'Accept': 'application/json, text/plain, */*',
        'Content-Type': 'application/json'
        },
        body: JSON.stringify(wifiToConnect.value)
      });

    const data = await response.json()
    if (response.ok){
      if(mode === "connect"){
        const Msg = "New Wifi recived, board will try to connect and then it will be store"
      } else {
        const Msg = "The WiFi was stored"
      }
      
      toast.success("The WiFi was added to the list, performing a connection!", {
        autoClose: 5000,
      });
      fetchData()
    } else {
      const errorMsg = data?.error || "Unable to add a new WiFi";
      toast.error(errorMsg, {
        autoClose: 500,
      });
    }

  } catch (err) {
    console.error(err)
  } finally {
    wifiToConnect.value = {"ssid":"","psw":"", "default":false}
  }
}
const addNewWifi = async () => {

  if(wifiToConnect.value.ssid == ""){
      toast.error("SSID can't be empty",{
        autoClose: 3000,
      })
    return
  }

  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+"/wifi-api/add-wifi", {
        method: "POST",
        headers: {
        'Accept': 'application/json, text/plain, */*',
        'Content-Type': 'application/json'
        },
        body: JSON.stringify(wifiToConnect.value)
      });

    const data = await response.json()
    if (response.ok){
      toast.success("The WiFi was added to the list", {
        autoClose: 500,
      });
      fetchData()
    } else {
      toast.error("Unable to add a new WiFi",{
        autoClose: 3000,
      })
    }
    
  } catch (err) {
    console.error(err)
  } finally {
    wifiToConnect.value = {"ssid":"","psw":"", "default":false}
  }
}

const deleteWifi = async (wifi) => {

  console.log("deleting wifi: ",wifi.ssid)
  if(wifi.ssid == ""){
    console.error("SSID can't be empty")
    return
  }

  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+"/wifi-api/delete-wifi", {
        method: "POST",
        headers: {
        'Accept': 'application/json, text/plain, */*',
        'Content-Type': 'application/json'
        },
        body: JSON.stringify(wifi)
      });

    const data = await response.json()
    if (response.ok){
      fetchData()
    }

  } catch (err) {
    console.error(err)
  }
}

let intervalId = null
onMounted(() => {
  fetchData()
  intervalId = setInterval(fetchData, 20000)
})

onUnmounted(() => {
  clearInterval(intervalId)
})
</script>