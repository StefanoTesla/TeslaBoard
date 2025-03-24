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

const emit = defineEmits(['update:reboot']);

const coverC = ref({})

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
  coverC.value = JSON.parse(JSON.stringify(coverC.value));
}

const fetchData = async () => {
  try {
    const response = await fetch('http://localhost:3000/api/coverc/cfg')  // Sostituisci con il tuo endpoint API
    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    const data = await response.json()
    coverC.value = data
    dataLoaded.value = true


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

    const response = await fetch("http://localhost:3000/api/coverc/cfg", {
        method: "POST",
        headers: {
        'Accept': 'application/json, text/plain, */*',
        'Content-Type': 'application/json'
        },
        body: JSON.stringify(coverC.value)
      });

    if (!response.ok) { 
      if (response.status === 500) {
        throw new Error(props.txt.errors.general.configRejected)
      } else {
        throw new Error('Network response was not ok')
      }
    }

    const data = await response.json()
    cmdExecutedNotify()

    if(data.reboot){
      console.log("reboot needed")
      emit('update:reboot', true);
      statusClass.value = 'orange'
    }

    
  } catch (error) {
    errorResponseNotify(error)
  }
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
</script>