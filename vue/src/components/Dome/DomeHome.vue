<template>
  <Card 
    v-if="t('dome')"
    :moduleName="t('dome.title')"
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
  >
    <div class="grid sm:grid-cols1 md:grid-cols-2 gap-4">

      <div class="card flex flex-col gap-y-8">
        <button :class="shutterOpenCmdClass" @click="cmdShutterOpen">{{ t('gen.action.open') }}</button>
        <button :class="shutterCloseCmdClass" @click="cmdShutterClose">{{ t('gen.action.close') }}</button>
        <button class="red cursor-pointer" @click="cmdShutterHalt">{{ t('gen.action.halt') }}</button>
      </div>



      <div class="card flex flex-col justify-evenly">
        <p>{{ t('dome.home.roofState') }} <b>{{ shutterStateEnum(dome.shutter.roofState) }}</b></p>
        <p>{{ t('dome.home.input') }}: 
          <span :class="[ dome.shutter.input.open ? 'txt-green' : 'txt-black' ]">{{ t('gen.status.open') }}</span>                                  
          <span :class="[ dome.shutter.input.close ? 'txt-green' : 'txt-black']">{{ t('gen.status.close') }}</span>
        </p>
          <p>{{ t('dome.home.actualCommand') }} <b>{{ commandEnum(dome.shutter.actualCommand) }}</b></p>
          <p>{{ t('dome.home.lastTravelTime') }} <b>{{ dome.shutter.lastTravelTime }} sec.</b></p>
          <p>{{ t('dome.home.autoClose.title') }} 
            
            <b class="txt-green" v-if="dome.shutter.autoClose?.enable">{{ t('dome.home.autoClose.enabled') }}</b>
            <b class="txt-black" v-else>{{ t('dome.home.autoClose.disabled') }}</b></p>
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

let pollingTimeout = null;
let isPolling = false;
let abortController = null;

const dome = ref({})

let dataLoaded = ref(false)
let statusClass = ref('red')

const fetchData = async () => {

  if (abortController) {
    abortController.abort();
  }

  abortController = new AbortController();

  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+'/api/dome/status',{
      signal: abortController.signal,
    });

    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    const data = await response.json()
    dome.value = data
    dataLoaded.value = true

    const classes = ['green', 'black', 'orange', 'orange', 'red']
    statusClass.value = classes[dome.value.shutter.roofState] 
    
  } catch (error) {
    console.error('Errore durante la chiamata API:', error)
  } finally {
    if (isPolling) {
      pollingTimeout = setTimeout(fetchData, 3000);
    }
  }
}


const startPolling = () => {
  isPolling = true;
  fetchData();
};

const stopPolling = () => {
  isPolling = false;
  if (pollingTimeout) {
    clearTimeout(pollingTimeout);
    pollingTimeout = null;
  }
  if (abortController) {
    abortController.abort();
    abortController = null;
  }
};



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
  startPolling()
})

onUnmounted(() => {
  stopPolling()
})
</script>
