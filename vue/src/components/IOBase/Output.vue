<template>
  <div class="grid grid-cols-2">
    <div class="setting_row">
      <p>{{ t('IOBase.pin') }}</p>
      <div class="input_with_unit">
        <span class="unit">n</span><input :id="`sw_${index}_pin`" :class="['with_unit', pinUnvalid ? 'validation_error' : '']" type="number" v-model="swi.pin" @change="validate()"/>
      </div>
    </div>
    <div class="setting_row">
      <p>{{ t('IOBase.DO.func') }}</p>
      <select :id="`sw_${index}_invert`" :class="[invertUnvalid ? 'validation_error' : '']" v-model="swi.invert" @change="validate()">
        <option :value="false">{{ t('IOBase.DO.invertEnum.0') }}</option>
        <option :value="true">{{ t('IOBase.DO.invertEnum.1') }}</option>
      </select>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import { toast } from 'vue3-toastify';
import { useValidator } from '../../composables/Validator';

const { isInvalidPin, isGreaterThan } = useValidator();

const props = defineProps({
  t: Function,
  index: Number,
  swi: Object
})

const emit = defineEmits(['update:validated','update:pinUsed']);

let pinUnvalid = ref(false)
let invertUnvalid = ref(false)
 
const validate = () => {
  emit('update:validated', { index: props.index, isValid: false});
  props.swi.pin = parseInt(props.swi.pin)
  pinUnvalid.value = false
  if (isInvalidPin(props.swi.pin,'output')){
    pinUnvalid.value = true
    const errorMessage = props.t('gen.pin') + " " + props.swi.pin + " - " + props.t('errors.gpio.noUsableAsOutput')
    errorResponseNotify(errorMessage)
    return
  }
   
  if(isGreaterThan(props.swi.invert,1)){
    invertUnvalid.value = true
    errorResponseNotify(props.t('errors.general.positiveValue'))
    return
  }
  
  emit('update:validated', { index: props.index, isValid: true});
  emit('update:pinUsed', { pin: props.swi.pin, type:2, index:props.index});
}

const errorResponseNotify = (errorMessage) => {
  toast.error(errorMessage, {
    autoClose: 3000,
  });
}

onMounted(()=>{
  props.swi.pin = props.swi.pin ?? null
  props.swi.invert = props.swi.invert ?? 0;
  validate()
})
</script>
