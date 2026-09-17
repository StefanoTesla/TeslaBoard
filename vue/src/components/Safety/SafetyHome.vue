<template>
  <Card 
    v-if="t('safety')"
    :moduleName="t('safety.title')"
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
  >
    <div class="grid grid-cols1 gap-4">
      <div class="card">
        <p class="text-center">{{ t('safety.status') }} <span class="font-bold text-green-500!" v-if=safety.isSafe>SICURO</span> <span class="font-bold text-red-500!" v-if=!safety.isSafe>NON SICURO</span></p>
      </div>

      <div  class="card">
        <div v-for="cnd in safety.Conditions" class="pb-2">
          <Condition
            :t="(t)"
            :cnd="cnd"
          />
        </div>
      </div>
    </div>


  </Card>
</template>

<script setup>
import { ref, onMounted, onUnmounted, computed } from 'vue'
import Card from '../Card.vue';
import Condition from '../../components/IOBase/Conditions/ConditionHome.vue'

const props = defineProps({
  t: Function
})

let pollingTimeout = null;
let isPolling = false;
let abortController = null;

const safety = ref({})

let dataLoaded = ref(false)
let statusClass = ref('red')

const fetchData = async () => {

  if (abortController) {
    abortController.abort();
  }

  abortController = new AbortController();

  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+'/api/safety/status',{
      signal: abortController.signal,
    });

    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    const data = await response.json()
    safety.value = data
    dataLoaded.value = true

    const classes = ['red', 'green']
    statusClass.value = classes[safety.value.isSafe ? 1 : 0]
    
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

const safetyStateEnum = (status) => {
  const enumShutterState = [
    props.t('safety.home.unknow'), 
    props.t('safety.home.error'), 
    props.t('safety.home.safe'),  
    props.t('safety.home.unsafe'), 
  ]
  return enumSafetyState[status]
}

onMounted(() => {
  startPolling()
})

onUnmounted(() => {
  stopPolling()
})
</script>
