<template>


<Card 
v-if="props.txt.dome"
    :moduleName="props.txt.dome.title"
    :dataLoaded="dataLoaded"
    :statusClass=statusClass
    >

  <div class="card">
    <div class="flex">
      <p>{{ txt.dome.home.roofState }}</p>
      <p class="pl-2">{{ shutterStateEnum(dome.status.roofState) }}</p>
    </div>

  </div>
  <div class="card">
    <div class="flex justify-evenly">
      <button :class="shutterOpenCmdClass" @click="cmdShutterOpen">{{ txt.gen.action.open }}</button>
      <button :class="shutterCloseCmdClass" @click="cmdShutterClose">{{ txt.gen.action.close }}</button>
      <button class="red cursor-pointer" @click="cmdShutterHalt">{{ txt.gen.action.halt }}</button>
    </div>

  </div>
  <div class="card">
    <div class="title">
      <p>{{ txt.dome.home.input }}</p>
    </div>
    <div class="grid grid-cols-2">
      <div class="justify-items-center uppercase">{{ txt.gen.status.open }} 
        <div :class="['led', dome.input.open ? 'green' : 'red']"></div>
      </div>
      <div class="justify-items-center uppercase">{{ txt.gen.status.close }} 
        <div :class="['led', dome.input.close ? 'green' : 'red']"></div>
      </div>
    </div>
  </div>
  <div class="card text-center">
    <p>{{ txt.dome.home.actualCommand }}</p>
    <p>{{ commandEnum(dome.status.actualCommand) }}</p>
    <p>{{ txt.dome.home.lastCommand }}</p>
    <p>{{ commandEnum(dome.status.lastCommand) }}</p>
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


const dome = ref({})
let dataLoaded = ref(false)
let statusClass = ref('red')


const fetchData = async () => {
  try {
    const response = await fetch('http://localhost:3000/api/dome/status')
    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    const data = await response.json()
    dome.value = data
    dataLoaded.value = true

    const classes = ['green', 'green', 'orange', 'orange', 'red']
    statusClass.value = classes[dome.value.status.roofState] 

    
  } catch (error) {
    console.error('Errore durante la chiamata API:', error)
  }
}


const shutterStateEnum = (status) => {
  const enumShutterState = [props.txt.gen.status.open, props.txt.gen.status.close, props.txt.dome.home.shutterState.opening, props.txt.dome.home.shutterState.closing, props.txt.dome.home.shutterState.error]
  return enumShutterState[status]
}

const commandEnum = (status) => {
  const enumCommand = [props.txt.dome.home.shutterCommand.idle, props.txt.dome.home.shutterState.opening, props.txt.dome.home.shutterState.closing, props.txt.dome.home.shutterCommand.halt]
  return enumCommand[status]
}

const canOpenShutter = () =>{
  if(!dataLoaded)return false
  return (dome.value.status.actualCommand == 0 && dome.value.status.roofState != 0) ? true : false;
}

const canCloseShutter = () =>{
  if(!dataLoaded)return false
  return (dome.value.status.actualCommand == 0 && dome.value.status.roofState != 1) ? true : false;
}

const shutterOpenCmdClass = computed(() => {
  return canOpenShutter() ?['green', 'cursor-pointer'] : ['disactivated', 'cursor-not-allowed'] 
})

const shutterCloseCmdClass = computed(() => {
  return canCloseShutter() ?['green', 'cursor-pointer'] : ['disactivated', 'cursor-not-allowed'] 
})


const cmdShutterOpen = async() =>{
  if(!canOpenShutter()){
    cmdRefusedNotify()
    return
  }
  try {
      const response = await fetch("http://localhost:3000/api/dome/open", {
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
        errorResponseNotify(res.error)
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
const cmdShutterClose = async() =>{
  if(!canCloseShutter()){
    cmdRefusedNotify()
    return
  }
  try {
      const response = await fetch("http://localhost:3000/api/dome/close", {
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
        errorResponseNotify(res.error)
        return
      }

      if(res.execute){
        cmdExecutedNotify()
        return
      }

      cmdRefusedNotify()

    } catch (error) {
      noResponseNotify(error)
    }

}

const cmdShutterHalt= async() =>{
  try {
      const response = await fetch("http://localhost:3000/api/dome/halt", {
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
        errorResponseNotify(res.error)
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

const cmdExecutedNotify = () => {
    toast.success(props.txt.gen.cmdAck, {
    autoClose: 500,
  });
}
const cmdRefusedNotify = () => {
    toast.error(props.txt.gen.cmdRefused, {
    autoClose: 500,
  });
}
const errorResponseNotify = (errorKey) => {
    const errorMessage = props.txt.errors.dome[errorKey]
    toast.error(errorMessage, {
    autoClose: 3000,
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