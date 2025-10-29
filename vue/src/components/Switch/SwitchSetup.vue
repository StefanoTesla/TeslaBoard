<template>
  <Card
    v-if="t('switch')"
    :moduleName="t('switch.title')"
    :dataLoaded="dataLoaded"
    :statusClass="statusClass"
  >
    <div class="card flex">
      <div class="txt pr-4">
        {{ t('gen.moduleIs') }}
      </div>
      <div class="module_toggle">
        <label class="toggle " for="switch_module_status">
          <input class="toggle__input" name="" type="checkbox" id="switch_module_status" v-model="switches.enable" @click="handleValidation">
          <div class="toggle__fill"></div>
        </label>
      </div>
    </div>
    <div class="sw_tools" v-if="switches.enable">
      <button class="ml-4 sw_add green" @click="addNewSwitch()"><svg xmlns="http://www.w3.org/2000/svg"  viewBox="0 0 72 72" width="40px" height="40px"><path d="M36,12c13.255,0,24,10.745,24,24c0,13.255-10.745,24-24,24S12,49.255,12,36C12,22.745,22.745,12,36,12z M44,39 c1.657,0,3-1.343,3-3c0-1.657-1.343-3-3-3c-0.329,0-2.426,0-5,0c0-2.574,0-4.672,0-5c0-1.657-1.343-3-3-3c-1.657,0-3,1.343-3,3  c0,0.328,0,2.426,0,5c-2.574,0-4.671,0-5,0c-1.657,0-3,1.343-3,3c0,1.657,1.343,3,3,3c0.329,0,2.426,0,5,0c0,2.574,0,4.672,0,5  c0,1.657,1.343,3,3,3c1.657,0,3-1.343,3-3c0-0.328,0-2.426,0-5C41.574,39,43.671,39,44,39z"/></svg></button>
    </div>

    <div class="sw_grid" v-if="switches.enable">
      <div class="card" v-for="(swi,index) in switches.Switches" :key="index">
        <div class="sw_delete">
          <button v-if="index > 0" @click=moveSwitch(index,true)><svg fill="#000000" width="30px" height="30px" viewBox="0 0 32 32" version="1.1" xmlns="http://www.w3.org/2000/svg"><path d="M11.25 15.688l-7.656 7.656-3.594-3.688 11.063-11.094 11.344 11.344-3.5 3.5z"></path></svg></button>
          <button v-if="index < switches.Switches.length - 1" @click="moveSwitch(index)"><svg fill="#000000" width="30px" height="30px" viewBox="0 0 32 32" version="1.1" xmlns="http://www.w3.org/2000/svg"><path d="M11.125 16.313l7.688-7.688 3.594 3.719-11.094 11.063-11.313-11.313 3.5-3.531z"></path></svg></button>
          <button class="red" @click="deleteSwitch(index)"><svg xmlns="http://www.w3.org/2000/svg"  viewBox="0 0 128 128" width="20px" height="20px"><path d="M 49 1 C 47.34 1 46 2.34 46 4 C 46 5.66 47.34 7 49 7 L 79 7 C 80.66 7 82 5.66 82 4 C 82 2.34 80.66 1 79 1 L 49 1 z M 24 15 C 16.83 15 11 20.83 11 28 C 11 35.17 16.83 41 24 41 L 101 41 L 101 104 C 101 113.37 93.37 121 84 121 L 44 121 C 34.63 121 27 113.37 27 104 L 27 52 C 27 50.34 25.66 49 24 49 C 22.34 49 21 50.34 21 52 L 21 104 C 21 116.68 31.32 127 44 127 L 84 127 C 96.68 127 107 116.68 107 104 L 107 40.640625 C 112.72 39.280625 117 34.14 117 28 C 117 20.83 111.17 15 104 15 L 24 15 z M 24 21 L 104 21 C 107.86 21 111 24.14 111 28 C 111 31.86 107.86 35 104 35 L 24 35 C 20.14 35 17 31.86 17 28 C 17 24.14 20.14 21 24 21 z M 50 55 C 48.34 55 47 56.34 47 58 L 47 104 C 47 105.66 48.34 107 50 107 C 51.66 107 53 105.66 53 104 L 53 58 C 53 56.34 51.66 55 50 55 z M 78 55 C 76.34 55 75 56.34 75 58 L 75 104 C 75 105.66 76.34 107 78 107 C 79.66 107 81 105.66 81 104 L 81 58 C 81 56.34 79.66 55 78 55 z"/></svg></button>
        </div>
        <div class="sw_header">
          <p>{{ t('IOBase.name') }}</p>
          <input type="text" :id="`sw_${index}_name`" class="w-full" v-model="swi.name" maxlength=20  @change="sanitize(index)"/>
        </div>
        <div class="sw_header">
          <p>{{ t('IOBase.description') }}</p>
          <input type="text" :id="`sw_${index}_desc`" class="w-full" v-model="swi.desc" maxlength=20 @change="sanitize(index)"/>
        </div>
        <div class="sw_header">
          <p>{{ t('IOBase.type') }}</p>
          <select :id="`sw_${index}_type`" v-model="swi.type">
            <option value="1">{{ t('IOBase.typeEnum.1') }}</option>
            <option value="2">{{ t('IOBase.typeEnum.2') }}</option>
            <option value="3">{{ t('IOBase.typeEnum.3') }}</option>
            <option value="4">{{ t('IOBase.typeEnum.4') }}</option>
          </select>
        </div>

        <Input
          v-if="swi.type==1"
          :t="t"
          :index="index"
          :swi="swi"
          @update:validated="handleValidation"
          @update:pinUsed="updatePin"
        />

        <Output
          v-if="swi.type==2"
          :t="t"
          :index="index"
          :swi="swi"
          @update:validated="handleValidation"
          @update:pinUsed="updatePin"
        />

        <PWM
          v-if="swi.type==3"
          :t="t"
          :index="index"
          :swi="swi"
          @update:validated="handleValidation"
          @update:pinUsed="updatePin"
        />
        
        <Servo
          v-if="swi.type==4"
          :t="t"
          :index="index"
          :swi="swi"
          @update:validated="handleValidation"
          @update:pinUsed="updatePin"
        />
      </div>
    </div>

    <div class="config_buttons">
      <button class="green cursor-pointer" @click="getOriginal()">{{ t('gen.loadFromBoard') }}</button>
      <button :class="[validationState ? 'red cursor-pointer' : 'black cursor-not-allowed']" @click="saveData()">{{ t('gen.save') }}</button>
    </div>
  </Card>
