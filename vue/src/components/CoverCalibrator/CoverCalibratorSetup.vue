<template>

  <Card
    v-if="props.txt.switch"
    :moduleName="props.txt.coverC.title"
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
  >
  <div class="card">
    <div class="setting_table">
    <div class="sw_header">
    <p>{{ props.txt.coverC.calibrator }}</p>
    <label class="toggle" for="coverc_calib_present">
      <input class="toggle__input" name="" type="checkbox" id="coverc_calib_present" v-model="coverC.calibrator.present">
      <div class="toggle__fill"></div>
    </label>
    </div>
    <div>
      <PWM
          v-if="coverC.calibrator.present"
          :txt="props.txt"
          :index = 0
          :swi = "coverC.calibrator.pwm"
          @update:validated="handleValidation"
          @update:pinUsed="updateCalibratorPin"
        />
    </div>
    </div>
  </div>

  <div class="card">
    <div class="setting_table">
      <div class="sw_header">
    <p>{{ props.txt.coverC.cover }}</p>
    <label class="toggle" for="coverc_cover_present">
      <input class="toggle__input" name="" type="checkbox" id="coverc_cover_present" v-model="coverC.cover.present">
      <div class="toggle__fill"></div>
    </label>
  </div>

      <Servo
        v-if="coverC.cover.present"
        :txt="props.txt"
        :index = 1
        :swi = "coverC.cover.servo"
        @update:validated="handleValidation"
        @update:pinUsed="updateCoverPin"
      />
    </div>
  </div>



  <div class="config_buttons">
      <button class="green cursor-pointer" @click="getOriginal()">{{ props.txt.gen?.loadFromBoard }}</button>
      <a href="./switch/switchconfig.txt" class="f_button orange" download>{{ props.txt.gen?.downloadFile }}</a>
      <button :class="[validationState ? 'red cursor-pointer' : 'black cursor-not-allowed']" @click="saveData()">{{ props.txt.gen?.save }}</button>
    </div>

  </Card>
</template>


<script setup>
import { ref,onMounted,watch } from 'vue'
import { toast } from 'vue3-toastify';
import Card from '../Card.vue';
import PWM from '../IOBase/PWM.vue';
import Servo from '../IOBase/Servo.vue';

const props = defineProps({
  txt: Object,
  reboot: Boolean
})

const emit = defineEmits(['update:reboot','update:pinUsed']);

const coverC = ref({})
const pinUsed = ref([])

const originalData = ref({})
let dataLoaded = ref(false)
let statusClass = ref('green')
let validation = ref([])
let validationState = ref(true)


const handleValidation = (data) => {
const { index, isValid } = data;
validation.value[index] = isValid;
}

const getOriginal = () => {
  coverC.value = JSON.parse(JSON.stringify(originalData.value));
}

const fetchData = async () => {
  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+'/api/coverc/cfg')
    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    const data = await response.json()
    coverC.value = data
    dataLoaded.value = true

    if(!coverC.value.cover.servo){
      coverC.value.cover.servo = {}
    }

    if(!coverC.value.calibrator.pwm){
      coverC.value.calibrator.pwm = {}
    }
    updatePinsforObserver(data)
    setupWatch()
    if(data.reboot){
      statusClass.value = 'orange'
    }

    originalData.value = JSON.parse(JSON.stringify(coverC.value));

    
  } catch (error) {
    console.error('Errore durante la chiamata API:', error)
  }
}



const saveData = async () => {

  if(!validationState.value){
    errorResponseNotify(props.txt.errors.general.validationFailed)
    return
  }

  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+"/api/coverc/cfg", {
        method: "POST",
        headers: {
        'Accept': 'application/json, text/plain, */*',
        'Content-Type': 'application/json'
        },
        body: JSON.stringify(coverC.value)
      });

      const data = await response.json()
      if (!response.ok) throw { status: response.status, data }
    cmdExecutedNotify()

    if(data.reboot){
      console.log("reboot needed")
      emit('update:reboot', true);
      statusClass.value = 'orange'
    }

    
  } catch (err) {
    if (err?.status === 500 && Array.isArray(err.data?.errors)) {
      err.data.errors.forEach(e =>
        typeof e === 'object'
          ?  handleStructuredError(e)
          : errorResponseNotify(e)
      )
    } else {
      errorResponseNotify(err?.message || props.txt.errors.general.configRejected)
    }
  }
}

const handleStructuredError = (e) => {
  let msg
  let pinName
  switch (e.id) {
    case 1:
      pinName = props.txt.coverC.calibrator
      break
    case 2:
      pinName = props.txt.coverC.cover
      break
    default:
      pinName = `Pin ${e.id}`
  }
  
  msg = pinName + ": " + props.txt.errors.gpioValidation[e.error]
  errorResponseNotify(msg)
}

const cmdExecutedNotify = () => {
  toast.success(props.txt.gen.configSaved, {
  autoClose: 500,
});
}

const errorResponseNotify = (errorMessage) => {
 toast.error(errorMessage, {
  autoClose: 3000,
  });
}

onMounted(() => {
  fetchData()
})

function setupWatch(){

  watch(
  () => [validation.value, coverC.value.calibrator.present, coverC.value.cover.present],
  () => {
    
    if (coverC.value.calibrator.present && validation.value[0] === false) {
      validationState.value = false;
    } else if (coverC.value.cover.present && validation.value[1] === false) {
      validationState.value = false;
    } else {
      validationState.value = true;
    }
  },
  { deep: true } // Per rilevare modifiche all'interno dell'array
);
}

const updatePinsforObserver = (data) => {
  pinUsed.value=[]
  pinUsed.value[0]={"pin":data.calibrator.pwm.pin,"type":3,"module": 2}
  pinUsed.value[1]={"pin":data.cover.servo.pin,"type":4,"module": 2}
  emit('update:pinUsed', pinUsed.value);
}

const updateCalibratorPin = (data) => {
  pinUsed.value[0]={"pin":data.pin,"type":3,"module": 2}
  emit('update:pinUsed', pinUsed.value);
}

const updateCoverPin = (data) => {
  pinUsed.value[1]={"pin":data.pin,"type":4,"module": 2}
  emit('update:pinUsed', pinUsed.value);
}

</script>