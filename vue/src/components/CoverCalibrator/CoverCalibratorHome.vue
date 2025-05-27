<template>

  <Card 
  v-if="props.txt.dome"
      :moduleName="props.txt.coverC.title"
      :dataLoaded="dataLoaded"
      :statusClass=statusClass
  >
      <div class="card" v-if="coverC.calibrator.status > 0">
        <div class="title">
          <p>{{ txt.coverC.calibrator}}</p>
        </div>
        <div class="range">
          <input type="range" v-model="coverC.calibrator.brightness" min='0' max='4095' step='1' @change="calibratorBrightnessChange" />
        </div>
        <div class="flex justify-center">
          <p>{{ txt.gen.status.actualValue }}</p>
          <p class="pl-2">{{ coverC.calibrator.brightness }}/4095</p>
        </div>
        <div class="flex justify-around">
          <button :class="calibratorPowerOnCmdClass" @click="calibratorPowerOnCmd">{{ txt.gen.action.powerOn }}</button>
          <button :class="calibratorPowerOffCmdClass" @click="calibratorPowerOffCmd">{{ txt.gen.action.powerOff }}</button>
        </div>
      </div>
      <div class="card" v-if="coverC.cover.status > 0">
        <div class="title">
          <p>{{ txt.coverC.cover}}</p>
        </div>
        <div class="flex justify-center">
          <p>{{ txt.coverC.home.coverState }}</p>
          <p class="pl-2">{{ coverStatus }}</p>
        </div>
        <div class="flex justify-around">
          <button :class="coverOpenCmdClass" @click="coverOpenCmd">{{ txt.gen.action.open }}</button>
          <button :class="coverCloseCmdClass" @click="coverCloseCmd">{{ txt.gen.action.close }}</button>
        </div>
        <div class="flex justify-center">
          <p>{{ txt.gen.status.actualPos }}</p>
          <p class="pl-2">{{ coverC.cover.angle }}°</p>
        </div>
      </div>

  </Card>
  </template>
  
  
  <script setup>
  import { ref,onMounted,onUnmounted,computed } from 'vue'
  import { toast } from 'vue3-toastify';
  import Card from '../Card.vue';
  
  const props = defineProps({
    txt: Object
  })
  
  const open = ref(false);
  
  const toggleOpen = () => {
    open.value = !open.value
  }
  
  const coverC = ref({})
  let dataLoaded = ref(false)
  let statusClass = ref('red')
  let canOpenCover =  ref(false)
  let canCloseCover =  ref(false)
  let coverStatus = ref('')
  const fetchData = async () => {
    try {
      const ip = import.meta.env.VITE_API_IP;
      const response = await fetch(ip+'/api/coverc/status')
      if (!response.ok) {
        throw new Error('Network response was not ok')
      }
      const data = await response.json()
      coverC.value = data
      dataLoaded.value = true
      updateStatusData()
      
    } catch (error) {
      console.error('Errore durante la chiamata API:', error)
    }
  }
  
  const updateStatusData = () => {
    if(coverC.value.cover.status >= 4) {
      statusClass.value = 'red'
    } else if (coverC.value.cover.status == 2) {
      statusClass.value = 'orange'
    } else {
      statusClass.value = 'green'
    }

    canOpenCover.value = (coverC.value.cover.status == 2 || coverC.value.cover.status == 3) ? false : true
  
    canCloseCover.value = coverC.value.cover.status <= 2 ? false : true

    const enumCommand = [props.txt.coverC.home.coverEnum.notPresent , props.txt.gen.status.close, props.txt.coverC.home.coverEnum.moving, props.txt.gen.status.open,props.txt.coverC.home.coverEnum.unknow , props.txt.coverC.home.coverEnum.error]
    coverStatus.value = enumCommand[coverC.value.cover.status]
  }
  


  const coverOpenCmdClass = computed(() => {
  return canOpenCover.value ?['green', 'cursor-pointer'] : ['disactivated', 'cursor-not-allowed'] 
  })

  const coverCloseCmdClass = computed(() => {
  return canCloseCover.value ?['green', 'cursor-pointer'] : ['disactivated', 'cursor-not-allowed'] 
  })

  const calibratorPowerOffCmdClass = computed(() => {
    return coverC.value.calibrator.brightness != 0 ?['green', 'cursor-pointer'] : ['disactivated', 'cursor-not-allowed'] 
  })

  const calibratorPowerOnCmdClass = computed(() => {
    return coverC.value.calibrator.brightness == 0 ?['green', 'cursor-pointer'] : ['disactivated', 'cursor-not-allowed'] 
  })

  const calibratorPowerOnCmd = async () => {
    if( coverC.value.calibrator.brightness == 4095){
      cmdRefusedNotify()
      return
    }
    try {
      const ip = import.meta.env.VITE_API_IP;
      const response = await fetch(ip+"/api/coverc/on", {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
            "Accept": "application/json, text/plain, */*"
          }
        });
      if (!response.ok) {
        throw new Error('Network response was not ok')
      }
      const res = await response.json()

      if(res.error){
        errorResponseNotify(error)
        return
      }

      if(res.execute){
        coverC.value.calibrator.brightness = 4095;
        cmdExecutedNotify()
        return
      }
  
      
    } catch (error) {
      noResponseNotify(error)
    }
  }

  const calibratorPowerOffCmd = async () => {
    if( coverC.value.calibrator.brightness == 0){
      cmdRefusedNotify()
      return
    }
    try {
      const ip = import.meta.env.VITE_API_IP;
      const response = await fetch(ip+"/api/coverc/off", {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
            "Accept": "application/json, text/plain, */*"
          }
        });
      if (!response.ok) {
        throw new Error('Network response was not ok')
      }
      const res = await response.json()

      if(res.error){
        errorResponseNotify(error)
        return
      }

      if(res.execute){
        coverC.value.calibrator.brightness = 0;
        cmdExecutedNotify()
        return
      }
  
      
    } catch (error) {
      noResponseNotify(error)
    }
  }

  const calibratorBrightnessChange = async() => {
    if( coverC.value.calibrator.brightness < 0 || coverC.value.calibrator.brightness > 4095){
      cmdRefusedNotify()
      return
    }
    try {
      const ip = import.meta.env.VITE_API_IP;
      const response = await fetch(ip+"/api/coverc/brightness", {
          method: "POST",
          headers: {
            "Content-Type": "application/x-www-form-urlencoded",
            "Accept": "application/json, text/plain, */*"
          },
          body:new URLSearchParams({
            "brightness" : coverC.value.calibrator.brightness
          })
        });
      if (!response.ok) {
        throw new Error('Network response was not ok')
      }
      const res = await response.json()

      if(res.error){
        errorResponseNotify(error)
        return
      }
      if(res.execute){
        cmdExecutedNotify()
        return
      }
  
      
    } catch (error) {
      noResponseNotify(error)
    }
  }

  const coverOpenCmd = async () => {
    if(!canOpenCover.value){
      cmdRefusedNotify()
      return
    }
    try {
      const ip = import.meta.env.VITE_API_IP;
      const response = await fetch(ip+"/api/coverc/open", {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
            "Accept": "application/json, text/plain, */*"
          }
        });
      if (!response.ok) {
        throw new Error('Network response was not ok')
      }
      const res = await response.json()

      if(res.execute){
        cmdExecutedNotify()
      }
  
      
    } catch (error) {
      noResponseNotify(error)
    }
  }

  const coverCloseCmd = async () => {
    if(!canCloseCover.value){
      cmdRefusedNotify()
      return
    }
    try {
      const ip = import.meta.env.VITE_API_IP;
      const response = await fetch(ip+"/api/coverc/close", {
          method: "POST",
          headers: {
            "Content-Type": "application/json",
            "Accept": "application/json, text/plain, */*"
          }
        });
      if (!response.ok) {
        throw new Error('Network response was not ok')
      }
      const res = await response.json()

      if(res.execute){
        cmdExecutedNotify()
      }

    } catch (error) {
      noResponseNotify(error)
    }
  }

  const cmdExecutedNotify = () => {
    toast.success(props.txt.gen.cmdAck, {
    autoClose: 500,
  });
}

const errorResponseNotify = (errorKey) => {
    const errorMessage = props.txt.errors.coverc[errorKey]
    toast.error(errorMessage, {
    autoClose: 3000,
  });
}

const cmdRefusedNotify = () => {
    toast.error(props.txt.gen.cmdRefused, {
    autoClose: 500,
  });
}

const noResponseNotify = (error) => {
    toast.error(error, {
    autoClose: 3000,
  });
}
  
  let intervalId = null
  onMounted(() => {
    fetchData()
    intervalId = setInterval(fetchData, 3000)
  })
  
  onUnmounted(() => {
    clearInterval(intervalId)
  })
  
  </script>