<template>

  <Card
    v-if="props.txt.dome"
    :moduleName="props.txt.dome.title"
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
  >
  <div class="card">
    <div class="setting_table">
      <div class="sw_header">
        <p>{{ props.txt.dome.setting.driverType }}</p>
      </div>
      <select :id="`dome_cmd_type`" :class="[invertUnvalid ? 'validation_error' : '']" v-model="dome.driverType" @change="validate()">
        <option v-for="[key, value] in Object.entries(props.txt.dome.setting.driverTypeEnum)" :key="key" :value="key">
          {{ value }}
        </option>
      </select> 
    </div>
  </div>
  <div class="card">
    <div class="setting_table">
      <div class="sw_header">
        <p>{{ props.txt.dome.setting.inOpen }}</p>
      </div>
      <Input
            :txt="props.txt"
            :index = 0
            :swi = "dome.pinOpen"
             @update:validated="handleValidation"
          />
    </div>
  </div>
  <div class="card">
    <div class="setting_table">
      <div class="sw_header">
        <p>{{ props.txt.dome.setting.inClose }}</p>
      </div>
      <Input
            :txt="props.txt"
            :index = 1
            :swi = "dome.pinClose"
             @update:validated="handleValidation"
          />
    </div>
  </div>
  <div class="card">
    <div class="setting_table">
      <div class="sw_header">
        <p>{{ props.txt.dome.setting.outStart }}</p>
      </div>
      <Output
            :txt="props.txt"
            :index = 2
            :swi = "dome.pinStart"
             @update:validated="handleValidation"
          />
    </div>
  </div>
  <div class="card">
    <div class="setting_table">
      <div class="sw_header">
        <p>{{ props.txt.dome.setting.outHalt }}</p>
      </div>
      <Output
            :txt="props.txt"
            :index = 2
            :swi = "dome.pinHalt"
             @update:validated="handleValidation"
          />
    </div>
  </div>
  <div class="card">
    <div class="setting_table">
      <div class="sw_header">
        <p>{{ props.txt.dome.setting.timeOutMoviment }}</p>
      </div>
      <div class="input_with_unit">
        <span class="unit">sec</span><input :id="`dome_timeout`" :class="['with_unit', pinUnvalid ? 'validation_error' : '']" type="number" v-model="dome.movTimeOut" @change="validate()"/>
      </div>
    </div>
  </div>
  <div class="card">
    <div>
      <div class="sw_header">
        <p>{{ props.txt.dome.setting.autoClose }}</p>
      </div>
      <p>{{ props.txt.dome.setting.autoclosewarning1 }}</p>
      <p>{{ props.txt.dome.setting.autoclosewarning2 }}</p>
      <p>{{ props.txt.dome.setting.autoclosewarning3 }}</p>
      <p>{{ props.txt.dome.setting.autoclosewarning4 }}</p>
      <p>{{ props.txt.dome.setting.autoclosewarning5 }}</p>
      <label class="toggle" for="dome_autoclose_enable">
        <input class="toggle__input" name="" type="checkbox" id="dome_autoclose_enable" v-model="dome.autoclose.enable">
        <div class="toggle__fill"></div>
      </label>
      <div class="input_with_unit">
        <span class="unit">min</span><input :id="`dome_timeout`" :class="['with_unit', pinUnvalid ? 'validation_error' : '']" type="number" v-model="dome.autoclose.minutes" @change="validate()"/>
      </div>
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
import Input from '../IOBase/Input.vue';
import Output from '../IOBase/Output.vue';

const props = defineProps({
  txt: Object,
  reboot: Boolean
})

const emit = defineEmits(['update:reboot']);

const dome = ref({})

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
  dome.value = JSON.parse(JSON.stringify(originalData.value));
}

const fetchData = async () => {
  try {
    const response = await fetch('http://localhost:3000/api/dome/cfg') 
    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    const data = await response.json()
    dome.value = data
    dataLoaded.value = true

    if(data.reboot){
      statusClass.value = 'orange'
    }

    originalData.value = JSON.parse(JSON.stringify(dome.value));

    
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

    const response = await fetch("http://localhost:3000/api/dome/cfg", {
        method: "POST",
        headers: {
        'Accept': 'application/json, text/plain, */*',
        'Content-Type': 'application/json'
        },
        body: JSON.stringify(dome.value)
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

watch(() => validation.value.some(item => item === false), (containsFalse) => {
    validationState.value = containsFalse ? false : true;    
  });

</script>