<template>
  <Card
    v-if="t('switch')"
    :moduleName="t('switch.title')"
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
  >
    <div class="sw_grid">
      <div class="card" v-for="(swi,index) in switches.Switches" :key="index">
        <div class="grid grid-flow-row grid-rows-2 grid-cols-1 lg:grid-rows-1 lg:grid-cols-8">
        <div class="col-span-1 lg:col-span-4" >
          <p :title="swi.uId">{{ swi.name }}</p>
          <p class="italic">{{ swi.desc }}</p>
        </div>
          <div class="sw_containter" v-if="swi.type == 1">
            <div>
              <div class="status" :for="`sw_${index}_status`">
                <div :class="['led',swi.status ? 'green' : 'black']"></div>
              </div>
            </div>
          </div>
          <div class="sw_containter" v-if="swi.type == 2">
              <div class="status" :for="`sw_${index}_status`">
                  <div :class="['led',swi.status ? 'green' : 'black']"></div>
              </div>
              <button :class="[swi.status ? 'red' : 'green']" :name="`sw_${index}_status`" :id="`sw_${index}_status`" @click="changeValueCmd(index)">
                <span v-if="swi.status">{{ t('gen.action.powerOff') }}</span>
                <span class="txt-black" v-else>{{ t('gen.action.powerOn') }}</span></button>
          </div>
          <div class="sw_containter" v-if="swi.type == 3 || swi.type == 4">
            <div class="sw_sidebar">
              <div class="range">
                <input type="range" :id="`sw_${index}_slider`" :min="swi.min" :max="swi.max" step='1' v-model="swi.status" @change="changeValueCmd(index)"/>
              </div>
              <div class="sw_value">
                <p>{{ t('gen.status.actualValue') }}</p> <span> {{ swi.status }} </span> / <span> {{ swi.max }} </span>
              </div>
            </div>

          </div>
          <div class="sw_containter" v-if="swi.type == 5">
            <div class="sw_value">
              <p>{{ t('gen.status.actualValue') }}</p> <span> {{ swi.status }} </span>
            </div>
          </div>


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

const switches = ref({})
let pollingTimeout = null;
let isPolling = false;
let abortController = null;
let dataLoaded = ref(false)
let statusClass = ref('black')

const fetchData = async () => {

  if (abortController) {
    abortController.abort();
  }

  abortController = new AbortController();
  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+'/api/switch/status',{
      signal: abortController.signal,
    });
    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    const data = await response.json()

    switches.value = data
    dataLoaded.value = true

    updateStatusData()
    
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


const changeValueCmd = async(index) => {

  if( switches.value.Switches[index].type == 1){
    cmdRefusedNotify()
    return
  }

  if( switches.value.Switches[index].type == 2){
    switches.value.Switches[index].status = switches.value.Switches[index].status ? 0:1
  }

  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+"/api/switch/set-value", {
        method: "POST",
        headers: {
          "Content-Type": "application/x-www-form-urlencoded",
          "Accept": "application/json, text/plain, */*"
        },
        body:new URLSearchParams({
          "id": index,
          "value" : switches.value.Switches[index].status
        })
      });
    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    const res = await response.json()

    updateStatusData()
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

const updateStatusData = () => {
  let hasActive = false;
  switches.value.Switches.forEach(element => {
    if(element.type != 1 && element.status > 0){
      hasActive = true;
    }
  });

  statusClass.value = hasActive ? 'green' : 'black';
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
  const errorMessage = props.t(`errors.switch.${errorKey}`)
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
