<template>
  <Card 
    v-if="t('dome')"
    :moduleName="t('dome.title')"
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
  >
    <div class="grid sm:grid-cols1 md:grid-cols-2 gap-4">
      <div class="card">
        <p class="title">{{ t('dome.home.roofState') }}</p>
        <p class="text-center pl-2">{{ shutterStateEnum(dome.shutter.roofState) }}</p>
      </div>

      <div class="card">
        <p class="title">{{ t('dome.home.input') }}</p>
        <div class="justify-items-center uppercase flex items-center">
          <p>{{ t('gen.status.open') }}</p>
          <span class="block" :class="['led', dome.shutter.input.open ? 'green' : 'black']"></span>
        </div>
        <div class="justify-items-center uppercase flex">
          <p>{{ t('gen.status.close') }}</p>
          <div class="block" :class="['led', dome.shutter.input.close ? 'green' : 'black']"></div>
        </div>
      </div>

      <div class="card flex flex-col justify-evenly">
        <button :class="shutterOpenCmdClass" @click="cmdShutterOpen">{{ t('gen.action.open') }}</button>
        <button :class="shutterCloseCmdClass" @click="cmdShutterClose">{{ t('gen.action.close') }}</button>
        <button class="red cursor-pointer" @click="cmdShutterHalt">{{ t('gen.action.halt') }}</button>
      </div>
      
      <div class="card flex flex-col justify-evenly">
        <div>
          <p class="title">{{ t('dome.home.actualCommand') }}</p>
          <p>{{ commandEnum(dome.shutter.actualCommand) }}</p>
        </div>
        <div>
          <p class="title">{{ t('dome.home.lastTravelTime') }}</p>
          <p class="text-center">{{ dome.shutter.lastTravelTime }} sec.</p>
        </div>
        <div>
          <p class="title">{{ t('dome.home.autoClose.title') }}</p>
          <p class="text-center font-bold" v-if="dome.shutter.autoClose?.enable">{{ t('dome.home.autoClose.enabled') }}</p>
          <p class="text-center" v-if="!dome.shutter.autoClose?.enable">{{ t('dome.home.autoClose.disabled') }}</p>
        </div>
      </div>
    </div>
  </Card>
</template>

<script setup>
import { ref, onMounted, onUnmounted, computed } from 'vue'
import { toast } from 'vue3-toastify';
import Card from '../Card.vue';

const props = defineProps({
  t: Function
})

const dome = ref({})

let dataLoaded = ref(false)
let statusClass = ref('red')

const fetchData = async () => {
  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+'/api/dome/status')
    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    const data = await response.json()
    dome.value = data
    dataLoaded.value = true

    const classes = ['green', 'green', 'orange', 'orange', 'red']
    statusClass.value = classes[dome.value.shutter.roofState] 
    
  } catch (error) {
    console.error('Errore durante la chiamata API:', error)
  }
}

const shutterStateEnum = (status) => {
  const enumShutterState = [
    props.t('gen.status.open'), 
    props.t('gen.status.close'), 
    props.t('dome.home.shutterState.opening'), 
    props.t('dome.home.shutterState.closing'), 
    props.t('dome.home.shutterState.error')
  ]
  return enumShutterState[status]
}

const commandEnum = (status) => {
  const enumCommand = [
    props.t('dome.home.shutterCommand.idle'), 
    props.t('dome.home.shutterState.opening'), 
    props.t('dome.home.shutterState.closing'), 
    props.t('dome.home.shutterCommand.halt')
  ]
  return enumCommand[status]
}

const canOpenShutter = () => {
  if(!dataLoaded.value) return false
  return dome.value.shutter.canOpen ? true : false;
}

const canCloseShutter = () => {
  if(!dataLoaded.value) return false
  return dome.value.shutter.canClose ? true : false;
}

const shutterOpenCmdClass = computed(() => {
  return canOpenShutter() ? ['green', 'cursor-pointer'] : ['disactivated', 'cursor-not-allowed'] 
})

const shutterCloseCmdClass = computed(() => {
  return canCloseShutter() ? ['green', 'cursor-pointer'] : ['disactivated', 'cursor-not-allowed'] 
})

const cmdShutterOpen = () => sendCommand('open', canOpenShutter())
const cmdShutterClose = () => sendCommand('close', canCloseShutter())
const cmdShutterHalt = () => sendCommand('halt', true)

const sendCommand = async (endpoint, canExecute = true) => {
  if (!canExecute) {
    cmdRefusedNotify()
    return
  }
  
  try {
    const ip = import.meta.env.VITE_API_IP
    const response = await fetch(`${ip}/api/dome/${endpoint}`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        "Accept": "application/json, text/plain, */*"
      }
    })
    
    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    
    const res = await response.json()

    if (res.error) {
      errorResponseNotify(res.error)
      return
    }

    if (res.execute) {
      cmdExecutedNotify()
      return
    }
    
    cmdRefusedNotify()
    
  } catch (error) {
    noResponseNotify(error)
  }
}

const cmdExecutedNotify = () => {
  toast.success(props.t('gen.cmdAck'), {
    autoClose: 500,
  });
}

const cmdRefusedNotify = () => {
  toast.error(props.t('gen.cmdRefused'), {
    autoClose: 500,
  });
}

const errorResponseNotify = (errorKey) => {
  const errorMessage = props.t(`errors.dome.${errorKey}`)
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
