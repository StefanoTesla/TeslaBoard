<template>

  <Card 
    :moduleName="title" 
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
    >
      <div class="card">
        <div class="title">
          <p>WiFi</p>
        </div>
        <div class="row">
          <p>{{ props.txt.board.home.wifi.ssid }}</p>
          <p>{{ board.wifi.ssid }}</p>
        </div>
        <div class="row">
          <p>IP:</p>
          <p>{{ board.wifi.ip }}</p>
        </div>
        <div class="row">
          <p>{{ props.txt.board.home.wifi.macAddress }}</p>
          <p>{{ board.wifi.mac }}</p>
        </div>
        <div class="row">
          <p>{{ props.txt.board.home.wifi.db }}</p>
          <p>{{ board.wifi.db }} dB</p>
        </div>
        <div class="row">
          <p>{{ props.txt.board.home.uptime.from }}</p>
          <p>{{ upTime(board.wifi.uptime) }}</p>
        </div>
      </div>
      <div class="card">
        <div class="title">
          <p>CPU</p>
        </div>
        <div class="row">
          <p>CPU:</p>
          <p>{{ board.board.cpu }}</p>
        </div>
        <div class="row">
          <p>REV:</p>
          <p>{{ board.board.rev }}</p>
        </div>
        <div class="row">
          <p>{{ props.txt.board.home.cpu.speed }}</p>
          <p>{{ board.board.freq }} MHz</p>
        </div>
        <div class="row">
          <p>{{ props.txt.board.home.uptime.from }}</p>
          <p>{{ upTime(board.board.uptime) }}</p>
        </div>
        <div class="row">
          <p>{{ props.txt.board.home.memory.total }}</p>
          <p>{{ kBconv(board.memory.heapSize) }} kB</p>
        </div>
        <div class="row">
          <p>{{ props.txt.board.home.memory.free }}</p>
          <p>{{ kBconv(board.memory.freeHeap) }} kB ({{ percentage(board.memory.heapSize,board.memory.freeHeap) }}%)</p>
        </div>
        <div class="row">
          <p>{{ props.txt.board.home.memory.minFree }}</p>
          <p>{{ kBconv(board.memory.minHeap) }} kB ({{ percentage(board.memory.heapSize,board.memory.minHeap) }}%)</p>
        </div>
      </div>
    </Card>
    <footer class="mt-4"></footer>
  </template>
  
  
  <script setup>
  import Card from '../Card.vue';
  import { ref,onMounted,onUnmounted,computed } from 'vue'
  
  const title = "TeslaBoard"

  const props = defineProps({
    txt: Object
  })
  

  const board = ref({})
  let dataLoaded = ref(false)
  
  const fetchData = async () => {
    try {
      const ip = import.meta.env.VITE_API_IP;
      const response = await fetch(ip+'/api/board/status') 
      if (!response.ok) {
        throw new Error('Network response was not ok')
      }
      const data = await response.json()
      board.value = data
      dataLoaded.value = true
  
      
    } catch (error) {
      console.error('Errore durante la chiamata API:', error)
    }
  }
  
  const statusClass = computed(() => {
        return 'green'
  })


  const upTime = (time)=> {
    let days = 0;
    let hours = 0;
    let minutes = 0;

    days = Math.floor(time / (24 * 60));
    hours = Math.floor((time % (24 * 60)) / 60);
    minutes = time % 60;
    let string = ""
    if(days > 0){
      string += days + " " +  props.txt.board.home.uptime.days + " "
    }
    if(hours > 0){
      string += hours + " " +  props.txt.board.home.uptime.hours + " "
    }
    if(minutes > 0){
      string += minutes + " " + props.txt.board.home.uptime.minutes
    }

    return string
  }

  const kBconv = (bytes) => {
    return Math.round(bytes / 1000).toFixed(1)
  }

  const percentage = (max,value) => {
    return Math.floor((value*100)/max)
  }

  let intervalId = null
  
  onMounted(() => {
    fetchData()
    intervalId = setInterval(fetchData, 30000)
  })
  
  onUnmounted(() => {
    clearInterval(intervalId)
  })
  
  </script>