</template>

<script setup>
import { ref, onMounted, watch } from 'vue'
import { toast } from 'vue3-toastify';
import Card from '../Card.vue';
import Input from '../IOBase/Input.vue';
import Output from '../IOBase/Output.vue';
import PWM from '../IOBase/PWM.vue';
import Servo from '../IOBase/Servo.vue';

const props = defineProps({
  t: Function,
  reboot: Boolean
})

const emit = defineEmits(['update:reboot','update:pinUsed']);

const switches = ref({})
let pinUsed = ref([])

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
  switches.value = JSON.parse(JSON.stringify(originalData.value));
}

const fetchData = async () => {
  const ip = import.meta.env.VITE_API_IP;
  try {
    const response = await fetch(ip+'/api/switch/cfg')
    if (!response.ok) {
      throw new Error('Network response was not ok')
    }
    const data = await response.json()
    switches.value = data
    dataLoaded.value = true

    updatePinsforObserver(data)
    
    if(data.reboot){
      statusClass.value = 'orange'
    }

    originalData.value = JSON.parse(JSON.stringify(switches.value));

  } catch (error) {
    console.error('Errore durante la chiamata API:', error)
  }
}

const cleanProperty = () => {
  switches.value.Switches = switches.value.Switches.map(swi => {

    let cleanedSwi = {
      name: swi.name,
      desc: swi.desc,
      type: parseInt(swi.type),
      pin: swi.pin
    };

    swi.pin = parseInt(swi.pin)
    swi.type = parseInt(swi.type)

    if (swi.type === 1) {
      cleanedSwi.dOn = swi.dOn ?? 0;
      cleanedSwi.dOff = swi.dOff ?? 0;
    }

    if (swi.type === 1 || swi.type === 2) {
      cleanedSwi.invert = swi.invert ?? 0;
    }

    if (swi.type === 4) {
      cleanedSwi.openDeg = swi.openDeg ?? 0;
      cleanedSwi.closeDeg = swi.closeDeg ?? 0;
      cleanedSwi.maxDeg = swi.maxDeg ?? 270;
      cleanedSwi.movTime = swi.movTime ?? 270;
    }

    return cleanedSwi;
  });
}

const saveData = async () => {
  cleanProperty();

  if(!validationState.value && switches.enable){
    errorResponseNotify(props.t('errors.general.validationFailed'))
    return
  }

  try {
    const ip = import.meta.env.VITE_API_IP;
    const response = await fetch(ip+"/api/switch/cfg", {
      method: "POST",
      headers: {
        'Accept': 'application/json, text/plain, */*',
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(switches.value)
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
          ? errorResponseNotify(props.t('switch.switch') + " n°" + e.id + ": " + props.t(`errors.gpioValidation.${e.error}`))
          : errorResponseNotify(e)
      )
    } else {
      errorResponseNotify(err?.message || props.t('errors.general.configRejected'))
    }
  }
}

const deleteSwitch = (index) => {
  switches.value.Switches.splice(index,1)
  validation.value.splice(index,1)
}

const moveSwitch = (id,dir) => {
  if(dir){
    [switches.value.Switches[id], switches.value.Switches[id - 1]] = [switches.value.Switches[id - 1], switches.value.Switches[id]];
    [validation.value[id], validation.value[id - 1]] = [validation.value[id - 1], validation.value[id]];
  } else {
    [switches.value.Switches[id], switches.value.Switches[id + 1]] = [switches.value.Switches[id + 1], switches.value.Switches[id]];
    [validation.value[id], validation.value[id + 1]] = [validation.value[id + 1], validation.value[id]];
  }
}

const sanitize = (index) => {
  switches.value.Switches[index].name = switches.value.Switches[index].name.replace(/[<>#!?*]/g, "").slice(0, 20);
  switches.value.Switches[index].desc = switches.value.Switches[index].desc.replace(/[<>#!?*]/g, "").slice(0, 20);
}

const addNewSwitch = () => {
  if(switches.value.Switches.length <= 14){
    switches.value.Switches.push({"name":"","desc":"","type":0,"pin":null,"invert":0,"maxDeg":90,"closeDeg":0,"openDeg":0,"dOn":0,"dOff":0})
  } else {
    errorResponseNotify(props.t('errors.switch.limitReached'))
  }
}

const cmdExecutedNotify = () => {
  toast.success(props.t('gen.configSaved'), {
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

const updatePinsforObserver = (data) => {
  pinUsed.value=[]
  data.Switches.forEach((element,i) => {
    pinUsed.value[i] = {pin:element.pin, type:element.type, module: 3}
  });
  emit('update:pinUsed', pinUsed.value);
}

const updatePin = () => {
  pinUsed.value = []
  switches.value.Switches.forEach((element,i) => {
    pinUsed.value[i] = {pin:element.pin, type:element.type, module: 3}
  });

  emit('update:pinUsed', pinUsed.value);
}
</script>
