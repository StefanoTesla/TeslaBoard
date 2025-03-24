<template>

  <Card
    v-if="props.txt.dome"
    :moduleName="props.txt.switch.title"
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
  >

    <div class="sw_grid">
      <div class="card" v-for="(swi,index) in switches.Switches" :key="index">
        <div class="title">
          <p>{{ swi.name }}</p>
        </div>
        <div class="sw_desc">
          <p>{{ swi.desc }}</p>
        </div>
          <div class="sw_containter" v-if="swi.type == 1">
            <div>
              <label class="toggle" :for="`sw_${index}_status`">
                <input class="toggle__input cursor-not-allowed" 
                  :name="`sw_${index}_status`" 
                  type="checkbox" 
                  :id="`sw_${index}_status`" 
                  v-model="swi.boValue" 
                  disabled>
                <div class="toggle__fill cursor-not-allowed"></div>
              </label>
            </div>
          </div>
          <div class="sw_containter" v-if="swi.type == 2">
            <div>
              <label class="toggle" :for="`sw_${index}_status`">
                <input class="toggle__input" type="checkbox" v-model="swi.boValue" :name="`sw_${index}_status`" :id="`sw_${index}_status`" @click="changeValueCmd(index)">
                <div class="toggle__fill cursor-pointer"></div>
              </label>
            </div>
          </div>
          <div class="sw_containter" v-if="swi.type == 3 || swi.type == 4">
            <div class="range">
              <input type="range" :id="`sw_${index}_slider`" :min="swi.min" :max="swi.max" step='1' v-model="swi.intValue" @change="changeValueCmd(index)"/>
            </div>
            <div class="sw_value">
              <p>{{ props.txt.gen.status.actualValue }}</p> <span> {{ swi.intValue }} </span> / <span> {{ swi.max }} </span>
            </div>
          </div>
      </div>  
    </div>  
  </Card>
</template>
  
  
  <script setup>
  import { ref,defineProps,onMounted,onUnmounted,computed } from 'vue'
  import { toast } from 'vue3-toastify';
  import Card from '../Card.vue';
  
  const props = defineProps({
    txt: Object
  })
  
  const switches = ref({})
  let dataLoaded = ref(false)
  let statusClass = ref('black')

  const fetchData = async () => {
    try {
      const response = await fetch('http://localhost:3000/api/switch/status')  // Sostituisci con il tuo endpoint API
      if (!response.ok) {
        throw new Error('Network response was not ok')
      }
      const data = await response.json()
      switches.value = data
      dataLoaded.value = true

      updateStatusData()
      
      
    } catch (error) {
      console.error('Errore durante la chiamata API:', error)
    }
  }


  const changeValueCmd = async(index) => {

    if( switches.value.Switches[index].type == 1){
      cmdRefusedNotify()
      return
    }
    if( switches.value.Switches[index].type == 2){
      switches.value.Switches[index].boValue = !switches.value.Switches[index].boValue
      switches.value.Switches[index].intValue = switches.value.Switches[index].boValue
    }

    try {
      const response = await fetch("http://localhost:3000/api/switch/set-value", {
          method: "POST",
          headers: {
            "Content-Type": "application/x-www-form-urlencoded",
            "Accept": "application/json, text/plain, */*"
          },
          body:new URLSearchParams({
            "id": index,
            "value" : switches.value.Switches[index].intValue
          })
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


  const updateStatusData = () => {
    const hasActive = switches.value.Switches.some(swi => swi.boValue);
    statusClass.value = hasActive ? 'green' : 'black';
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
    console.log("errori")
    console.log(errorKey)
    console.log(props.txt.errors.switch)
      const errorMessage = props.txt.errors.switch[errorKey]
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