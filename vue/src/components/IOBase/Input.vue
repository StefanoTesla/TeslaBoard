<template>
  <div class="setting_row">
    <p>{{ props.txt.IOBase.pin }}</p>
      <div class="input_with_unit">
        <span class="unit">n</span><input :id="`sw_${index}_pin`" :class="['with_unit', pinUnvalid ? 'validation_error' : '']" type="number" v-model="swi.pin" @change="validate()"/>
      </div>
  </div>
  <div class="setting_row">
    <p>{{ props.txt.IOBase.DI.onDelay }}</p>
    <div class="input_with_unit">
       <span class="unit">ms</span><input :id="`sw_${index}_dOn`" :class="['with_unit', dOnUnvalid ? 'validation_error' : '']" type="number" v-model="swi.dOn" @change="validate()"/>
    </div>
  </div>
  <div class="setting_row">
    <p>{{ props.txt.IOBase.DI.offDelay }}</p>
    <div class="input_with_unit">
      <span class="unit">ms</span><input :id="`sw_${index}_dOff`" :class="['with_unit', dOffUnvalid ? 'validation_error' : '']" type="number" v-model="swi.dOff" @change="validate()"/>
    </div>
  </div>
  <div class="setting_row">
    <p>{{ props.txt.IOBase.DI.func }}</p>
    <select :id="`sw_${index}_invert`" :class="[invertUnvalid ? 'validation_error' : '']" v-model="swi.invert" @change="validate()">
      <option v-for="[key, value] in Object.entries(props.txt.IOBase.DI.invertEnum)" :key="key" :value="key">
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

  const emit = defineEmits(['update:validated','update:pinUsed']);

  let pinUnvalid = ref(false)
  let dOnUnvalid = ref(false)
  let dOffUnvalid = ref(false)
  let invertUnvalid = ref(false)


  const validate = () => {

    emit('update:validated', { index: props.index, isValid: false});
    props.swi.pin = parseInt(props.swi.pin)
    pinUnvalid.value = false

    if (isInvalidPin(props.swi.pin,'input')){
        pinUnvalid.value = true
        const errorMessage = props.txt.gen.pin + " " + props.swi.pin + " - " +  props.txt.errors.gpio.noUsableAsInput
        errorResponseNotify(errorMessage)
        return
    }
    

    props.swi.dOn = parseInt(props.swi.dOn)
    dOnUnvalid.value = false
    if(isNegative(props.swi.dOn)){
      dOnUnvalid.value = true
      errorResponseNotify(props.txt.errors.general.negativeValue)
      return
    }
    
    props.swi.dOff = parseInt(props.swi.dOff)
    dOffUnvalid.value = false
    if(isNegative(props.swi.dOff)){
      dOffUnvalid.value = true
      errorResponseNotify(props.txt.errors.general.negativeValue)
      return
    }
    
    props.swi.invert = parseInt(props.swi.invert)
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

    emit('update:validated', { index: props.index, isValid: true });
    emit('update:pinUsed', { pin: props.swi.pin, type:1 , index:props.index});
  }

  const errorResponseNotify = (errorMessage) => {
    toast.error(errorMessage, {
    autoClose: 3000,
  });
}


  onMounted(()=>{
    props.swi.pin = props.swi.pin ?? null
    props.swi.dOn = props.swi.dOn ?? 0;
    props.swi.dOff = props.swi.dOff ?? 0;
    props.swi.invert = props.swi.invert ?? 0;

    validate()
  })

</script>