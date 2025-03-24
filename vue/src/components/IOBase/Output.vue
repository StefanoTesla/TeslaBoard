<template>
  <div>
    <p>{{ props.txt.IOBase.pin }}</p>
    <div class="input_with_unit">
      <span class="unit">n</span><input :id="`sw_${index}_pin`" :class="['with_unit', pinUnvalid ? 'validation_error' : '']" type="number" v-model="swi.pin" @change="validate()"/>
    </div>
  </div>
  <div>
    <p>{{ props.txt.IOBase.DI.func }}</p>
    <select :id="`sw_${index}_invert`" :class="[invertUnvalid ? 'validation_error' : '']" v-model="swi.invert" @change="validate()">
      <option v-for="[key, value] in Object.entries(props.txt.IOBase.DO.invertEnum)" :key="key" :value="key">
        {{ value }}
      </option>
    </select>
  </div>
</template>


<script setup>

  import { ref,onMounted } from 'vue'
  import { toast } from 'vue3-toastify';
  import { useValidator } from '../../composables/Validator';

  const { isInvalidPin, isGreaterThan, isNegative } = useValidator();

 const props = defineProps({
   txt: Object,
   index: Number,
   swi: Object
 })

 const emit = defineEmits(['update:validated']);

 let pinUnvalid = ref(false)
 let invertUnvalid = ref(false)
 
 const validate = () => {

  emit('update:validated', { index: props.index, isValid: false});
  props.swi.pin = parseInt(props.swi.pin)
  pinUnvalid.value = false
  if (isInvalidPin(props.swi.pin,'output')){
      pinUnvalid.value = true
      const errorMessage = props.txt.gen.pin + " " + props.swi.pin + " - " +  props.txt.errors.gpio.noUsableAsOutput
      errorResponseNotify(errorMessage)
      return
  }
   

  invertUnvalid.value = false
  if(isNegative(props.swi.invert)){
   invertUnvalid.value = true
    errorResponseNotify(props.txt.errors.general.negativeValue)
    return
  }
   
  if(isGreaterThan(props.swi.invert,1)){
    invertUnvalid.value = true
    errorResponseNotify(props.txt.errors.general.positiveValue)
    return
  }
  
  emit('update:validated', { index: props.index, isValid: true});
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